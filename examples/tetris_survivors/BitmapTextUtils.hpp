#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BitmapFont.hpp"
#include "BitmapTextAlignment.hpp"

#include "Zancle/Graphics/IndexType.hpp"
#include "Zancle/Graphics/Transform.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/String/FromChars.hpp"
#include "Zancle/String/StringView.hpp"

#include "Zancle/Container/InPlaceVector.hpp"
#include "Zancle/Container/Vector.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"

#include "Zancle/Math/MinMax.hpp"
#include "Zancle/Math/Sin.hpp"

#include "Zancle/Base/SizeT.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] BitmapTextToVerticesOptions // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    za::Vector<za::Vertex>&    outVertices;
    za::Vector<za::IndexType>& outIndices;
    const BitmapFont&          bitmapFont;
    za::Rect2f                 fontTextureRect;
    BitmapTextAlignment        alignment;
    za::Color                  baseColor;
    float                      time;
    za::StringView             string;
};


////////////////////////////////////////////////////////////
template <bool TBoundsOnly = false>
inline auto bitmapTextToVertices(const BitmapTextToVerticesOptions& options)
{
    const auto& [outVertices, outIndices, bitmapFont, fontTextureRect, alignment, baseColor, time, str] = options;

    struct [[nodiscard]] Wobble
    {
        float frequency;
        float amplitude;
        float phase;
    };

    struct [[nodiscard]] FormattingState // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        za::Color color;
        Wobble    wobble;
        bool      bold;
        float     hSpace;
        float     vSpace;
    };

    using za::StringView::nPos;

    const auto parseArg = [](za::StringView& args, auto& outValue)
    {
        const auto delimiterPos = args.find(',');
        const auto segment      = (delimiterPos == nPos) ? args : args.substrByPosLen(0, delimiterPos);

        [[maybe_unused]] const auto [ptr, ec] = za::fromChars(segment.data(), segment.data() + segment.size(), outValue);
        ZA_ASSERT(ec == za::FromCharsError::None);

        args.removePrefix((delimiterPos == nPos) ? args.size() : delimiterPos + 1);
    };

    const auto parseText = [&](auto&& onChar)
    {
        za::InPlaceVector<FormattingState, 16> formattingStack{{
            .color  = baseColor,
            .wobble = {.frequency = 0.f, .amplitude = 0.f, .phase = 0.f},
            .bold   = false,
            .hSpace = 0.f,
            .vSpace = 0.f,
        }};

        for (za::SizeT i = 0u; i < str.size(); ++i)
        {
            if (str[i] == '^')
            {
                const auto funcEnd   = str.find('[', i + 1);
                const auto argsEnd   = str.find(']', funcEnd);
                const auto textStart = str.find('(', argsEnd);

                if (funcEnd != nPos && argsEnd != nPos && textStart != nPos)
                {
                    const auto funcName = str.substrByPosLen(i + 1, funcEnd - (i + 1));

                    // Create a new state based on the previous one. This correctly inherits properties.
                    auto newState   = formattingStack.back();
                    bool tagHandled = false;

                    if (funcName == "color")
                    {
                        auto args = str.substrByPosLen(funcEnd + 1, argsEnd - (funcEnd + 1));

                        parseArg(args, newState.color.r);
                        parseArg(args, newState.color.g);
                        parseArg(args, newState.color.b);

                        tagHandled = true;
                    }
                    else if (funcName == "wobble")
                    {
                        auto args = str.substrByPosLen(funcEnd + 1, argsEnd - (funcEnd + 1));

                        parseArg(args, newState.wobble.frequency);
                        parseArg(args, newState.wobble.amplitude);
                        parseArg(args, newState.wobble.phase);

                        tagHandled = true;
                    }
                    else if (funcName == "bold")
                    {
                        newState.bold = true;
                        tagHandled    = true;
                    }
                    else if (funcName == "hspace")
                    {
                        auto args = str.substrByPosLen(funcEnd + 1, argsEnd - (funcEnd + 1));

                        parseArg(args, newState.hSpace);

                        tagHandled = true;
                    }
                    else if (funcName == "vspace")
                    {
                        auto args = str.substrByPosLen(funcEnd + 1, argsEnd - (funcEnd + 1));

                        parseArg(args, newState.vSpace);

                        tagHandled = true;
                    }

                    if (tagHandled)
                    {
                        formattingStack.emplaceBack(newState);
                        i = textStart; // Jump cursor past the tag definition

                        continue;
                    }
                }

                // If tag is malformed or unknown, fall through to treat '^' as a literal.
            }
            else if (str[i] == ')' && i + 1 < str.size() && str[i + 1] == '^') // Check for ")^"
            {
                ZA_ASSERT(formattingStack.size() > 1);
                formattingStack.popBack();

                i += 1; // Jump cursor past the ")^" sequence
            }
            else
            {
                onChar(i, str[i], formattingStack.back());
            }
        }
    };

    // Measure line widths

    za::InPlaceVector<za::SizeT, 64> linePixelWidths;
    linePixelWidths.emplaceBack(0u); // Width of the first line

    za::SizeT maxPixelWidth = 0;

    za::Vec2f maxs;

    const auto [hSpacing, vSpacing] = bitmapFont.getGlyphSize('i');

    parseText([&](const za::SizeT /* charIdx */, const char c, const FormattingState& fs)
    {
        if (c == '\n')
        {
            maxPixelWidth = za::max(maxPixelWidth, linePixelWidths.back());
            linePixelWidths.emplaceBack(0u);
            return;
        }

        if (c == '\t')
        {
            linePixelWidths.back() += hSpacing * 4;
            return;
        }

        if (c == ' ')
        {
            linePixelWidths.back() += hSpacing;
            return;
        }

        linePixelWidths.back() += (fs.bold ? hSpacing + 1 : hSpacing);
    });

    maxPixelWidth = za::max(maxPixelWidth, linePixelWidths.back());

    // Generate vertices

    za::Vec2f cursor         = {0.f, 0.f};
    za::SizeT currentLineIdx = 0u;

    const auto getAlignedX = [&](const za::SizeT lineIdx)
    {
        if (alignment == BitmapTextAlignment::Center)
            return (static_cast<float>(maxPixelWidth - linePixelWidths[lineIdx])) / 2.f;

        if (alignment == BitmapTextAlignment::Right)
            return static_cast<float>(maxPixelWidth - linePixelWidths[lineIdx]);

        ZA_ASSERT(alignment == BitmapTextAlignment::Left);
        return 0.f;
    };

    cursor.x = getAlignedX(0u);

    const auto emitQuad =
        [&](const za::Vec2f position, const za::Vec2f size, const za::Rect2f& textureRect, const za::Color color)
    {
        const auto baseIndex = static_cast<za::IndexType>(outVertices.size());

        outVertices.emplaceBack(position, color, textureRect.getTopLeft());
        outVertices.emplaceBack(position.addX(size.x), color, textureRect.getTopRight());
        outVertices.emplaceBack(position + size, color, textureRect.getBottomRight());
        outVertices.emplaceBack(position.addY(size.y), color, textureRect.getBottomLeft());

        outIndices.pushBackMultiple(baseIndex + 0u, baseIndex + 1u, baseIndex + 2u, baseIndex + 0u, baseIndex + 2u, baseIndex + 3u);
    };

    parseText([&](const za::SizeT charIdx, const char c, const FormattingState& fs)
    {
        if (c == '\n')
        {
            cursor.y += static_cast<float>(vSpacing);

            ++currentLineIdx;

            if (currentLineIdx < linePixelWidths.size())
                cursor.x = getAlignedX(currentLineIdx);

            return;
        }

        if (c == '\t')
        {
            cursor.x += static_cast<float>(hSpacing) * 4.f;
            return;
        }

        if (c == ' ')
        {
            cursor.x += static_cast<float>(hSpacing);
            return;
        }

        const auto texRect = bitmapFont.getGlyphTextureRect(fontTextureRect, c);

        const auto wobbleAmount = fs.wobble.amplitude *
                                  za::sin(fs.wobble.frequency * time + static_cast<float>(charIdx) * fs.wobble.phase);

        const auto fGlyphSize = bitmapFont.getGlyphSize(c).toVec2f();

        const auto adjustedCursor = cursor.addX(fs.hSpace).addY(wobbleAmount + fs.vSpace);

        if constexpr (!TBoundsOnly)
        {
            emitQuad(adjustedCursor, fGlyphSize, texRect, fs.color);

            if (fs.bold)
                emitQuad(adjustedCursor.addX(1), fGlyphSize, texRect, fs.color);
        }

        cursor.x += fs.bold ? fGlyphSize.x + 1.f : fGlyphSize.x;

        maxs.x = za::max(maxs.x, cursor.x + fs.hSpace);
        maxs.y = za::max(maxs.y, cursor.y + fs.vSpace + fGlyphSize.y);
    });

    return maxs;
}


////////////////////////////////////////////////////////////
inline za::Rect2f bitmapTextToVerticesPretransformed(const BitmapTextToVerticesOptions& options, const za::Transform& transform)
{
    const auto prevVerticesSize = options.outVertices.size();
    const auto localBoundsSize  = bitmapTextToVertices</* TBoundsOnly */ false>(options);

    for (za::SizeT i = prevVerticesSize; i < options.outVertices.size(); ++i)
        options.outVertices[i].position = transform.transformPoint(options.outVertices[i].position);

    return transform.transformRect({{0.f, 0.f}, localBoundsSize});
}

} // namespace tsurv
