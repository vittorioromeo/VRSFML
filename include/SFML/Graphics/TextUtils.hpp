#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/String.hpp"

#include "SFML/Base/Builtins/Restrict.hpp"
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
[[nodiscard]] inline TextSpacingConstants precomputeSpacingConstants(
    const Font&        font,
    const TextStyle    style,
    const unsigned int characterSize,
    const float        letterSpacing,
    const float        lineSpacing)
{
    const bool isBold = !!(style & TextStyle::Bold);

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
[[nodiscard]] inline base::SizeT precomputeTextQuadCount(const String& string, const TextStyle style)
{
    SFML_BASE_ASSERT(!string.isEmpty());

    base::SizeT result = 0u;

    // Compute values related to the text style
    const bool isUnderlined    = !!(style & TextStyle::Underlined);
    const bool isStrikeThrough = !!(style & TextStyle::StrikeThrough);

    char32_t prevChar        = 0;
    bool     lineHasContents = false;

    for (const char32_t curChar : string)
    {
        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            continue;

        // Apply the kerning offset
        lineHasContents = true;

        // If we're using the underlined or strike through style and there's a new line, draw a line
        if ((curChar == U'\n' && prevChar != U'\n') && (isUnderlined || isStrikeThrough))
            ++result;

        prevChar = curChar;

        // Handle special characters
        switch (curChar)
        {
            case U' ':
                [[fallthrough]];
            case U'\t':
                lineHasContents = true;
                continue;
            case U'\n':
                lineHasContents = false;
                continue;
        }

        // Apply the outline
        ++result;

        // Advance to the next character
        lineHasContents = true;
    }

    // If we're using the underlined style, add the last line
    if (isUnderlined && lineHasContents)
        ++result;

    // If we're using the strike through style, add the last line across all characters
    if (isStrikeThrough && lineHasContents)
        ++result;

    return result;
}


////////////////////////////////////////////////////////////
// Add an underline or strikethrough line to the vertex array
inline void addLine(Vertex* const SFML_BASE_RESTRICT vertices,
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

    *ptr++ = {{-outlineThickness, top - outlineThickness}, color, {1.f, 1.f}};
    *ptr++ = {{lineLength + outlineThickness, top - outlineThickness}, color, {1.f, 1.f}};
    *ptr++ = {{-outlineThickness, bottom + outlineThickness}, color, {1.f, 1.f}};
    *ptr++ = {{lineLength + outlineThickness, bottom + outlineThickness}, color, {1.f, 1.f}};

    index += 4u;
}


////////////////////////////////////////////////////////////
// Add a glyph quad to the vertex array
inline void addGlyphQuad(Vertex* const SFML_BASE_RESTRICT vertices,
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

    *ptr++ = {position + Vec2f{p1.x - italicShear * p1.y, p1.y}, color, {uv1.x, uv1.y}};
    *ptr++ = {position + Vec2f{p2.x - italicShear * p1.y, p1.y}, color, {uv2.x, uv1.y}};
    *ptr++ = {position + Vec2f{p1.x - italicShear * p2.y, p2.y}, color, {uv1.x, uv2.y}};
    *ptr++ = {position + Vec2f{p2.x - italicShear * p2.y, p2.y}, color, {uv2.x, uv2.y}};

    index += 4u;
}


////////////////////////////////////////////////////////////
// Add an underline or strikethrough line to the vertex array (pre-transformed)
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
    const float top    = SFML_BASE_MATH_FLOORF(lineTop + offset - (thickness / 2.f) + 0.5f);
    const float bottom = top + SFML_BASE_MATH_FLOORF(thickness + 0.5f);

    auto* ptr = vertices + index;

    *ptr++ = {transform.transformPoint({-outlineThickness, top - outlineThickness}), color, {1.f, 1.f}};
    *ptr++ = {transform.transformPoint({lineLength + outlineThickness, top - outlineThickness}), color, {1.f, 1.f}};
    *ptr++ = {transform.transformPoint({-outlineThickness, bottom + outlineThickness}), color, {1.f, 1.f}};
    *ptr++ = {transform.transformPoint({lineLength + outlineThickness, bottom + outlineThickness}), color, {1.f, 1.f}};

    index += 4u;
}


////////////////////////////////////////////////////////////
// Add a glyph quad to the vertex array (pre-transformed)
inline void addGlyphQuadPreTransformed(
    const Transform&                 transform,
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

    *ptr++ = {transform.transformPoint(position + Vec2f{p1.x - italicShear * p1.y, p1.y}), color, {uv1.x, uv1.y}};
    *ptr++ = {transform.transformPoint(position + Vec2f{p2.x - italicShear * p1.y, p1.y}), color, {uv2.x, uv1.y}};
    *ptr++ = {transform.transformPoint(position + Vec2f{p1.x - italicShear * p2.y, p2.y}), color, {uv1.x, uv2.y}};
    *ptr++ = {transform.transformPoint(position + Vec2f{p2.x - italicShear * p2.y, p2.y}), color, {uv2.x, uv2.y}};

    index += 4u;
}


////////////////////////////////////////////////////////////
template <bool CalculateBounds>
inline auto createTextGeometryAndGetBounds(
    const base::SizeT  outlineVertexCount,
    const Font&        font,
    const String&      string,
    const TextStyle    style,
    const unsigned int characterSize,
    const float        letterSpacing,
    const float        lineSpacing,
    const float        outlineThickness,
    const Color        fillColor,    // TODO P1: remove?
    const Color        outlineColor, // TODO P1: remove?
    auto&&             fAddLine,
    auto&&             fAddGlyphQuad)
{
    // Compute values related to the text style
    const bool  isBold             = !!(style & TextStyle::Bold);
    const bool  isUnderlined       = !!(style & TextStyle::Underlined);
    const bool  isStrikeThrough    = !!(style & TextStyle::StrikeThrough);
    const float italicShear        = !!(style & TextStyle::Italic) ? degrees(12).asRadians() : 0.f;
    const float underlineOffset    = font.getUnderlinePosition(characterSize);
    const float underlineThickness = font.getUnderlineThickness(characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    const float strikeThroughOffset = isStrikeThrough
                                          ? font.getGlyph(U'x', characterSize, isBold, /* outlineThickness */ 0.f)
                                                .bounds.getCenter()
                                                .y
                                          : 0.f;

    // Precompute the variables needed by the algorithm
    const auto [whitespaceWidth,
                finalLetterSpacing,
                finalLineSpacing] = precomputeSpacingConstants(font, style, characterSize, letterSpacing, lineSpacing);

    base::SizeT currFillIndex    = outlineVertexCount;
    base::SizeT currOutlineIndex = 0u;

    float x = 0.f;
    auto  y = static_cast<float>(characterSize);

    // Bounds state (only updated if `CalculateBounds` is `true`)
    [[maybe_unused]] auto  minX = static_cast<float>(characterSize);
    [[maybe_unused]] auto  minY = static_cast<float>(characterSize);
    [[maybe_unused]] float maxX = 0.f;
    [[maybe_unused]] float maxY = 0.f;

    char32_t prevChar = 0;

    const auto addLines = [&](float offset)
    {
        fAddLine(currFillIndex, x, y, fillColor, offset, underlineThickness, /* outlineThickness */ 0.f);

        if (outlineThickness != 0.f)
            fAddLine(currOutlineIndex, x, y, outlineColor, offset, underlineThickness, outlineThickness);
    };

    const auto updateBoundsAndAdvance = [&](const Glyph& fillGlyph) SFML_BASE_LAMBDA_ALWAYS_INLINE
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

    for (const char32_t curChar : string)
    {
        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            continue;

        // Apply the kerning offset
        x += font.getKerning(prevChar, curChar, characterSize, isBold);

        if (curChar == U'\n' && prevChar != U'\n')
        {
            // If we're using the underlined style and there's a new line, draw a line
            if (isUnderlined)
                addLines(underlineOffset);

            // If we're using the strike through style and there's a new line, draw a line across all characters
            if (isStrikeThrough)
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
            continue;
        }

        if (outlineThickness == 0.f)
        {
            const Glyph& fillGlyph = font.getGlyph(curChar, characterSize, isBold, /* outlineThickness */ 0.f);
            fAddGlyphQuad(currFillIndex, Vec2f{x, y}, fillColor, fillGlyph, italicShear);

            updateBoundsAndAdvance(fillGlyph);
        }
        else
        {
            const auto& [fillGlyph,
                         outlineGlyph] = font.getFillAndOutlineGlyph(curChar, characterSize, isBold, outlineThickness);

            fAddGlyphQuad(currFillIndex, Vec2f{x, y}, fillColor, fillGlyph, italicShear);
            fAddGlyphQuad(currOutlineIndex, Vec2f{x, y}, outlineColor, outlineGlyph, italicShear);

            updateBoundsAndAdvance(fillGlyph);
        }
    }

    // If we're using outline, update the current bounds
    if constexpr (CalculateBounds)
    {
        if (outlineThickness != 0.f)
        {
            const float outline = SFML_BASE_MATH_FABSF(SFML_BASE_MATH_CEILF(outlineThickness));
            minX -= outline;
            maxX += outline;
            minY -= outline;
            maxY += outline;
        }
    }

    // If we're using the underlined style, add the last line
    if (isUnderlined && (x > 0))
        addLines(underlineOffset);

    // If we're using the strike through style, add the last line across all characters
    if (isStrikeThrough && (x > 0))
        addLines(strikeThroughOffset);

    if constexpr (CalculateBounds)
    {
        return FloatRect{{minX, minY}, {maxX - minX, maxY - minY}};
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline FloatRect precomputeTextLocalBounds(const Font& font, const TextData& textData)
{
    return createTextGeometryAndGetBounds</* CalculateBounds */ true>(
        /* outlineVertexCount */ 0u,
        font,
        textData.string,
        textData.style,
        textData.characterSize,
        textData.letterSpacing,
        textData.lineSpacing,
        textData.outlineThickness,
        /* fillColor */ {},    // doesn't matter
        /* outlineColor */ {}, // doesn't matter
        /* fAddLine */ [](auto&&...) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {},
        /* fAddGlyphQuad */ [](auto&&...) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {});
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline FloatRect precomputeTextLocalBounds(const Font& font, const Text& text)
{
    return createTextGeometryAndGetBounds</* CalculateBounds */ true>(
        /* outlineVertexCount */ 0u,
        font,
        text.getString(),
        text.getStyle(),
        text.getCharacterSize(),
        text.getLetterSpacing(),
        text.getLineSpacing(),
        text.getOutlineThickness(),
        /* fillColor */ {},    // doesn't matter
        /* outlineColor */ {}, // doesn't matter
        /* fAddLine */ [](auto&&...) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {},
        /* fAddGlyphQuad */ [](auto&&...) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {});
}

// TODO P1: precompute globalbounds as well?

} // namespace sf::TextUtils


////////////////////////////////////////////////////////////
/// \namespace sf::TextUtils
/// \ingroup graphics
///
/// \brief Utility functions for text processing and geometry generation.
///
/// This namespace provides a collection of helper functions tailored for
/// advanced text manipulation within SFML. These functions handle tasks
/// such as calculating the bounding box of a string (`precomputeTextLocalBounds`),
/// determining the number of quads required to render a given text
/// (`precomputeTextQuadCount`), and generating the vertex data for
/// text rendering, including handling styles like underline and strikethrough,
/// as well as character and line spacing (`createTextGeometryAndGetBounds`,
/// `addGlyphQuad`, `addLine`).
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
