// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GlyphMappedText.hpp"

#include "SFML/Graphics/GlyphMapping.hpp"
#include "SFML/Graphics/TextBase.hpp"
#include "SFML/Graphics/TextBase.inl" // IWYU pragma: keep

#include "SFML/System/LifetimeDependant.hpp"

#include "SFML/Base/Assert.hpp"


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class sf::TextBase<sf::GlyphMappedText>;


namespace sf
{
////////////////////////////////////////////////////////////
GlyphMappedText::GlyphMappedText(const Texture& texture, const GlyphMapping& glyphMapping, const Data& data) :
    TextBase(data),
    m_texture(&texture),
    m_glyphMapping(&glyphMapping)
{
    SFML_UPDATE_LIFETIME_DEPENDANT(GlyphMapping, GlyphMappedText, this, m_glyphMapping);
}


////////////////////////////////////////////////////////////
GlyphMappedText::~GlyphMappedText() = default;


////////////////////////////////////////////////////////////
GlyphMappedText::GlyphMappedText(const GlyphMappedText&)            = default;
GlyphMappedText& GlyphMappedText::operator=(const GlyphMappedText&) = default;


////////////////////////////////////////////////////////////
GlyphMappedText::GlyphMappedText(GlyphMappedText&&) noexcept            = default;
GlyphMappedText& GlyphMappedText::operator=(GlyphMappedText&&) noexcept = default;


////////////////////////////////////////////////////////////
void GlyphMappedText::setGlyphMapping(const Texture& texture, const GlyphMapping& glyphMapping)
{
    if (m_texture == &texture && m_glyphMapping == &glyphMapping)
        return;

    m_texture            = &texture;
    m_glyphMapping       = &glyphMapping;
    m_geometryNeedUpdate = true;

    SFML_UPDATE_LIFETIME_DEPENDANT(GlyphMapping, GlyphMappedText, this, m_glyphMapping);
}


////////////////////////////////////////////////////////////
const GlyphMapping& GlyphMappedText::getGlyphMapping() const
{
    SFML_BASE_ASSERT(m_glyphMapping != nullptr);
    return *m_glyphMapping;
}


////////////////////////////////////////////////////////////
bool GlyphMappedText::isBold() const
{
    SFML_BASE_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->bold;
}


////////////////////////////////////////////////////////////
const GlyphMapping& GlyphMappedText::getFontSource() const
{
    return getGlyphMapping();
}


////////////////////////////////////////////////////////////
const Texture& GlyphMappedText::getTexture() const
{
    SFML_BASE_ASSERT(m_texture != nullptr);
    return *m_texture;
}


////////////////////////////////////////////////////////////
unsigned int GlyphMappedText::getCharacterSize() const
{
    SFML_BASE_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->characterSize;
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getOutlineThickness() const
{
    SFML_BASE_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->outlineThickness;
}

} // namespace sf
