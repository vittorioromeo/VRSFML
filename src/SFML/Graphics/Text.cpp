// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Text.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/TextBase.hpp"
#include "SFML/Graphics/TextBase.inl" // IWYU pragma: keep
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/LifetimeDependant.hpp"

#include "SFML/Base/Assert.hpp"


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class sf::TextBase<sf::Text>;


namespace sf
{
////////////////////////////////////////////////////////////
Text::Text(const Font& font, const Data& data) :
    TextBase(data),
    m_font(&font),
    m_characterSize(data.characterSize),
    m_outlineThickness(data.outlineThickness),
    m_bold(data.bold)
{
    SFML_UPDATE_LIFETIME_DEPENDANT(Font, Text, this, m_font);
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

    SFML_UPDATE_LIFETIME_DEPENDANT(Font, Text, this, m_font);
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
    SFML_BASE_ASSERT(m_font != nullptr);
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

} // namespace sf
