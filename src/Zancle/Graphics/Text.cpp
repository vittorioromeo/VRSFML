// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Text.hpp"

#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/TextBase.hpp"
#include "Zancle/Graphics/Texture.hpp"

#include "Zancle/System/LifetimeDependant.hpp"

#include "ZancleBase/Assert.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Text::Text(const Font& font, const Data& data) :
    TextBase(data),
    m_font(&font),
    m_characterSize(data.characterSize),
    m_outlineThickness(data.outlineThickness),
    m_bold(data.bold)
{
    ZA_UPDATE_LIFETIME_DEPENDANT(Font, Text, this, m_font);
}


////////////////////////////////////////////////////////////
Text::~Text()                          = default;
Text::Text(const Text&)                = default;
Text& Text::operator=(const Text&)     = default;
Text::Text(Text&&) noexcept            = default;
Text& Text::operator=(Text&&) noexcept = default;


////////////////////////////////////////////////////////////
void Text::setFont(const Font& font)
{
    if (m_font == &font)
        return;

    m_font               = &font;
    m_geometryNeedUpdate = true;

    ZA_UPDATE_LIFETIME_DEPENDANT(Font, Text, this, m_font);
}


////////////////////////////////////////////////////////////
void Text::setBold(const bool bold)
{
    if (m_bold == bold)
        return;

    m_bold               = bold;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setCharacterSize(const unsigned int size)
{
    if (m_characterSize == size)
        return;

    m_characterSize      = size;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setOutlineThickness(const float thickness)
{
    if (thickness == m_outlineThickness)
        return;

    m_outlineThickness   = thickness;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
const Font& Text::getFont() const
{
    ZB_ASSERT(m_font != nullptr);
    return *m_font;
}


////////////////////////////////////////////////////////////
bool Text::isBold() const
{
    return m_bold;
}


////////////////////////////////////////////////////////////
const Font& Text::getFontSource() const
{
    return getFont();
}


////////////////////////////////////////////////////////////
const Texture& Text::getTexture() const
{
    return m_font->getTexture();
}


////////////////////////////////////////////////////////////
unsigned int Text::getCharacterSize() const
{
    return m_characterSize;
}


////////////////////////////////////////////////////////////
float Text::getOutlineThickness() const
{
    return m_outlineThickness;
}

} // namespace za
