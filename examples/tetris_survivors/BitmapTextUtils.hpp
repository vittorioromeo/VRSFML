#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BitmapTextAlignment.hpp"
#include "MonospaceBitmapFont.hpp"

#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/FromChars.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
//////////////////////////////////////////////////////////////
struct [[nodiscard]] MonospaceBitmapTextToVerticesOptions // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    sf::base::Vector<sf::Vertex>&    outVertices;
    sf::base::Vector<sf::IndexType>& outIndices;
    const MonospaceBitmapFont&       monospaceBitmapFont;
    sf::Rect2f                       fontTextureRect;
    BitmapTextAlignment              alignment;
    sf::Color                        baseColor;
    float                            time;
    sf::base::StringView             string;
};


//////////////////////////////////////////////////////////////
inline void monospaceBitmapTextToVertices(const MonospaceBitmapTextToVerticesOptions& options)
{
    const auto& [outVertices, outIndices, monospaceBitmapFont, fontTextureRect, alignment, baseColor, time, str] = options;

    struct [[nodiscard]] Wobble
    {
        float frequency;
        float amplitude;
        float phase;
    };

    struct [[nodiscard]] FormattingState // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        sf::Color color;
        Wobble    wobble;
        bool      bold;
    };

    using sf::base::StringView::nPos;

    const auto parseArg = [](sf::base::StringView& args, auto& outValue)
    {
        const auto delimiterPos = args.find(',');
        const auto segment      = (delimiterPos == nPos) ? args : args.substrByPosLen(0, delimiterPos);

        [[maybe_unused]] const auto [ptr, ec] = sf::base::fromChars(segment.data(), segment.data() + segment.size(), outValue);
        SFML_BASE_ASSERT(ec == sf::base::FromCharsError::None);

        args.removePrefix((delimiterPos == nPos) ? args.size() : delimiterPos + 1);
    };

    const auto parseText = [&](auto&& onChar)
    {
        sf::base::InPlaceVector<FormattingState, 16> formattingStack{{
            .color  = baseColor,
            .wobble = {.frequency = 0.f, .amplitude = 0.f, .phase = 0.f},
            .bold   = false,
        }};

        for (sf::base::SizeT i = 0u; i < str.size(); ++i)
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

                        parseArg(args, newState.wobble.amplitude);
                        parseArg(args, newState.wobble.frequency);
                        parseArg(args, newState.wobble.phase);

                        tagHandled = true;
                    }
                    else if (funcName == "bold")
                    {
                        newState.bold = true;
                        tagHandled    = true;
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
                SFML_BASE_ASSERT(formattingStack.size() > 1);
                formattingStack.popBack();

                i += 1; // Jump cursor past the ")^" sequence
            }
            else
            {
                onChar(i, str[i], formattingStack.back());
            }
        }
    };

    // --- Pass 1: Measure line widths without allocating glyph structures ---

    sf::base::InPlaceVector<sf::base::SizeT, 64> linePixelWidths;
    linePixelWidths.emplaceBack(0u); // Width of the first line

    sf::base::SizeT maxPixelWidth = 0;
    const auto      glyphSize     = monospaceBitmapFont.getGlyphSize();

    parseText([&](const sf::base::SizeT /* charIdx */, const char c, const FormattingState& fs)
    {
        if (c == '\n')
        {
            maxPixelWidth = sf::base::max(maxPixelWidth, linePixelWidths.back());
            linePixelWidths.emplaceBack(0u);
            return;
        }

        linePixelWidths.back() += (c == '\t') ? glyphSize.x * 4 : (fs.bold ? glyphSize.x + 1 : glyphSize.x);
    });

    maxPixelWidth = sf::base::max(maxPixelWidth, linePixelWidths.back());


    // --- Pass 2: Generate vertices directly ---

    sf::Vec2f       cursor         = {0.f, 0.f};
    sf::base::SizeT currentLineIdx = 0u;

    const auto getAlignedX = [&](const sf::base::SizeT lineIdx)
    {
        switch (alignment)
        {
            case BitmapTextAlignment::Center:
                return (static_cast<float>(maxPixelWidth - linePixelWidths[lineIdx])) / 2.f;
            case BitmapTextAlignment::Right:
                return static_cast<float>(maxPixelWidth - linePixelWidths[lineIdx]);
            case BitmapTextAlignment::Left:
                return 0.f;
        }
    };

    cursor.x = getAlignedX(0u);

    const auto emitQuad =
        [&](const sf::Vec2f position, const sf::Vec2f size, const sf::Rect2f& textureRect, const sf::Color color)
    {
        const auto baseIndex = static_cast<sf::IndexType>(outVertices.size());

        outVertices.emplaceBack(position, color, textureRect.getTopLeft());
        outVertices.emplaceBack(position.addX(size.x), color, textureRect.getTopRight());
        outVertices.emplaceBack(position + size, color, textureRect.getBottomRight());
        outVertices.emplaceBack(position.addY(size.y), color, textureRect.getBottomLeft());

        outIndices.pushBackMultiple(baseIndex + 0u, baseIndex + 1u, baseIndex + 2u, baseIndex + 0u, baseIndex + 2u, baseIndex + 3u);
    };

    const auto fGlyphSize = glyphSize.toVec2f();

    parseText([&](const sf::base::SizeT charIdx, const char c, const FormattingState& fs)
    {
        if (c == '\n')
        {
            cursor.y += static_cast<float>(glyphSize.y);

            ++currentLineIdx;

            if (currentLineIdx < linePixelWidths.size())
                cursor.x = getAlignedX(currentLineIdx);

            return;
        }

        if (c != ' ' && c != '\t')
        {
            const auto texRect = monospaceBitmapFont.getGlyphTextureRect(fontTextureRect, c);

            const auto wobbleAmount = fs.wobble.amplitude * sf::base::sin(fs.wobble.frequency * time +
                                                                          static_cast<float>(charIdx) * fs.wobble.phase);

            emitQuad(cursor.addY(wobbleAmount), fGlyphSize, texRect, fs.color);

            if (fs.bold)
                emitQuad(cursor.addY(wobbleAmount).addX(1), fGlyphSize, texRect, fs.color);
        }

        // Advance cursor
        cursor.x += (c == '\t') ? fGlyphSize.x * 4.f : (fs.bold ? fGlyphSize.x + 1.f : fGlyphSize.x);
    });
}

} // namespace tsurv
