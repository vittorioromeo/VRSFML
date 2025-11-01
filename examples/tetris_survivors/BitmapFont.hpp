#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Assert.hpp"


namespace tsurv
{
//////////////////////////////////////////////////////////////
class [[nodiscard]] BitmapFont
{
private:
    //////////////////////////////////////////////////////////////
    sf::base::Array<sf::Rect2uz, 256> m_glyphRects;

public:
    //////////////////////////////////////////////////////////////
    void addGlyph(const char c, const sf::Vec2uz position, const sf::Vec2uz size)
    {
        SFML_BASE_ASSERT(static_cast<sf::base::SizeT>(c) < m_glyphRects.size());

        m_glyphRects[static_cast<sf::base::SizeT>(c)] = {position, size};
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2uz getGlyphRect(const char c) const
    {
        SFML_BASE_ASSERT(static_cast<sf::base::SizeT>(c) < m_glyphRects.size());

        const auto& [glyphPos, glyphSize] = m_glyphRects[static_cast<sf::base::SizeT>(c)];

        return {
            .position = glyphPos,
            .size     = glyphSize,
        };
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f getGlyphTextureRect(const sf::Rect2f& fontTextureRect, const char c) const
    {
        const auto glyphRect = getGlyphRect(c).toRect2f();

        return {
            .position = fontTextureRect.position + glyphRect.position,
            .size     = glyphRect.size,
        };
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2uz getGlyphSize([[maybe_unused]] const char c) const
    {
        SFML_BASE_ASSERT(static_cast<sf::base::SizeT>(c) < m_glyphRects.size());

        return m_glyphRects[static_cast<sf::base::SizeT>(c)].size;
    }


    //////////////////////////////////////////////////////////////
    void adjustSize(const char c, const sf::Vec2i offset)
    {
        SFML_BASE_ASSERT(static_cast<sf::base::SizeT>(c) < m_glyphRects.size());

        auto& glyphRect = m_glyphRects[static_cast<sf::base::SizeT>(c)];
        glyphRect.size  = (glyphRect.size.toVec2i() + offset).toVec2uz();
    }
};

} // namespace tsurv
