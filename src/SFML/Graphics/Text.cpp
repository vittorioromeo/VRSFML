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
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/String.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/Memcpy.hpp"
#include "SFML/Base/TrivialVector.hpp"

#include <cstdint>


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

    const sf::Vertex vertexData[] = {{{-outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}},
                                     {{lineLength + outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}},
                                     {{-outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}},
                                     {{-outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}},
                                     {{lineLength + outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}},
                                     {{lineLength + outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}}};

    SFML_BASE_MEMCPY(vertices + index, vertexData, sizeof(sf::Vertex) * 6);
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

    const auto uv1 = glyph.textureRect.position.to<sf::Vector2f>() - padding;
    const auto uv2 = (glyph.textureRect.position + glyph.textureRect.size).to<sf::Vector2f>() + padding;

    const sf::Vertex vertexData[] = {{position + sf::Vector2f(p1.x - italicShear * p1.y, p1.y), color, {uv1.x, uv1.y}},
                                     {position + sf::Vector2f(p2.x - italicShear * p1.y, p1.y), color, {uv2.x, uv1.y}},
                                     {position + sf::Vector2f(p1.x - italicShear * p2.y, p2.y), color, {uv1.x, uv2.y}},
                                     {position + sf::Vector2f(p1.x - italicShear * p2.y, p2.y), color, {uv1.x, uv2.y}},
                                     {position + sf::Vector2f(p2.x - italicShear * p1.y, p1.y), color, {uv2.x, uv1.y}},
                                     {position + sf::Vector2f(p2.x - italicShear * p2.y, p2.y), color, {uv2.x, uv2.y}}};

    SFML_BASE_MEMCPY(vertices + index, vertexData, sizeof(sf::Vertex) * 6);
    index += 6;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Text::Impl
{
    const Font*                         font{};                     //!< Font used to display the string
    String                              string;                     //!< String to display
    unsigned int                        characterSize{30};          //!< Base size of characters, in pixels
    float                               letterSpacingFactor{1.f};   //!< Spacing factor between letters
    float                               lineSpacingFactor{1.f};     //!< Spacing factor between lines
    Style                               style{Style::Regular};      //!< Text style (see Style enum)
    Color                               fillColor{Color::White};    //!< Text fill color
    Color                               outlineColor{Color::Black}; //!< Text outline color
    float                               outlineThickness{0.f};      //!< Thickness of the text's outline
    mutable base::TrivialVector<Vertex> vertices;   //!< Vertex array containing the outline and fill geometry
    mutable base::SizeT   fillVerticesStartIndex{}; //!< Index in the vertex array where the fill vertices start
    mutable FloatRect     bounds;                   //!< Bounding rectangle of the text (in local coordinates)
    mutable bool          geometryNeedUpdate{};     //!< Does the geometry need to be recomputed?
    mutable std::uint64_t fontTextureId{};          //!< The font texture id

    explicit Impl(const Font& theFont, String theString, unsigned int theCharacterSize) :
    font(&theFont),
    string(SFML_BASE_MOVE(theString)),
    characterSize(theCharacterSize)
    {
    }
};


////////////////////////////////////////////////////////////
Text::Text(const Font& font, String string, unsigned int characterSize) :
m_impl(font, SFML_BASE_MOVE(string), characterSize)
{
    SFML_UPDATE_LIFETIME_DEPENDANT(Font, Text, this, m_impl->font);
}


////////////////////////////////////////////////////////////
Text::Text(const Font& font, const char* string, unsigned int characterSize) : Text(font, String(string), characterSize)
{
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
    if (m_impl->string == string)
        return;

    m_impl->string             = string;
    m_impl->geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setString(const char* string)
{
    setString(String(string));
}


////////////////////////////////////////////////////////////
void Text::setFont(const Font& font)
{
    if (m_impl->font == &font)
        return;

    m_impl->font               = &font;
    m_impl->geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setCharacterSize(unsigned int size)
{
    if (m_impl->characterSize == size)
        return;

    m_impl->characterSize      = size;
    m_impl->geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setLetterSpacing(float spacingFactor)
{
    if (m_impl->letterSpacingFactor == spacingFactor)
        return;

    m_impl->letterSpacingFactor = spacingFactor;
    m_impl->geometryNeedUpdate  = true;
}


////////////////////////////////////////////////////////////
void Text::setLineSpacing(float spacingFactor)
{
    if (m_impl->lineSpacingFactor == spacingFactor)
        return;

    m_impl->lineSpacingFactor  = spacingFactor;
    m_impl->geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setStyle(Text::Style style)
{
    if (m_impl->style == style)
        return;

    m_impl->style              = style;
    m_impl->geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setFillColor(Color color)
{
    if (color == m_impl->fillColor)
        return;

    m_impl->fillColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_impl->geometryNeedUpdate)
    {
        for (base::SizeT i = m_impl->fillVerticesStartIndex; i < m_impl->vertices.size(); ++i)
            m_impl->vertices[i].color = m_impl->fillColor;
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineColor(Color color)
{
    if (color == m_impl->outlineColor)
        return;

    m_impl->outlineColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_impl->geometryNeedUpdate)
    {
        for (base::SizeT i = 0; i < m_impl->fillVerticesStartIndex; ++i)
            m_impl->vertices[i].color = m_impl->outlineColor;
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineThickness(float thickness)
{
    if (thickness == m_impl->outlineThickness)
        return;

    m_impl->outlineThickness   = thickness;
    m_impl->geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
const String& Text::getString() const
{
    return m_impl->string;
}


////////////////////////////////////////////////////////////
const Font& Text::getFont() const
{
    return *m_impl->font;
}


////////////////////////////////////////////////////////////
unsigned int Text::getCharacterSize() const
{
    return m_impl->characterSize;
}


////////////////////////////////////////////////////////////
float Text::getLetterSpacing() const
{
    return m_impl->letterSpacingFactor;
}


////////////////////////////////////////////////////////////
float Text::getLineSpacing() const
{
    return m_impl->lineSpacingFactor;
}


////////////////////////////////////////////////////////////
Text::Style Text::getStyle() const
{
    return m_impl->style;
}


////////////////////////////////////////////////////////////
Color Text::getFillColor() const
{
    return m_impl->fillColor;
}


////////////////////////////////////////////////////////////
Color Text::getOutlineColor() const
{
    return m_impl->outlineColor;
}


////////////////////////////////////////////////////////////
float Text::getOutlineThickness() const
{
    return m_impl->outlineThickness;
}


////////////////////////////////////////////////////////////
Vector2f Text::findCharacterPos(base::SizeT index) const
{
    // Adjust the index if it's out of range
    index = base::min(index, m_impl->string.getSize());

    // Precompute the variables needed by the algorithm
    const bool  isBold          = !!(m_impl->style & Style::Bold);
    float       whitespaceWidth = m_impl->font->getGlyph(U' ', m_impl->characterSize, isBold).advance;
    const float letterSpacing   = (whitespaceWidth / 3.f) * (m_impl->letterSpacingFactor - 1.f);
    whitespaceWidth += letterSpacing;
    const float lineSpacing = m_impl->font->getLineSpacing(m_impl->characterSize) * m_impl->lineSpacingFactor;

    // Compute the position
    Vector2f      position;
    std::uint32_t prevChar = 0;
    for (base::SizeT i = 0; i < index; ++i)
    {
        const std::uint32_t curChar = m_impl->string[i];

        // Apply the kerning offset
        position.x += m_impl->font->getKerning(prevChar, curChar, m_impl->characterSize, isBold);
        prevChar = curChar;

        // Handle special characters
        switch (curChar)
        {
            case U' ':
                position.x += whitespaceWidth;
                continue;
            case U'\t':
                position.x += whitespaceWidth * 4;
                continue;
            case U'\n':
                position.y += lineSpacing;
                position.x = 0;
                continue;
        }

        // For regular characters, add the advance offset of the glyph
        position.x += m_impl->font->getGlyph(curChar, m_impl->characterSize, isBold).advance + letterSpacing;
    }

    // Transform the position to global coordinates
    position = getTransform().transformPoint(position);

    return position;
}


////////////////////////////////////////////////////////////
const FloatRect& Text::getLocalBounds() const
{
    ensureGeometryUpdate(*m_impl->font);

    return m_impl->bounds;
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
    states.texture        = &m_impl->font->getTexture(m_impl->characterSize);
    states.coordinateType = CoordinateType::Pixels;

    const auto [data, size] = getVertices();
    target.draw(data, size, PrimitiveType::Triangles, states);
}


////////////////////////////////////////////////////////////
[[nodiscard]] base::Span<const Vertex> Text::getVertices() const
{
    ensureGeometryUpdate(*m_impl->font);

    return {m_impl->vertices.data(), m_impl->vertices.size()};
}


////////////////////////////////////////////////////////////
void Text::ensureGeometryUpdate(const Font& font) const
{
    const auto fontTextureCacheId = font.getTexture(m_impl->characterSize).m_cacheId;

    // Do nothing, if geometry has not changed and the font texture has not changed
    if (!m_impl->geometryNeedUpdate && fontTextureCacheId == m_impl->fontTextureId)
        return;

    // Save the current fonts texture id
    m_impl->fontTextureId = fontTextureCacheId;

    // Mark geometry as updated
    m_impl->geometryNeedUpdate = false;

    // Clear the previous geometry
    m_impl->vertices.clear();
    m_impl->fillVerticesStartIndex = 0u;
    m_impl->bounds                 = {};

    // No text: nothing to draw
    if (m_impl->string.isEmpty())
        return;

    // Compute values related to the text style
    const bool  isBold             = !!(m_impl->style & Style::Bold);
    const bool  isUnderlined       = !!(m_impl->style & Style::Underlined);
    const bool  isStrikeThrough    = !!(m_impl->style & Style::StrikeThrough);
    const float italicShear        = !!(m_impl->style & Style::Italic) ? degrees(12).asRadians() : 0.f;
    const float underlineOffset    = font.getUnderlinePosition(m_impl->characterSize);
    const float underlineThickness = font.getUnderlineThickness(m_impl->characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    const float strikeThroughOffset = font.getGlyph(U'x', m_impl->characterSize, isBold).bounds.getCenter().y;

    // Precompute the variables needed by the algorithm
    float       whitespaceWidth = font.getGlyph(U' ', m_impl->characterSize, isBold).advance;
    const float letterSpacing   = (whitespaceWidth / 3.f) * (m_impl->letterSpacingFactor - 1.f);
    whitespaceWidth += letterSpacing;
    const float lineSpacing = font.getLineSpacing(m_impl->characterSize) * m_impl->lineSpacingFactor;

    // TODO P1: docs and cleanup
    base::SizeT fillQuadCount    = 0;
    base::SizeT outlineQuadCount = 0;

    {
        float x = 0.f;

        const base::SizeT outlineQuadIncrement = (m_impl->outlineThickness == 0) ? 0 : 1;

        const auto addLinesFake = [&]
        {
            outlineQuadCount += outlineQuadIncrement;
            ++fillQuadCount;
        };

        const auto addGlyphsFake = addLinesFake;

        std::uint32_t prevChar = 0;

        for (const std::uint32_t curChar : m_impl->string)
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
            if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t'))
            {
                switch (curChar)
                {
                    case U' ':
                        x += 1.f;
                        break;
                    case U'\t':
                        x += 1.f;
                        break;
                    case U'\n':
                        x = 0;
                        break;
                }

                // Next glyph, no need to create a quad for whitespace
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

    const base::SizeT outlineVertexCount = outlineQuadCount * 6;
    const base::SizeT fillVertexCount    = fillQuadCount * 6;

    m_impl->vertices.resize(outlineVertexCount + fillVertexCount);
    m_impl->fillVerticesStartIndex = outlineVertexCount;

    base::SizeT currFillIndex    = outlineVertexCount;
    base::SizeT currOutlineIndex = 0;

    float x = 0.f;
    auto  y = static_cast<float>(m_impl->characterSize);

    // Create one quad for each character
    auto  minX = static_cast<float>(m_impl->characterSize);
    auto  minY = static_cast<float>(m_impl->characterSize);
    float maxX = 0.f;
    float maxY = 0.f;

    std::uint32_t prevChar = 0;

    const auto addLines = [this, &currFillIndex, &currOutlineIndex, &x, &y, &underlineThickness](float offset)
    {
        addLine(m_impl->vertices.data(), currFillIndex, x, y, m_impl->fillColor, offset, underlineThickness);

        if (m_impl->outlineThickness != 0)
            addLine(m_impl->vertices.data(),
                    currOutlineIndex,
                    x,
                    y,
                    m_impl->outlineColor,
                    offset,
                    underlineThickness,
                    m_impl->outlineThickness);
    };

    for (const std::uint32_t curChar : m_impl->string)
    {
        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            continue;

        // Apply the kerning offset
        x += font.getKerning(prevChar, curChar, m_impl->characterSize, isBold);

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
            minX = base::min(minX, x);
            minY = base::min(minY, y);

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
            maxX = base::max(maxX, x);
            maxY = base::max(maxY, y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Apply the outline
        if (m_impl->outlineThickness != 0)
        {
            const Glyph& glyph = font.getGlyph(curChar, m_impl->characterSize, isBold, m_impl->outlineThickness);

            // Add the outline glyph to the vertices
            addGlyphQuad(m_impl->vertices.data(), currOutlineIndex, Vector2f{x, y}, m_impl->outlineColor, glyph, italicShear);
        }

        // Extract the current glyph's description
        const Glyph& glyph = font.getGlyph(curChar, m_impl->characterSize, isBold);

        // Add the glyph to the vertices
        addGlyphQuad(m_impl->vertices.data(), currFillIndex, Vector2f{x, y}, m_impl->fillColor, glyph, italicShear);

        // Update the current bounds
        const Vector2f p1 = glyph.bounds.position;
        const Vector2f p2 = glyph.bounds.position + glyph.bounds.size;

        minX = base::min(minX, x + p1.x - italicShear * p2.y);
        maxX = base::max(maxX, x + p2.x - italicShear * p1.y);
        minY = base::min(minY, y + p1.y);
        maxY = base::max(maxY, y + p2.y);

        // Advance to the next character
        x += glyph.advance + letterSpacing;
    }

    // If we're using outline, update the current bounds
    if (m_impl->outlineThickness != 0)
    {
        const float outline = base::fabs(base::ceil(m_impl->outlineThickness));
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
    m_impl->bounds.position = Vector2f{minX, minY};
    m_impl->bounds.size     = Vector2f{maxX, maxY} - Vector2f{minX, minY};
}

} // namespace sf
