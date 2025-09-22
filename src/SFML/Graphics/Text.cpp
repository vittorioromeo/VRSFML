#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/String.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Text::Text(const Font& font, const Data& data) :
    m_string(data.string),
    m_font(&font),
    m_characterSize(data.characterSize),
    m_letterSpacing(data.letterSpacing),
    m_lineSpacing(data.lineSpacing),
    m_outlineThickness(data.outlineThickness),
    m_fillColor(data.fillColor),
    m_outlineColor(data.outlineColor),
    position{data.position},
    scale{data.scale},
    origin{data.origin},
    rotation{data.rotation},
    m_style(data.style),
    m_geometryNeedUpdate{true}
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
void Text::setString(const String& string)
{
    if (m_string == string)
        return;

    m_string             = string;
    m_geometryNeedUpdate = true;
}


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
void Text::setCharacterSize(const unsigned int size)
{
    if (m_characterSize == size)
        return;

    m_characterSize      = size;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setLetterSpacing(const float spacingFactor)
{
    if (m_letterSpacing == spacingFactor)
        return;

    m_letterSpacing      = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setLineSpacing(const float spacingFactor)
{
    if (m_lineSpacing == spacingFactor)
        return;

    m_lineSpacing        = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setStyle(const Text::Style style)
{
    if (m_style == style)
        return;

    m_style              = style;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setFillColor(const Color color)
{
    if (color == m_fillColor)
        return;

    m_fillColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
        for (base::SizeT i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color = m_fillColor;
}


////////////////////////////////////////////////////////////
void Text::setOutlineColor(const Color color)
{
    if (color == m_outlineColor)
        return;

    m_outlineColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
        for (base::SizeT i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color = m_outlineColor;
}


////////////////////////////////////////////////////////////
void Text::setFillColorAlpha(const base::U8 alpha)
{
    if (m_fillColor.a == alpha)
        return;

    m_fillColor.a = alpha;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
        for (base::SizeT i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color.a = m_fillColor.a;
}


////////////////////////////////////////////////////////////
void Text::setOutlineColorAlpha(const base::U8 alpha)
{
    if (m_outlineColor.a == alpha)
        return;

    m_outlineColor.a = alpha;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
        for (base::SizeT i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color.a = m_outlineColor.a;
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
const String& Text::getString() const
{
    return m_string;
}


////////////////////////////////////////////////////////////
const Font& Text::getFont() const
{
    return *m_font;
}


////////////////////////////////////////////////////////////
unsigned int Text::getCharacterSize() const
{
    return m_characterSize;
}


////////////////////////////////////////////////////////////
float Text::getLetterSpacing() const
{
    return m_letterSpacing;
}


////////////////////////////////////////////////////////////
float Text::getLineSpacing() const
{
    return m_lineSpacing;
}


////////////////////////////////////////////////////////////
Text::Style Text::getStyle() const
{
    return m_style;
}


////////////////////////////////////////////////////////////
Color Text::getFillColor() const
{
    return m_fillColor;
}


////////////////////////////////////////////////////////////
Color Text::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
base::U8 Text::getFillColorAlpha() const
{
    return m_fillColor.a;
}


////////////////////////////////////////////////////////////
base::U8 Text::getOutlineColorAlpha() const
{
    return m_outlineColor.a;
}


////////////////////////////////////////////////////////////
float Text::getOutlineThickness() const
{
    return m_outlineThickness;
}


////////////////////////////////////////////////////////////
Vec2f Text::findCharacterPos(base::SizeT index) const
{
    // Adjust the index if it's out of range
    index = base::min(index, m_string.getSize());

    // Precompute the variables needed by the algorithm
    const bool isBold = !!(m_style & Style::Bold);

    const auto [whitespaceWidth,
                letterSpacing,
                lineSpacing] = TextUtils::precomputeSpacingConstants(*m_font, m_style, m_characterSize, m_letterSpacing, m_lineSpacing);

    // Compute the position
    Vec2f    characterPos;
    char32_t prevChar = 0;
    for (base::SizeT i = 0u; i < index; ++i)
    {
        const char32_t curChar = m_string[i];

        // Apply the kerning offset
        characterPos.x += m_font->getKerning(prevChar, curChar, m_characterSize, isBold);
        prevChar = curChar;

        // Handle special characters
        switch (curChar)
        {
            case U' ':
                characterPos.x += whitespaceWidth;
                continue;
            case U'\t':
                characterPos.x += whitespaceWidth * 4;
                continue;
            case U'\n':
                characterPos.y += lineSpacing;
                characterPos.x = 0;
                continue;
        }

        // For regular characters, add the advance offset of the glyph
        characterPos.x += m_font->getGlyph(curChar, m_characterSize, isBold, /* outlineThickness */ 0.f).advance +
                          letterSpacing;
    }

    // Transform the characterPos to global coordinates
    return getTransform().transformPoint(characterPos);
}


////////////////////////////////////////////////////////////
const FloatRect& Text::getLocalBounds() const
{
    ensureGeometryUpdate(*m_font);

    return m_bounds;
}


////////////////////////////////////////////////////////////
FloatRect Text::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
void Text::draw(RenderTarget& target, RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_font->getTexture();

    ensureGeometryUpdate(*m_font);

    target.drawQuads({
        .vertexData    = m_vertices.data(),
        .vertexCount   = m_vertices.size(),
        .primitiveType = PrimitiveType::Triangles,
        .renderStates  = states,
    });
}


////////////////////////////////////////////////////////////
void Text::ensureGeometryUpdate(const Font& font) const
{
    // Do nothing, if geometry has not changed and the font texture has not changed
    if (!m_geometryNeedUpdate)
        return;

    // Mark geometry as updated
    m_geometryNeedUpdate = false;

    // Clear the previous geometry
    m_vertices.clear();
    m_fillVerticesStartIndex = 0u;
    m_bounds                 = {};

    // No text: nothing to draw
    if (m_string.isEmpty())
        return;

    // Precalculate the amount of quads that will be produced
    const auto fillQuadCount    = TextUtils::precomputeTextQuadCount(m_string, m_style);
    const auto outlineQuadCount = m_outlineThickness == 0.f ? 0u : fillQuadCount;

    const base::SizeT outlineVertexCount = outlineQuadCount * 4u;
    const base::SizeT fillVertexCount    = fillQuadCount * 4u;

    m_vertices.resize(outlineVertexCount + fillVertexCount);
    m_fillVerticesStartIndex = outlineVertexCount;

    m_bounds = TextUtils::createTextGeometryAndGetBounds<
        /* CalculateBounds */ true>(outlineVertexCount,
                                    font,
                                    m_string,
                                    m_style,
                                    m_characterSize,
                                    m_letterSpacing,
                                    m_lineSpacing,
                                    m_outlineThickness,
                                    m_fillColor,
                                    m_outlineColor,
                                    [this](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return TextUtils::addLine(m_vertices.data(), SFML_BASE_FORWARD(xs)...); },
                                    [this](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return TextUtils::addGlyphQuad(m_vertices.data(), SFML_BASE_FORWARD(xs)...); });
}

} // namespace sf
