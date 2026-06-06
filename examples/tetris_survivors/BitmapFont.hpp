#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Container/Array.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"

#include "Zancle/Diagnostic/Assert.hpp"


namespace tsurv
{
//////////////////////////////////////////////////////////////
class [[nodiscard]] BitmapFont
{
private:
    //////////////////////////////////////////////////////////////
    za::Array<za::Rect2uz, 256> m_glyphRects;

public:
    //////////////////////////////////////////////////////////////
    void addGlyph(const char c, const za::Vec2uz position, const za::Vec2uz size)
    {
        ZA_ASSERT(static_cast<za::SizeT>(c) < m_glyphRects.size());

        m_glyphRects[static_cast<za::SizeT>(c)] = {position, size};
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] za::Rect2uz getGlyphRect(const char c) const
    {
        ZA_ASSERT(static_cast<za::SizeT>(c) < m_glyphRects.size());

        const auto& [glyphPos, glyphSize] = m_glyphRects[static_cast<za::SizeT>(c)];

        return {
            .position = glyphPos,
            .size     = glyphSize,
        };
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] za::Rect2f getGlyphTextureRect(const za::Rect2f& fontTextureRect, const char c) const
    {
        const auto glyphRect = getGlyphRect(c).toRect2f();

        return {
            .position = fontTextureRect.position + glyphRect.position,
            .size     = glyphRect.size,
        };
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] za::Vec2uz getGlyphSize([[maybe_unused]] const char c) const
    {
        ZA_ASSERT(static_cast<za::SizeT>(c) < m_glyphRects.size());

        return m_glyphRects[static_cast<za::SizeT>(c)].size;
    }


    //////////////////////////////////////////////////////////////
    void adjustSize(const char c, const za::Vec2i offset)
    {
        ZA_ASSERT(static_cast<za::SizeT>(c) < m_glyphRects.size());

        auto& glyphRect = m_glyphRects[static_cast<za::SizeT>(c)];
        glyphRect.size  = (glyphRect.size.toVec2i() + offset).toVec2uz();
    }
};

} // namespace tsurv
