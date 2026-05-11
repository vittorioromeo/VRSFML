#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Utf8String.hpp"
#include "SFML/System/Utf8StringCodepoints.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Restrict.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::TextUtils
{
////////////////////////////////////////////////////////////
struct TextSpacingConstants
{
    float whitespaceWidth;    //!< Width of a whitespace character
    float finalLetterSpacing; //!< Letter spacing factor
    float finalLineSpacing;   //!< Line spacing factor
};


////////////////////////////////////////////////////////////
/// \brief Layout-relevant subset of `TextData` (no transform, no colors)
///
/// Aggregates the fields of `TextData` that drive glyph layout. Bundling
/// them avoids forwarding 8+ scalars individually through the layout API.
///
////////////////////////////////////////////////////////////
struct TextLayoutInputs
{
    bool         bold;             //!< Bold characters
    bool         italic;           //!< Italic characters
    bool         underlined;       //!< Underlined characters
    bool         strikeThrough;    //!< Strike through characters
    unsigned int characterSize;    //!< Base size of characters, in pixels
    float        letterSpacing;    //!< Spacing factor between letters
    float        lineSpacing;      //!< Spacing factor between lines
    float        outlineThickness; //!< Thickness of the text's outline
};


////////////////////////////////////////////////////////////
/// \brief Build a `TextLayoutInputs` from a `TextData`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline TextLayoutInputs makeTextLayoutInputs(const TextData& textData)
{
    return {
        .bold             = textData.bold,
        .italic           = textData.italic,
        .underlined       = textData.underlined,
        .strikeThrough    = textData.strikeThrough,
        .characterSize    = textData.characterSize,
        .letterSpacing    = textData.letterSpacing,
        .lineSpacing      = textData.lineSpacing,
        .outlineThickness = textData.outlineThickness,
    };
}


////////////////////////////////////////////////////////////
template <typename TFontSource>
[[nodiscard]] inline TextSpacingConstants precomputeSpacingConstants(
    const TFontSource& font,
    const bool         isBold,
    const unsigned int characterSize,
    const float        letterSpacing,
    const float        lineSpacing)
{
    float whitespaceWidth = font.getGlyph(U' ', characterSize, isBold, /* outlineThickness */ 0.f).advance;

    const float finalLetterSpacing = (whitespaceWidth / 3.f) * (letterSpacing - 1.f);
    whitespaceWidth += finalLetterSpacing;

    return {
        .whitespaceWidth    = whitespaceWidth,
        .finalLetterSpacing = finalLetterSpacing,
        .finalLineSpacing   = font.getLineSpacing(characterSize) * lineSpacing,
    };
}


////////////////////////////////////////////////////////////
/// \brief Count the number of textured quads needed to render a string
///
/// The returned value is the number of quads `createTextGeometryAndGetBounds`
/// will emit for the same string and styling -- one quad per visible glyph,
/// plus an extra quad per non-empty line for each enabled decoration
/// (underline / strikethrough).
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline base::SizeT precomputeTextQuadCount(const Utf8String& string, const bool isUnderlined, const bool isStrikeThrough)
{
    SFML_BASE_ASSERT(!string.empty());

    const base::SizeT linesPerNewline = base::SizeT{isUnderlined} + base::SizeT{isStrikeThrough};

    base::SizeT result          = 0u;
    char32_t    prevChar        = 0;
    bool        lineHasContents = false;

    string.forCodepoints([&] [[gnu::always_inline]] (const char32_t curChar)
    {
        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            return;

        if (curChar == U'\n')
        {
            // A mid-string newline emits a decoration line for the line just ended.
            // Consecutive newlines (empty lines) skip emission.
            if (prevChar != U'\n')
                result += linesPerNewline;

            lineHasContents = false;
        }
        else
        {
            // Whitespace doesn't add a glyph quad but still counts as line content
            // for the purposes of trailing-line decoration emission.
            if (curChar != U' ' && curChar != U'\t')
                ++result;

            lineHasContents = true;
        }

        prevChar = curChar;
    });

    if (lineHasContents)
        result += linesPerNewline;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Convenience overload that reads styling from a `TextData`
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline base::SizeT precomputeTextQuadCount(const TextData& textData)
{
    if (textData.string.empty())
        return 0u;

    return precomputeTextQuadCount(textData.string, textData.underlined, textData.strikeThrough);
}


////////////////////////////////////////////////////////////
// Add an underline or strikethrough line to the vertex array, optionally pre-transformed
template <typename F>
[[gnu::always_inline]] inline void addLineImpl(
    F&&                              fTransform,
    Vertex* const SFML_BASE_RESTRICT vertices,
    base::SizeT&                     index,
    const float                      lineLength,
    const float                      lineTop,
    const Color                      color,
    const float                      offset,
    const float                      thickness,
    const float                      outlineThickness)
{
    const float top    = SFML_BASE_MATH_FLOORF(lineTop + offset - (thickness / 2.f) + 0.5f);
    const float bottom = top + SFML_BASE_MATH_FLOORF(thickness + 0.5f);

    auto* ptr = vertices + index;

    *ptr++ = {fTransform({-outlineThickness, top - outlineThickness}), color, {1.f, 1.f}};
    *ptr++ = {fTransform({lineLength + outlineThickness, top - outlineThickness}), color, {1.f, 1.f}};
    *ptr++ = {fTransform({-outlineThickness, bottom + outlineThickness}), color, {1.f, 1.f}};
    *ptr++ = {fTransform({lineLength + outlineThickness, bottom + outlineThickness}), color, {1.f, 1.f}};

    index += 4u;
}


////////////////////////////////////////////////////////////
// Add a glyph quad to the vertex array, optionally pre-transformed
template <typename F>
[[gnu::always_inline]] inline void addGlyphQuadImpl(
    F&&                              fTransform,
    Vertex* const SFML_BASE_RESTRICT vertices,
    base::SizeT&                     index,
    const Vec2f                      position,
    const Color                      color,
    const Glyph&                     glyph,
    const float                      italicShear)
{
    constexpr Vec2f padding{1.f, 1.f};

    const Vec2f p1 = glyph.bounds.position - padding;
    const Vec2f p2 = glyph.bounds.position + glyph.bounds.size + padding;

    const auto uv1 = glyph.textureRect.position - padding;
    const auto uv2 = (glyph.textureRect.position + glyph.textureRect.size) + padding;

    auto* ptr = vertices + index;

    *ptr++ = {fTransform(position + Vec2f{p1.x - italicShear * p1.y, p1.y}), color, {uv1.x, uv1.y}};
    *ptr++ = {fTransform(position + Vec2f{p2.x - italicShear * p1.y, p1.y}), color, {uv2.x, uv1.y}};
    *ptr++ = {fTransform(position + Vec2f{p1.x - italicShear * p2.y, p2.y}), color, {uv1.x, uv2.y}};
    *ptr++ = {fTransform(position + Vec2f{p2.x - italicShear * p2.y, p2.y}), color, {uv2.x, uv2.y}};

    index += 4u;
}


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr Vec2f identityTransformFn(const Vec2f v) noexcept
{
    return v;
}


////////////////////////////////////////////////////////////
inline void addLine(Vertex* const SFML_BASE_RESTRICT vertices,
                    base::SizeT&                     index,
                    const float                      lineLength,
                    const float                      lineTop,
                    const Color                      color,
                    const float                      offset,
                    const float                      thickness,
                    const float                      outlineThickness)
{
    addLineImpl(identityTransformFn, vertices, index, lineLength, lineTop, color, offset, thickness, outlineThickness);
}


////////////////////////////////////////////////////////////
inline void addLinePreTransformed(
    const Transform&                 transform,
    Vertex* const SFML_BASE_RESTRICT vertices,
    base::SizeT&                     index,
    const float                      lineLength,
    const float                      lineTop,
    const Color                      color,
    const float                      offset,
    const float                      thickness,
    const float                      outlineThickness)
{
    addLineImpl([&] [[gnu::always_inline]] (const Vec2f v) {
        return transform.transformPoint(v);
    }, vertices, index, lineLength, lineTop, color, offset, thickness, outlineThickness);
}


////////////////////////////////////////////////////////////
inline void addGlyphQuad(Vertex* const SFML_BASE_RESTRICT vertices,
                         base::SizeT&                     index,
                         const Vec2f                      position,
                         const Color                      color,
                         const Glyph&                     glyph,
                         const float                      italicShear)
{
    addGlyphQuadImpl(identityTransformFn, vertices, index, position, color, glyph, italicShear);
}


////////////////////////////////////////////////////////////
inline void addGlyphQuadPreTransformed(
    const Transform&                 transform,
    Vertex* const SFML_BASE_RESTRICT vertices,
    base::SizeT&                     index,
    const Vec2f                      position,
    const Color                      color,
    const Glyph&                     glyph,
    const float                      italicShear)
{
    addGlyphQuadImpl([&] [[gnu::always_inline]] (const Vec2f v) {
        return transform.transformPoint(v);
    }, vertices, index, position, color, glyph, italicShear);
}


////////////////////////////////////////////////////////////
/// \brief Walk a string laying out glyphs, optionally emitting quads and/or computing bounds
///
/// The two callbacks are invoked once per emitted quad. Their signatures are:
///
///     fAddLine(idx&, lineLength, lineTop, offset, thickness, outlineThickness, isOutline)
///     fAddGlyphQuad(idx&, position, glyph, italicShear, isOutline)
///
/// The `isOutline` flag tells the callback which color to use; this lets the
/// layout function stay color-agnostic. Pass no-op lambdas to use this purely
/// for bounds computation (see `precomputeTextLocalBounds`).
///
////////////////////////////////////////////////////////////
template <bool CalculateBounds, typename TFontSource>
inline auto createTextGeometryAndGetBounds(
    const base::SizeT       outlineVertexCount,
    const TFontSource&      font,
    const Utf8String&       string,
    const TextLayoutInputs& inputs,
    auto&&                  fAddLine,
    auto&&                  fAddGlyphQuad)
{
    if (string.empty())
    {
        if constexpr (CalculateBounds)
            return Rect2f{};
        else
            return;
    }

    // Compute values related to the text style
    const float italicShear        = inputs.italic ? degrees(12).asRadians() : 0.f;
    const float underlineOffset    = font.getUnderlinePosition(inputs.characterSize);
    const float underlineThickness = font.getUnderlineThickness(inputs.characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    const float strikeThroughOffset = inputs.strikeThrough
                                          ? font.getGlyph(U'x', inputs.characterSize, inputs.bold, /* outlineThickness */ 0.f)
                                                .bounds.getCenter()
                                                .y
                                          : 0.f;

    // Precompute the variables needed by the algorithm
    const auto [whitespaceWidth,
                finalLetterSpacing,
                finalLineSpacing] = precomputeSpacingConstants(font,
                                                               inputs.bold,
                                                               inputs.characterSize,
                                                               inputs.letterSpacing,
                                                               inputs.lineSpacing);

    base::SizeT currFillIndex    = outlineVertexCount;
    base::SizeT currOutlineIndex = 0u;

    float x = 0.f;
    auto  y = static_cast<float>(inputs.characterSize);

    // Bounds state (only updated if `CalculateBounds` is `true`)
    [[maybe_unused]] auto  minX = static_cast<float>(inputs.characterSize);
    [[maybe_unused]] auto  minY = static_cast<float>(inputs.characterSize);
    [[maybe_unused]] float maxX = 0.f;
    [[maybe_unused]] float maxY = 0.f;

    char32_t prevChar = 0;

    const auto addLines = [&](const float offset)
    {
        fAddLine(currFillIndex, x, y, offset, underlineThickness, /* outlineThickness */ 0.f, /* isOutline */ false);

        if (inputs.outlineThickness != 0.f)
            fAddLine(currOutlineIndex, x, y, offset, underlineThickness, inputs.outlineThickness, /* isOutline */ true);
    };

    const auto updateBoundsAndAdvance = [&] [[gnu::always_inline]] (const Glyph& fillGlyph)
    {
        if constexpr (CalculateBounds)
        {
            const Vec2f p1 = fillGlyph.bounds.position;
            const Vec2f p2 = fillGlyph.bounds.position + fillGlyph.bounds.size;

            const float newMinX = x + p1.x - italicShear * p2.y;
            const float newMaxX = x + p2.x - italicShear * p1.y;
            const float newMinY = y + p1.y;
            const float newMaxY = y + p2.y;

            minX = SFML_BASE_MIN(minX, newMinX);
            maxX = SFML_BASE_MAX(maxX, newMaxX);
            minY = SFML_BASE_MIN(minY, newMinY);
            maxY = SFML_BASE_MAX(maxY, newMaxY);
        }

        x += fillGlyph.advance + finalLetterSpacing;
    };

    string.forCodepoints([&] [[gnu::always_inline]] (const char32_t curChar)
    {
        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            return;

        // Apply the kerning offset
        x += font.getKerning(prevChar, curChar, inputs.characterSize, inputs.bold);

        if (curChar == U'\n' && prevChar != U'\n')
        {
            // If we're using the underlined style and there's a new line, draw a line
            if (inputs.underlined)
                addLines(underlineOffset);

            // If we're using the strike through style and there's a new line, draw a line across all characters
            if (inputs.strikeThrough)
                addLines(strikeThroughOffset);
        }

        prevChar = curChar;

        // Handle special characters
        if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t'))
        {
            // Update the current bounds (min coordinates)
            if constexpr (CalculateBounds)
            {
                minX = SFML_BASE_MIN(minX, x);
                minY = SFML_BASE_MIN(minY, y);
            }

            switch (curChar)
            {
                case U' ':
                    x += whitespaceWidth;
                    break;
                case U'\t':
                    x += whitespaceWidth * 4;
                    break;
                case U'\n':
                    y += finalLineSpacing;
                    x = 0.f;
                    break;
            }

            // Update the current bounds (max coordinates)
            if constexpr (CalculateBounds)
            {
                maxX = SFML_BASE_MAX(maxX, x);
                maxY = SFML_BASE_MAX(maxY, y);
            }

            // Next glyph, no need to create a quad for whitespace
            return;
        }

        if (inputs.outlineThickness == 0.f)
        {
            const Glyph& fillGlyph = font.getGlyph(curChar, inputs.characterSize, inputs.bold, /* outlineThickness */ 0.f);
            fAddGlyphQuad(currFillIndex, Vec2f{x, y}, fillGlyph, italicShear, /* isOutline */ false);

            updateBoundsAndAdvance(fillGlyph);
        }
        else
        {
            const auto& [fillGlyph,
                         outlineGlyph] = font.getFillAndOutlineGlyph(curChar, inputs.characterSize, inputs.bold, inputs.outlineThickness);

            fAddGlyphQuad(currFillIndex, Vec2f{x, y}, fillGlyph, italicShear, /* isOutline */ false);
            fAddGlyphQuad(currOutlineIndex, Vec2f{x, y}, outlineGlyph, italicShear, /* isOutline */ true);

            updateBoundsAndAdvance(fillGlyph);
        }
    });

    // If we're using outline, update the current bounds
    if constexpr (CalculateBounds)
    {
        if (inputs.outlineThickness != 0.f)
        {
            const float outline = SFML_BASE_MATH_CEILF(SFML_BASE_MATH_FABSF(inputs.outlineThickness));
            minX -= outline;
            maxX += outline;
            minY -= outline;
            maxY += outline;
        }
    }

    // If we're using the underlined style, add the last line
    if (inputs.underlined && (x > 0))
        addLines(underlineOffset);

    // If we're using the strike through style, add the last line across all characters
    if (inputs.strikeThrough && (x > 0))
        addLines(strikeThroughOffset);

    if constexpr (CalculateBounds)
    {
        return Rect2f{{minX, minY}, {maxX - minX, maxY - minY}};
    }
}


////////////////////////////////////////////////////////////
/// \brief Compute the local bounding box of a piece of text without rendering it
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline Rect2f precomputeTextLocalBounds(const Font& font, const TextData& textData)
{
    return createTextGeometryAndGetBounds</* CalculateBounds */ true>(
        /* outlineVertexCount */ 0u,
        font,
        textData.string,
        makeTextLayoutInputs(textData),
        /* fAddLine */ [] [[gnu::always_inline]] (auto&&...) {},
        /* fAddGlyphQuad */ [] [[gnu::always_inline]] (auto&&...) {});
}


////////////////////////////////////////////////////////////
/// \brief Compute the global (post-transform) bounding box of a piece of text
///
/// Applies the `position`, `scale`, `origin` and `rotation` from `textData` to
/// the local bounds computed by `precomputeTextLocalBounds`.
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline Rect2f precomputeTextGlobalBounds(const Font& font, const TextData& textData)
{
    return Transform::fromPositionScaleOriginRotation(textData.position, textData.scale, textData.origin, textData.rotation)
        .transformRect(precomputeTextLocalBounds(font, textData));
}


////////////////////////////////////////////////////////////
/// \brief Compute the `origin` value that aligns a given anchor of `textData` to its position
///
/// `anchor` is in normalized rect-anchor coordinates: `{0, 0}` is the top-left
/// of the local bounds, `{1, 1}` the bottom-right, `{0.5f, 0.f}` the
/// top-center, and so on (matching `sf::Rect2::getAnchorPoint`).
///
/// Typical use:
///
/// \code
/// textData.origin   = sf::TextUtils::computeAnchorOrigin(font, textData, {0.5f, 0.f});
/// textData.position = {centerX, y};
/// rt.draw(font, textData);
/// \endcode
///
/// Unlike `bounds.size * anchor`, this also accounts for `bounds.position`,
/// so styled or italic text remains correctly anchored.
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline Vec2f computeAnchorOrigin(const Font& font, const TextData& textData, const Vec2f anchor)
{
    return precomputeTextLocalBounds(font, textData).getAnchorPoint(anchor);
}


////////////////////////////////////////////////////////////
/// \brief Return a copy of `textData` with its `origin` set to anchor it at `textData.position`
///
/// Convenience wrapper around `computeAnchorOrigin` that lets the caller draw
/// anchored text in a single statement:
///
/// \code
/// rt.draw(font, sf::TextUtils::anchored(font, {
///     .position      = {centerX, y},
///     .string        = "...",
///     .characterSize = 24u,
/// }, {0.5f, 0.f}));
/// \endcode
///
/// To inset the text from its anchor by some pixel amount, fold the offset
/// into `position` (subtracting from `position` is equivalent to adding to
/// `origin`, since rendering uses `position - origin`).
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline TextData anchored(const Font& font, TextData textData, const Vec2f anchor)
{
    textData.origin = computeAnchorOrigin(font, textData, anchor);
    return textData;
}

} // namespace sf::TextUtils


////////////////////////////////////////////////////////////
/// \namespace sf::TextUtils
/// \ingroup graphics
///
/// \brief Utility functions for text processing and geometry generation.
///
/// This namespace provides a collection of helper functions tailored for
/// advanced text manipulation within SFML. These functions handle tasks
/// such as calculating the bounding box of a string (`precomputeTextLocalBounds`,
/// `precomputeTextGlobalBounds`), determining the number of quads required to
/// render a given text (`precomputeTextQuadCount`), generating the vertex data
/// for text rendering, including handling styles like underline and
/// strikethrough, as well as character and line spacing
/// (`createTextGeometryAndGetBounds`, `addGlyphQuad`, `addLine`), and
/// computing anchor offsets for centered/aligned drawing
/// (`computeAnchorOrigin`).
///
/// While these utilities are predominantly used internally by the `sf::Text`
/// class to manage and render text, they are exposed for users who might
/// need to perform custom text layout, implement specialized text rendering
/// techniques, or gain deeper insights into text geometry for effects or
/// editor tools.
///
/// \see sf::Text, sf::Font
///
////////////////////////////////////////////////////////////
