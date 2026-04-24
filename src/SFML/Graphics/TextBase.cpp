// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextBase.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GlyphMappedText.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextBase.inl" // IWYU pragma: keep


namespace sf
{
////////////////////////////////////////////////////////////
void TextBase::setString(const UnicodeString& string)
{
    if (m_string == string)
        return;

    m_string             = string;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void TextBase::setLetterSpacing(const float spacingFactor)
{
    if (m_letterSpacing == spacingFactor)
        return;

    m_letterSpacing      = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void TextBase::setLineSpacing(const float spacingFactor)
{
    if (m_lineSpacing == spacingFactor)
        return;

    m_lineSpacing        = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void TextBase::setItalic(const bool italic)
{
    if (m_italic == italic)
        return;

    m_italic             = italic;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void TextBase::setUnderlined(const bool underlined)
{
    if (m_underlined == underlined)
        return;

    m_underlined         = underlined;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void TextBase::setStrikeThrough(const bool strikeThrough)
{
    if (m_strikeThrough == strikeThrough)
        return;

    m_strikeThrough      = strikeThrough;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void TextBase::setFillColor(const Color color)
{
    if (color == m_fillColor)
        return;

    m_fillColor = color;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color = m_fillColor;
}


////////////////////////////////////////////////////////////
void TextBase::setOutlineColor(const Color color)
{
    if (color == m_outlineColor)
        return;

    m_outlineColor = color;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color = m_outlineColor;
}


////////////////////////////////////////////////////////////
void TextBase::setFillColorAlpha(const base::U8 alpha)
{
    if (m_fillColor.a == alpha)
        return;

    m_fillColor.a = alpha;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color.a = m_fillColor.a;
}


////////////////////////////////////////////////////////////
void TextBase::setOutlineColorAlpha(const base::U8 alpha)
{
    if (m_outlineColor.a == alpha)
        return;

    m_outlineColor.a = alpha;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color.a = m_outlineColor.a;
}


////////////////////////////////////////////////////////////
const UnicodeString& TextBase::getString() const
{
    return m_string;
}


////////////////////////////////////////////////////////////
float TextBase::getLetterSpacing() const
{
    return m_letterSpacing;
}


////////////////////////////////////////////////////////////
float TextBase::getLineSpacing() const
{
    return m_lineSpacing;
}


////////////////////////////////////////////////////////////
bool TextBase::isItalic() const
{
    return m_italic;
}


////////////////////////////////////////////////////////////
bool TextBase::isUnderlined() const
{
    return m_underlined;
}


////////////////////////////////////////////////////////////
bool TextBase::isStrikeThrough() const
{
    return m_strikeThrough;
}


////////////////////////////////////////////////////////////
Color TextBase::getFillColor() const
{
    return m_fillColor;
}


////////////////////////////////////////////////////////////
Color TextBase::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
base::U8 TextBase::getFillColorAlpha() const
{
    return m_fillColor.a;
}


////////////////////////////////////////////////////////////
base::U8 TextBase::getOutlineColorAlpha() const
{
    return m_outlineColor.a;
}

} // namespace sf


////////////////////////////////////////////////////////////
// Explicit template instantiations of the deducing-this methods
// for the two concrete derived types. Emits the symbols once in
// `sfml-graphics` so that downstream TUs only need the declarations
// in `TextBase.hpp` (no heavy rendering includes).
////////////////////////////////////////////////////////////
#define SFML_PRIV_INSTANTIATE_TEXTBASE_DEDUCING_THIS(TDerived)                                                       \
    template sf::Vec2f           sf::TextBase::findCharacterPos<TDerived>(const TDerived&, sf::base::SizeT);         \
    template const sf::Rect2f&   sf::TextBase::getLocalBounds<TDerived>(const TDerived&);                            \
    template sf::Rect2f          sf::TextBase::getGlobalBounds<TDerived>(const TDerived&);                           \
    template void                sf::TextBase::draw<TDerived>(const TDerived&, sf::RenderTarget&, sf::RenderStates); \
    template sf::ConstVertexSpan sf::TextBase::getVertices<TDerived>(const TDerived&);                               \
    template sf::VertexSpan      sf::TextBase::getVerticesMut<TDerived>(TDerived&);                                  \
    template void                sf::TextBase::ensureGeometryUpdate<TDerived>(const TDerived&)

SFML_PRIV_INSTANTIATE_TEXTBASE_DEDUCING_THIS(sf::Text);
SFML_PRIV_INSTANTIATE_TEXTBASE_DEDUCING_THIS(sf::GlyphMappedText);

#undef SFML_PRIV_INSTANTIATE_TEXTBASE_DEDUCING_THIS
