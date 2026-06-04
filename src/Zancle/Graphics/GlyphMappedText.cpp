// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/GlyphMappedText.hpp"

#include "Zancle/Graphics/FontFace.hpp"
#include "Zancle/Graphics/Glyph.hpp"
#include "Zancle/Graphics/GlyphMapping.hpp"
#include "Zancle/Graphics/TextBase.hpp"
#include "Zancle/Graphics/Texture.hpp"

#include "Zancle/System/LifetimeDependant.hpp"

#include "ZancleBase/Assert.hpp"


namespace za
{
////////////////////////////////////////////////////////////
GlyphMappedText::GlyphMappedText(const FontFace&     fontFace,
                                 const Texture&      texture,
                                 const GlyphMapping& glyphMapping,
                                 const Data&         data) :
    TextBase(data),
    m_fontFace(&fontFace),
    m_texture(&texture),
    m_glyphMapping(&glyphMapping)
{
    ZA_UPDATE_LIFETIME_DEPENDANT(FontFace, GlyphMappedText, this, m_fontFace);
    ZA_UPDATE_LIFETIME_DEPENDANT(Texture, GlyphMappedText, this, m_texture);
    ZA_UPDATE_LIFETIME_DEPENDANT(GlyphMapping, GlyphMappedText, this, m_glyphMapping);
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
void GlyphMappedText::setGlyphMapping(const FontFace& fontFace, const Texture& texture, const GlyphMapping& glyphMapping)
{
    if (m_fontFace == &fontFace && m_texture == &texture && m_glyphMapping == &glyphMapping)
        return;

    m_fontFace           = &fontFace;
    m_texture            = &texture;
    m_glyphMapping       = &glyphMapping;
    m_geometryNeedUpdate = true;

    ZA_UPDATE_LIFETIME_DEPENDANT(FontFace, GlyphMappedText, this, m_fontFace);
    ZA_UPDATE_LIFETIME_DEPENDANT(Texture, GlyphMappedText, this, m_texture);
    ZA_UPDATE_LIFETIME_DEPENDANT(GlyphMapping, GlyphMappedText, this, m_glyphMapping);
}


////////////////////////////////////////////////////////////
const GlyphMapping& GlyphMappedText::getGlyphMapping() const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return *m_glyphMapping;
}


////////////////////////////////////////////////////////////
bool GlyphMappedText::isBold() const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->bold;
}


////////////////////////////////////////////////////////////
const GlyphMappedText& GlyphMappedText::getFontSource() const
{
    return *this;
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getKerning(const char32_t first, const char32_t second, const unsigned int characterSize, const bool bold) const
{
    ZB_ASSERT(m_fontFace != nullptr);
    return m_fontFace->getKerning(first, second, characterSize, bold);
}


////////////////////////////////////////////////////////////
const Glyph& GlyphMappedText::getGlyph(const char32_t     codePoint,
                                       const unsigned int characterSize,
                                       const bool         bold,
                                       const float        outlineThickness) const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->getGlyph(codePoint, characterSize, bold, outlineThickness);
}


////////////////////////////////////////////////////////////
GlyphMapping::GlyphPair GlyphMappedText::getFillAndOutlineGlyph(
    const char32_t     codePoint,
    const unsigned int characterSize,
    const bool         bold,
    const float        outlineThickness) const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->getFillAndOutlineGlyph(codePoint, characterSize, bold, outlineThickness);
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getLineSpacing(const unsigned int characterSize) const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->getLineSpacing(characterSize);
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getAscent(const unsigned int characterSize) const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->getAscent(characterSize);
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getDescent(const unsigned int characterSize) const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->getDescent(characterSize);
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getUnderlinePosition(const unsigned int characterSize) const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->getUnderlinePosition(characterSize);
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getUnderlineThickness(const unsigned int characterSize) const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->getUnderlineThickness(characterSize);
}


////////////////////////////////////////////////////////////
const Texture& GlyphMappedText::getTexture() const
{
    ZB_ASSERT(m_texture != nullptr);
    return *m_texture;
}


////////////////////////////////////////////////////////////
unsigned int GlyphMappedText::getCharacterSize() const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->characterSize;
}


////////////////////////////////////////////////////////////
float GlyphMappedText::getOutlineThickness() const
{
    ZB_ASSERT(m_glyphMapping != nullptr);
    return m_glyphMapping->outlineThickness;
}

} // namespace za
