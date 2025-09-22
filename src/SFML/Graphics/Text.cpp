#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/FontInfo.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/ShapedGlyph.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/String.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/Vector.hpp"

#include <SheenBidi/SheenBidi.h>
#include <freetype/freetype.h>
#include <hb-ft.h>
#include <hb.h>


namespace sf
{
////////////////////////////////////////////////////////////
Text::Text(const Font& font, const Data& data) :
    m_string(data.string),
    m_font(&font),
    m_characterSize(data.characterSize),
    m_letterSpacingFactor(data.letterSpacing),
    m_lineSpacingFactor(data.lineSpacing),
    m_outlineThickness(data.outlineThickness),
    m_fillColor(data.fillColor),
    m_outlineColor(data.outlineColor),
    position{data.position},
    scale{data.scale},
    origin{data.origin},
    rotation{data.rotation},
    m_style(data.style),
    m_lineAlignment(data.lineAlignment),
    m_textOrientation(data.textOrientation),
    m_clusterGrouping(data.clusterGrouping),
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
    if (m_letterSpacingFactor == spacingFactor)
        return;

    m_letterSpacingFactor = spacingFactor;
    m_geometryNeedUpdate  = true;
}


////////////////////////////////////////////////////////////
void Text::setLineSpacing(const float spacingFactor)
{
    if (m_lineSpacingFactor == spacingFactor)
        return;

    m_lineSpacingFactor  = spacingFactor;
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
void Text::setLineAlignment(const LineAlignment lineAlignment)
{
    if (m_lineAlignment == lineAlignment)
        return;

    m_lineAlignment      = lineAlignment;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setOrientation(const TextOrientation textOrientation)
{
    if (m_textOrientation == textOrientation)
        return;

    m_textOrientation    = textOrientation;
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
    return m_letterSpacingFactor;
}


////////////////////////////////////////////////////////////
float Text::getLineSpacing() const
{
    return m_lineSpacingFactor;
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
Text::LineAlignment Text::getLineAlignment() const
{
    return m_lineAlignment;
}


////////////////////////////////////////////////////////////
Text::Orientation Text::getOrientation() const
{
    return m_textOrientation;
}


////////////////////////////////////////////////////////////
const base::Vector<ShapedGlyph>& Text::getShapedGlyphs() const
{
    ensureGeometryUpdate(*m_font);
    return m_glyphs;
}


////////////////////////////////////////////////////////////
Text::ClusterGrouping Text::getClusterGrouping() const
{
    return m_clusterGrouping;
}


////////////////////////////////////////////////////////////
void Text::setClusterGrouping(const ClusterGrouping clusterGrouping)
{
    if (m_clusterGrouping == clusterGrouping)
        return;

    m_clusterGrouping    = clusterGrouping;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setGlyphPreProcessor(GlyphPreProcessor glyphPreProcessor)
{
    m_glyphPreProcessor  = SFML_BASE_MOVE(glyphPreProcessor);
    m_geometryNeedUpdate = true;
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
    m_glyphs.clear();
    m_fillVerticesStartIndex = 0u;
    m_bounds                 = {};

    // No text: nothing to draw
    if (m_string.isEmpty())
        return;

    // Precalculate the amount of quads that will be produced
    const auto fillQuadCount = precomputeQuadCount(); // Very approximate
    // We must check for glyph preprocessor as it might introduce outlining
    const auto outlineQuadCount = (m_outlineThickness == 0.f && !m_glyphPreProcessor) ? 0u : fillQuadCount;

    const base::SizeT outlineVertexCount = outlineQuadCount * 4u;
    const base::SizeT fillVertexCount    = fillQuadCount * 4u;

    m_vertices.resize(outlineVertexCount + fillVertexCount);
    m_fillVerticesStartIndex = outlineVertexCount;

    // TODO P0:
    base::SizeT currFillIndex    = outlineVertexCount;
    base::SizeT currOutlineIndex = 0u;

    // Compute values related to the text style
    const bool  isBold             = !!(m_style & Style::Bold);
    const bool  isUnderlined       = !!(m_style & Style::Underlined);
    const bool  isStrikeThrough    = !!(m_style & Style::StrikeThrough);
    const float underlineOffset    = font.getUnderlinePosition(m_characterSize);
    const float underlineThickness = font.getUnderlineThickness(m_characterSize);

    // Compute the location of the strikethrough dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strikethrough as well
    const float strikeThroughOffset = TextUtils::precomputeStrikethroughOffset(font, isBold, isStrikeThrough, m_characterSize);

    // Precompute the variables needed by the algorithm
    const auto [whitespaceWidth,
                letterSpacing,
                lineSpacing] = TextUtils::precomputeSpacingConstants(font, isBold, m_characterSize, m_letterSpacingFactor, m_lineSpacingFactor);

    float x = 0.f;
    float y = (m_textOrientation == TextOrientation::Default) ? static_cast<float>(m_characterSize) : 0.f;

    // Variables used to compute bounds
    auto minX = static_cast<float>(m_characterSize);
    auto minY = static_cast<float>(m_characterSize);
    auto maxX = 0.f;
    auto maxY = 0.f;

    // Check that we have a usable font
    SFML_BASE_ASSERT(font.getInfo().id && "Font not usable for shaping text");

    // Ensure the font is set to the size expected by the shaper
    if (!font.setCurrentSize(m_characterSize))
    {
        SFML_BASE_ASSERT(false && "Failed to set font size");
        return;
    }

    // Shaping only supports single lines, we will need to
    // break multi-line strings into individual lines ourselves
    static thread_local String                  currentLine;
    static thread_local base::Vector<base::U32> currentLineIndices;
    static thread_local base::Vector<base::U32> currentLineTabIndices;

    currentLine.clear();
    currentLineIndices.clear();
    currentLineTabIndices.clear();

    const auto addHLines = [&](const float lineLeft, const float lineRight, const float lineTop, const float offset)
    {
        TextUtils::addLineHorizontal(m_vertices.data(), currFillIndex, lineLeft, lineRight, lineTop, m_fillColor, offset, underlineThickness);

        if (m_outlineThickness != 0.f)
            TextUtils::addLineHorizontal(m_vertices.data(),
                                         currOutlineIndex,
                                         lineLeft,
                                         lineRight,
                                         lineTop,
                                         m_outlineColor,
                                         offset,
                                         underlineThickness,
                                         m_outlineThickness);
    };

    const auto addVLines = [&](const float lineMinX, const float lineMaxX, const float lineMinY, const float lineMaxY)
    {
        const float offset = !!(m_style & Style::Italic) ? (lineMaxX - lineMinX) * 0.1f : 0.f;

        TextUtils::addLineVertical(m_vertices.data(), currFillIndex, lineMinY, lineMaxY, m_fillColor, offset, underlineThickness);

        if (m_outlineThickness != 0.f)
            TextUtils::addLineVertical(m_vertices.data(),
                                       currOutlineIndex,
                                       lineMinY,
                                       lineMaxY,
                                       m_outlineColor,
                                       offset,
                                       underlineThickness,
                                       m_outlineThickness);
    };

    static thread_local base::Vector<TextUtils::GlyphData> shapedGlyphs;

    hb_script_t currentScript{};

    const auto outputLine = [&]
    {
        // Variables used to compute bounds for the current line
        auto lineMinX = static_cast<float>(m_characterSize);
        auto lineMinY = static_cast<float>(m_characterSize);
        auto lineMaxX = 0.f;
        auto lineMaxY = 0.f;

        // We want to combine the multiple spaces we substituted a tab with back into a single glyph
        // To do that we just enlarge the first of its shaped spaces to 4 times its width
        // We then skip the following 3 spaces
        auto glyphsToSkip = 0;

        shapedGlyphs.clear();
        populateShapedGlyphs(static_cast<hb_font_t*>(m_font->getHBSubFont(m_characterSize)),
                             m_characterSize,
                             currentLine,
                             currentLineIndices,
                             currentScript,
                             m_textOrientation,
                             m_clusterGrouping,
                             m_outlineThickness,
                             !!(m_style & Style::Bold),
                             shapedGlyphs);

        // Create one quad for each character
        for (const TextUtils::GlyphData& shapeGlyph : shapedGlyphs)
        {
            // Skip trailing spaces of a tab
            if (glyphsToSkip > 0)
            {
                --glyphsToSkip;
                continue;
            }

            // Extract the current glyph's description
            const Glyph& glyph = m_font->getGlyphByGlyphIndex(shapeGlyph.id, m_characterSize, isBold, /* outlineThickness */ 0.f);

            // Add the glyph to the glyph list
            ShapedGlyph& glyphEntry = m_glyphs.emplaceBack(glyph);
            glyphEntry.cluster      = shapeGlyph.cluster;
            glyphEntry.position     = Vec2f{x, y} + shapeGlyph.offset;

            bool isVertical = false;

            switch (shapeGlyph.direction)
            {
                case HB_DIRECTION_LTR:
                    glyphEntry.textDirection = TextDirection::LeftToRight;
                    glyphEntry.glyph.advance = shapeGlyph.advance.x;
                    glyphEntry.baseline      = y;
                    break;
                case HB_DIRECTION_RTL:
                    glyphEntry.textDirection = TextDirection::RightToLeft;
                    glyphEntry.glyph.advance = shapeGlyph.advance.x;
                    glyphEntry.baseline      = y;
                    break;
                case HB_DIRECTION_TTB:
                    glyphEntry.textDirection = TextDirection::TopToBottom;
                    glyphEntry.glyph.advance = shapeGlyph.advance.y;
                    glyphEntry.baseline      = x;
                    isVertical               = true;
                    break;
                case HB_DIRECTION_BTT:
                    glyphEntry.textDirection = TextDirection::BottomToTop;
                    glyphEntry.glyph.advance = shapeGlyph.advance.y;
                    glyphEntry.baseline      = x;
                    isVertical               = true;
                    break;
                default:
                    glyphEntry.textDirection = TextDirection::Unspecified;
                    break;
            }

            Style style            = m_style;
            Color fillColor        = m_fillColor;
            Color outlineColor     = m_outlineColor;
            float outlineThickness = m_outlineThickness;
            float italicShear      = 0.f;

            // Add the glyph to the vertices, if the texture rect has an area
            // (not the case with transparent glyphs e.g. space character)
            if (glyph.textureRect.size.x != 0.f && glyph.textureRect.size.y != 0.f)
            {
                if (m_glyphPreProcessor)
                    m_glyphPreProcessor(glyphEntry, style, fillColor, outlineColor, outlineThickness);

                italicShear = !!(style & Style::Italic) ? degrees(12).asRadians() : 0.f;

                glyphEntry.vertexOffset = currFillIndex;

                if (outlineThickness != 0.f)
                {
                    const auto& [fillGlyph,
                                 outlineGlyph] = font.getFillAndOutlineGlyphByGlyphIndex(shapeGlyph.id,
                                                                                         m_characterSize,
                                                                                         !!(style & Style::Bold),
                                                                                         outlineThickness);

                    TextUtils::addGlyphQuad(m_vertices.data(), currFillIndex, glyphEntry.position, fillColor, fillGlyph, italicShear);

                    TextUtils::addGlyphQuad(m_vertices.data(),
                                            currOutlineIndex,
                                            glyphEntry.position,
                                            outlineColor,
                                            outlineGlyph,
                                            italicShear);
                }
                else
                {

                    const Glyph& fillGlyph = font.getGlyphByGlyphIndex(shapeGlyph.id,
                                                                       m_characterSize,
                                                                       !!(style & Style::Bold),
                                                                       /* outlineThickness */ 0.f);

                    TextUtils::addGlyphQuad(m_vertices.data(), currFillIndex, glyphEntry.position, fillColor, fillGlyph, italicShear);
                }

                glyphEntry.vertexCount = 4u;

                if (isVertical)
                {
                    const auto hasAdvance = glyphEntry.glyph.advance > 0.f;
                    if (isUnderlined && hasAdvance)
                    {
                        const Vec2f entryPosition = glyphEntry.position + Vec2f(glyphEntry.glyph.bounds.position.x, 0.f);
                        const float entryWidth = glyphEntry.glyph.bounds.size.x;

                        addHLines(entryPosition.x, entryPosition.x + entryWidth, entryPosition.y, underlineOffset);
                    }
                }
            }
            else
            {
                // Remove unwanted x-offset when dealing with vertical spaces
                if ((shapeGlyph.direction == HB_DIRECTION_TTB) || (shapeGlyph.direction == HB_DIRECTION_BTT))
                    glyphEntry.position.x -= shapeGlyph.offset.x;

                // Check if we are dealing with the start of a tab
                if (base::find(currentLineTabIndices.begin(), currentLineTabIndices.end(), shapeGlyph.cluster) !=
                    currentLineTabIndices.end())
                {
                    // Widen the size to 4 times the advance of the space glyph
                    glyphEntry.glyph.advance *= 4.0f;

                    if (isVertical)
                        glyphEntry.glyph.bounds.size.y = glyphEntry.glyph.advance;
                    else
                        glyphEntry.glyph.bounds.size.x = glyphEntry.glyph.advance;

                    // Skip the next 3 space glyphs
                    glyphsToSkip = 3;
                }
                else
                {
                    // If the glyph doesn't have a texture it is probably a space
                    // Set the bounds width to the width of the space so the user can
                    // make use of it
                    if (isVertical)
                        glyphEntry.glyph.bounds.size = {0.f, shapeGlyph.advance.y + letterSpacing};
                    else
                        glyphEntry.glyph.bounds.size = {shapeGlyph.advance.x + letterSpacing, 0.f};
                }

                if (m_glyphPreProcessor)
                    m_glyphPreProcessor(glyphEntry, style, fillColor, outlineColor, outlineThickness);

                italicShear = !!(style & Style::Italic) ? degrees(12).asRadians() : 0.f;
            }

            // Update the current bounds
            const Vec2f p1 = glyph.bounds.position + shapeGlyph.offset;
            const Vec2f p2 = p1 + glyphEntry.glyph.bounds.size;

            lineMinX = SFML_BASE_MIN(lineMinX, x + p1.x - italicShear * p2.y);
            lineMaxX = SFML_BASE_MAX(lineMaxX, x + p2.x - italicShear * p1.y);
            lineMinY = SFML_BASE_MIN(lineMinY, y + p1.y);
            lineMaxY = SFML_BASE_MAX(lineMaxY, y + p2.y);

            // Advance to the next character
            // Only apply additional letter spacing if the current glyph has an advance (base glyph)
            // Applying the letter spacing to glyphs without an advance would affect
            // mark glyphs as well which would lead to incorrect results
            const auto hasAdvance = glyphEntry.glyph.advance > 0.f;

            if (isVertical)
                y += glyphEntry.glyph.advance + (hasAdvance ? letterSpacing : 0.f);
            else
                x += glyphEntry.glyph.advance + (hasAdvance ? letterSpacing : 0.f);
        }

        // Update the multi-line bounds
        minX = SFML_BASE_MIN(minX, lineMinX);
        maxX = SFML_BASE_MAX(maxX, lineMaxX);
        minY = SFML_BASE_MIN(minY, lineMinY);
        maxY = SFML_BASE_MAX(maxY, lineMaxY);

        currentLine.clear();
        currentLineIndices.clear();
        currentLineTabIndices.clear();

        // If we're using the underlined style and there's a new line, draw a line
        // (Vertical underlines are applied per-glyph above)
        if (isUnderlined && m_textOrientation == TextOrientation::Default)
            addHLines(0.f, x, y, underlineOffset);

        // If we're using the strikethrough style and there's a new line, draw a line across all characters
        if (isStrikeThrough)
        {
            if (m_textOrientation == TextOrientation::Default)
                addHLines(0.f, x, y, strikeThroughOffset);
            else
            {
                // Slanting the text by 12 degrees means the strikethrough would have to be
                // offset by `width * sin(12 degrees) / 2` which is approximately `width * 0.1`
                addVLines(lineMinX, lineMaxX, lineMinY, lineMaxY);
            }
        }
    };

    // In order to be able to align text we have to record all line data until we can compute the text metrics
    // We then use the record data to shift the necessary lines to the right/left as necessary
    struct LineRecord
    {
        base::SizeT    glyphsStart{};
        base::SizeT    glyphsCount{};
        base::SizeT    verticesStart{};
        base::SizeT    verticesCount{};
        base::SizeT    outlineVerticesStart{};
        base::SizeT    outlineVerticesCount{};
        base::SizeT    firstCodepointOffset = static_cast<base::SizeT>(-1);
        hb_direction_t direction{};
        float          lineWidth{};
    };

    static thread_local base::Vector<LineRecord> lines;
    lines.clear();

    float maxLineWidth = 0.f;

    const auto beginLineRecord = [&]
    {
        // Start a new line record
        auto& lineRecord                = lines.emplaceBack();
        lineRecord.glyphsStart          = m_glyphs.size();
        lineRecord.verticesStart        = currFillIndex;
        lineRecord.outlineVerticesStart = currOutlineIndex;
    };

    const auto endLineRecord = [&]
    {
        // Complete the line record
        auto& lineRecord                = lines.back();
        lineRecord.glyphsCount          = m_glyphs.size() - lineRecord.glyphsStart;
        lineRecord.verticesCount        = currFillIndex - lineRecord.verticesStart;
        lineRecord.outlineVerticesCount = currOutlineIndex - lineRecord.outlineVerticesStart;
        lineRecord.lineWidth            = x;

        maxLineWidth = SFML_BASE_MAX(maxLineWidth, x);
    };

    static thread_local base::Vector<TextUtils::StringSegment> segments;

    segments.clear();
    populateStringSegments(m_string, segments);

    hb_direction_t currentDirection{};

    if (!segments.empty())
        beginLineRecord();

    // Split the input string into multiple segments with uniform
    // script and direction using the unicode bidirectional algorithm

    // Iterate over all segments
    for (const auto& [offset, length, script, direction] : segments)
    {
        currentScript    = static_cast<hb_script_t>(script);
        currentDirection = static_cast<hb_direction_t>(direction);

        if (offset < lines.back().firstCodepointOffset)
        {
            lines.back().firstCodepointOffset = offset;
            lines.back().direction            = currentDirection;
        }

        // We use the index into the input string as the input cluster IDs as well
        // This way the user will be able to map the resulting cluster IDs back to
        // characters in the input text they provided for advanced functionality
        for (auto index = static_cast<base::U32>(offset); index < static_cast<base::U32>(offset + length); ++index)
        {
            const auto& curChar = m_string[index];

            // Handle special characters
            if ((curChar == U'\n'))
            {
                if (!currentLine.isEmpty())
                    outputLine();

                // Add new entry to glyphs
                // For our purposes, we consider the newline
                // character to constitute its own cluster
                auto& glyph = m_glyphs.emplaceBack(font.getGlyph('\n', m_characterSize, isBold, /* outlineThickness */ 0.f));

                glyph.glyph.bounds.size = {};
                glyph.baseline          = y;
                glyph.position          = {x, y};
                glyph.cluster           = index;

                endLineRecord();
                beginLineRecord();

                // Update the current bounds (min coordinates)
                minX = SFML_BASE_MIN(minX, x);
                minY = SFML_BASE_MIN(minY, y);

                y += lineSpacing;
                x = 0;

                // Update the current bounds (max coordinates)
                maxX = SFML_BASE_MAX(maxX, x);
                maxY = SFML_BASE_MAX(maxY, y);

                // Next glyph, no need to create a quad for newline
                continue;
            }

            if (curChar == U'\t')
            {
                // Replace tab character with 4 spaces for shaping
                currentLine += "    ";
                currentLineIndices.resize(currentLineIndices.size() + 4u, index);
                currentLineTabIndices.emplaceBack(index);
                continue;
            }

            if ((curChar < 0x80) && TextUtils::isControlChar(static_cast<int>(curChar)))
            {
                // Skip all other control characters to avoid weird graphical issues
                continue;
            }

            currentLine += curChar;
            currentLineIndices.emplaceBack(index);
        }

        if (!currentLine.isEmpty())
            outputLine();
    }

    if (!segments.empty())
        endLineRecord();

    // If we're using outline, update the current bounds
    if (m_outlineThickness != 0.f)
    {
        const float outline = base::fabs(base::ceil(m_outlineThickness));
        minX -= outline;
        maxX += outline;
        minY -= outline;
        maxY += outline;
    }

    // Update the bounding rectangle
    m_bounds.position = Vec2f(minX, minY);
    m_bounds.size     = Vec2f(maxX, maxY) - Vec2f(minX, minY);

    // Use line record data to post-process lines e.g. re-alignment etc.
    if (lines.empty())
        return;

    for (LineRecord& line : lines)
    {
        float shift = 0.f;

        if (m_lineAlignment == LineAlignment::Center)
            shift = line.lineWidth / -2.0f;
        else if (m_lineAlignment == LineAlignment::Right)
            shift = -line.lineWidth;
        else if ((m_lineAlignment == LineAlignment::Default) && (line.direction == HB_DIRECTION_RTL))
            shift = maxLineWidth - line.lineWidth;
        else
            continue; // Skip modifying the data if there is nothing to shift

        // Shift glyphs
        for (auto i = line.glyphsStart; i < line.glyphsStart + line.glyphsCount; ++i)
            m_glyphs[i].position.x += shift;

        // Shift vertices
        for (auto i = line.verticesStart; i < line.verticesStart + line.verticesCount; ++i)
            m_vertices[i].position.x += shift;

        // Shift vertices
        for (auto i = line.outlineVerticesStart; i < line.outlineVerticesStart + line.outlineVerticesCount; ++i)
            m_vertices[i].position.x += shift;
    }

    // Update bounds if necessary
    if (m_lineAlignment == LineAlignment::Center)
        m_bounds.position.x -= m_bounds.size.x / 2.0f;
    else if (m_lineAlignment == LineAlignment::Right)
        m_bounds.position.x -= m_bounds.size.x;
}


////////////////////////////////////////////////////////////
base::SizeT Text::precomputeQuadCount() const
{
    SFML_BASE_ASSERT(!m_string.isEmpty());

    base::SizeT result = 0u;

    // Compute values related to the text style
    const bool isUnderlined    = !!(m_style & Style::Underlined);
    const bool isStrikeThrough = !!(m_style & Style::StrikeThrough);

    for (auto index = 0u; index < m_string.getSize(); ++index)
    {
        const char32_t curChar = m_string[index];

        // Handle special characters
        if ((curChar == U'\n'))
        {
            if (isStrikeThrough)
                ++result;

            // Next glyph, no need to create a quad for newline
            continue;
        }

        // Replace tab character with 4 spaces for shaping
        if (curChar == U'\t')
            continue;

        // Skip all other control characters to avoid weird graphical issues
        if ((curChar < 0x80) && TextUtils::isControlChar(static_cast<int>(curChar)))
            continue;

        result += 2u;

        if (isUnderlined)
            ++result;
    }

    return result;
}

} // namespace sf
