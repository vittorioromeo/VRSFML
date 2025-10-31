#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Array.hpp"


namespace tsurv
{
//////////////////////////////////////////////////////////////
class [[nodiscard]] MonospaceBitmapFont
{
private:
    sf::base::Array<sf::Vec2uz, 256> m_glyphPositions;
    sf::Vec2uz                       m_glyphSize;

public:
    //////////////////////////////////////////////////////////////
    explicit MonospaceBitmapFont(const sf::Vec2uz glyphSize) : m_glyphSize{glyphSize}
    {
    }

    //////////////////////////////////////////////////////////////
    void addGlyph(const char c, const sf::Vec2uz position)
    {
        m_glyphPositions[static_cast<sf::base::SizeT>(c)] = position;
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2uz getGlyphRect(const char c) const
    {
        const sf::Vec2uz glyphPos = m_glyphPositions[static_cast<sf::base::SizeT>(c)];

        return {
            .position = glyphPos.componentWiseMul(m_glyphSize),
            .size     = m_glyphSize,
        };
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f getGlyphTextureRect(const sf::Rect2f& fontTextureRect, const char c) const
    {
        const auto glyphRect = getGlyphRect(c).toRect2f();

        return {
            .position = fontTextureRect.position + glyphRect.position.toVec2f(),
            .size     = glyphRect.size.toVec2f(),
        };
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2uz getGlyphSize() const
    {
        return m_glyphSize;
    }
};

} // namespace tsurv
