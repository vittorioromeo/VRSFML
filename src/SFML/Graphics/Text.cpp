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
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/String.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/Math/Fmax.hpp"
#include "SFML/Base/Math/Fmin.hpp"
#include "SFML/Base/TrivialVector.hpp"


namespace
{
// Add an underline or strikethrough line to the vertex array
void addLine(sf::Vertex*      vertices,
             sf::base::SizeT& index,
             float            lineLength,
             float            lineTop,
             sf::Color        color,
             float            offset,
             float            thickness,
             float            outlineThickness = 0)
{
    const float top    = sf::base::floor(lineTop + offset - (thickness / 2) + 0.5f);
    const float bottom = top + sf::base::floor(thickness + 0.5f);

    auto* ptr = vertices + index;

    *ptr++ = {{-outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}};
    *ptr++ = {{lineLength + outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}};
    *ptr++ = {{-outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}};
    *ptr++ = {{-outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}};
    *ptr++ = {{lineLength + outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}};
    *ptr++ = {{lineLength + outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}};

    index += 6;
}

// Add a glyph quad to the vertex array
void addGlyphQuad(sf::Vertex*      vertices,
                  sf::base::SizeT& index,
                  sf::Vector2f     position,
                  sf::Color        color,
                  const sf::Glyph& glyph,
                  float            italicShear)
{
    const sf::Vector2f padding(1.f, 1.f);

    const sf::Vector2f p1 = glyph.bounds.position - padding;
    const sf::Vector2f p2 = glyph.bounds.position + glyph.bounds.size + padding;

    const auto uv1 = glyph.textureRect.position - padding;
    const auto uv2 = (glyph.textureRect.position + glyph.textureRect.size) + padding;

    auto* ptr = vertices + index;

    *ptr++ = {position + sf::Vector2f(p1.x - italicShear * p1.y, p1.y), color, {uv1.x, uv1.y}};
    *ptr++ = {position + sf::Vector2f(p2.x - italicShear * p1.y, p1.y), color, {uv2.x, uv1.y}};
    *ptr++ = {position + sf::Vector2f(p1.x - italicShear * p2.y, p2.y), color, {uv1.x, uv2.y}};
    *ptr++ = {position + sf::Vector2f(p1.x - italicShear * p2.y, p2.y), color, {uv1.x, uv2.y}};
    *ptr++ = {position + sf::Vector2f(p2.x - italicShear * p1.y, p1.y), color, {uv2.x, uv1.y}};
    *ptr++ = {position + sf::Vector2f(p2.x - italicShear * p2.y, p2.y), color, {uv2.x, uv2.y}};

    index += 6;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
Text::Text(const Font& font, const Settings& settings) :
Transformable{.position = settings.position, .scale = settings.scale, .origin = settings.origin, .rotation = settings.rotation},
m_font(&font),
m_string(settings.string),
m_characterSize(settings.characterSize),
m_letterSpacing(settings.letterSpacing),
m_lineSpacing(settings.lineSpacing),
m_style(settings.style),
m_fillColor(settings.fillColor),
m_outlineColor(settings.outlineColor),
m_outlineThickness(settings.outlineThickness),
m_geometryNeedUpdate{true}
{
    SFML_UPDATE_LIFETIME_DEPENDANT(Font, Text, this, m_font);
}


////////////////////////////////////////////////////////////
Text::~Text() = default;


////////////////////////////////////////////////////////////
Text::Text(const Text&) = default;


////////////////////////////////////////////////////////////
Text& Text::operator=(const Text&) = default;


////////////////////////////////////////////////////////////
Text::Text(Text&&) noexcept = default;


////////////////////////////////////////////////////////////
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
}


////////////////////////////////////////////////////////////
void Text::setCharacterSize(unsigned int size)
{
    if (m_characterSize == size)
        return;

    m_characterSize      = size;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setLetterSpacing(float spacingFactor)
{
    if (m_letterSpacing == spacingFactor)
        return;

    m_letterSpacing      = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setLineSpacing(float spacingFactor)
{
    if (m_lineSpacing == spacingFactor)
        return;

    m_lineSpacing        = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setStyle(Text::Style style)
{
    if (m_style == style)
        return;

    m_style              = style;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setFillColor(Color color)
{
    if (color == m_fillColor)
        return;

    m_fillColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
    {
        for (base::SizeT i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color = m_fillColor;
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineColor(Color color)
{
    if (color == m_outlineColor)
        return;

    m_outlineColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
    {
        for (base::SizeT i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color = m_outlineColor;
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineThickness(float thickness)
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
float Text::getOutlineThickness() const
{
    return m_outlineThickness;
}


////////////////////////////////////////////////////////////
Vector2f Text::findCharacterPos(base::SizeT index) const
{
    // Adjust the index if it's out of range
    index = base::min(index, m_string.getSize());

    // Precompute the variables needed by the algorithm
    const bool  isBold          = !!(m_style & Style::Bold);
    float       whitespaceWidth = m_font->getGlyph(U' ', m_characterSize, isBold).advance;
    const float letterSpacing   = (whitespaceWidth / 3.f) * (m_letterSpacing - 1.f);
    whitespaceWidth += letterSpacing;
    const float lineSpacing = m_font->getLineSpacing(m_characterSize) * m_lineSpacing;

    // Compute the position
    Vector2f  characterPos;
    base::U32 prevChar = 0;
    for (base::SizeT i = 0; i < index; ++i)
    {
        const base::U32 curChar = m_string[i];

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
        characterPos.x += m_font->getGlyph(curChar, m_characterSize, isBold).advance + letterSpacing;
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
    states.texture        = &m_font->getTexture();
    states.coordinateType = CoordinateType::Pixels;

    const auto [data, size] = getVertices();
    target.draw(data, size, PrimitiveType::Triangles, states);
}


////////////////////////////////////////////////////////////
[[nodiscard]] base::Span<const Vertex> Text::getVertices() const
{
    ensureGeometryUpdate(*m_font);

    return {m_vertices.data(), m_vertices.size()};
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

    // Compute values related to the text style
    const bool  isBold             = !!(m_style & Style::Bold);
    const bool  isUnderlined       = !!(m_style & Style::Underlined);
    const bool  isStrikeThrough    = !!(m_style & Style::StrikeThrough);
    const float italicShear        = !!(m_style & Style::Italic) ? degrees(12).asRadians() : 0.f;
    const float underlineOffset    = font.getUnderlinePosition(m_characterSize);
    const float underlineThickness = font.getUnderlineThickness(m_characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    const float strikeThroughOffset = font.getGlyph(U'x', m_characterSize, isBold).bounds.getCenter().y;

    // Precompute the variables needed by the algorithm
    float       whitespaceWidth = font.getGlyph(U' ', m_characterSize, isBold).advance;
    const float letterSpacing   = (whitespaceWidth / 3.f) * (m_letterSpacing - 1.f);
    whitespaceWidth += letterSpacing;
    const float lineSpacing = font.getLineSpacing(m_characterSize) * m_lineSpacing;

    // Precalculate the amount of quads that will be produced
    base::SizeT fillQuadCount    = 0;
    base::SizeT outlineQuadCount = 0;

    {
        const auto addLinesFake = [&, outlineQuadIncrement = (m_outlineThickness == 0.f) ? 0u : 1u]
        {
            outlineQuadCount += outlineQuadIncrement;
            ++fillQuadCount;
        };

        const auto addGlyphsFake = addLinesFake;

        base::U32 prevChar = 0;
        float     x        = 0.f;

        for (const base::U32 curChar : m_string)
        {
            // Skip the \r char to avoid weird graphical issues
            if (curChar == U'\r')
                continue;

            // Apply the kerning offset
            x += 1.f;

            // If we're using the underlined or strike through style and there's a new line, draw a line
            if ((curChar == U'\n' && prevChar != U'\n') && (isUnderlined || isStrikeThrough))
                addLinesFake();

            prevChar = curChar;

            // Handle special characters
            switch (curChar)
            {
                case U' ':
                    [[fallthrough]];
                case U'\t':
                    x += 1.f;
                    continue;
                case U'\n':
                    x = 0;
                    continue;
            }

            // Apply the outline
            addGlyphsFake();

            // Advance to the next character
            x += letterSpacing;
        }

        // If we're using the underlined style, add the last line
        if (isUnderlined && (x > 0))
            addLinesFake();

        // If we're using the strike through style, add the last line across all characters
        if (isStrikeThrough && (x > 0))
            addLinesFake();
    }

    const base::SizeT outlineVertexCount = outlineQuadCount * 6u;
    const base::SizeT fillVertexCount    = fillQuadCount * 6u;

    m_vertices.resize(outlineVertexCount + fillVertexCount);
    m_fillVerticesStartIndex = outlineVertexCount;

    base::SizeT currFillIndex    = outlineVertexCount;
    base::SizeT currOutlineIndex = 0u;

    float x = 0.f;
    auto  y = static_cast<float>(m_characterSize);

    // Create one quad for each character
    auto  minX = static_cast<float>(m_characterSize);
    auto  minY = static_cast<float>(m_characterSize);
    float maxX = 0.f;
    float maxY = 0.f;

    base::U32 prevChar = 0;

    const auto addLines = [this, &currFillIndex, &currOutlineIndex, &x, &y, &underlineThickness](float offset)
    {
        addLine(m_vertices.data(), currFillIndex, x, y, m_fillColor, offset, underlineThickness);

        if (m_outlineThickness != 0.f)
            addLine(m_vertices.data(), currOutlineIndex, x, y, m_outlineColor, offset, underlineThickness, m_outlineThickness);
    };

    for (const base::U32 curChar : m_string)
    {
        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            continue;

        // Apply the kerning offset
        x += font.getKerning(prevChar, curChar, m_characterSize, isBold);

        if (curChar == U'\n' && prevChar != U'\n')
        {
            // If we're using the underlined style and there's a new line, draw a line
            if (isUnderlined)
                addLines(underlineOffset);

            // If we're using the strike through style and there's a new line, draw a line across all characters
            if (isStrikeThrough)
                addLines(strikeThroughOffset);
        }

        prevChar = curChar;

        // Handle special characters
        if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t'))
        {
            // Update the current bounds (min coordinates)
            minX = base::fmin(minX, x);
            minY = base::fmin(minY, y);

            switch (curChar)
            {
                case U' ':
                    x += whitespaceWidth;
                    break;
                case U'\t':
                    x += whitespaceWidth * 4;
                    break;
                case U'\n':
                    y += lineSpacing;
                    x = 0;
                    break;
            }

            // Update the current bounds (max coordinates)
            maxX = base::fmax(maxX, x);
            maxY = base::fmax(maxY, y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Apply the outline
        if (m_outlineThickness != 0.f)
        {
            const Glyph& glyph = font.getGlyph(curChar, m_characterSize, isBold, m_outlineThickness);

            // Add the outline glyph to the vertices
            addGlyphQuad(m_vertices.data(), currOutlineIndex, Vector2f{x, y}, m_outlineColor, glyph, italicShear);
        }

        // Extract the current glyph's description
        const Glyph& glyph = font.getGlyph(curChar, m_characterSize, isBold);

        // Add the glyph to the vertices
        addGlyphQuad(m_vertices.data(), currFillIndex, Vector2f{x, y}, m_fillColor, glyph, italicShear);

        // Update the current bounds
        const Vector2f p1 = glyph.bounds.position;
        const Vector2f p2 = glyph.bounds.position + glyph.bounds.size;

        minX = base::fmin(minX, x + p1.x - italicShear * p2.y);
        maxX = base::fmax(maxX, x + p2.x - italicShear * p1.y);
        minY = base::fmin(minY, y + p1.y);
        maxY = base::fmax(maxY, y + p2.y);

        // Advance to the next character
        x += glyph.advance + letterSpacing;
    }

    // If we're using outline, update the current bounds
    if (m_outlineThickness != 0.f)
    {
        const float outline = base::fabs(base::ceil(m_outlineThickness));
        minX -= outline;
        maxX += outline;
        minY -= outline;
        maxY += outline;
    }

    // If we're using the underlined style, add the last line
    if (isUnderlined && (x > 0))
        addLines(underlineOffset);

    // If we're using the strike through style, add the last line across all characters
    if (isStrikeThrough && (x > 0))
        addLines(strikeThroughOffset);

    // Update the bounding rectangle
    m_bounds.position = Vector2f{minX, minY};
    m_bounds.size     = Vector2f{maxX, maxY} - Vector2f{minX, minY};
}

} // namespace sf
