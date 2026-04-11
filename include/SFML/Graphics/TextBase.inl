#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawQuadsSettings.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/TextBase.hpp"
#include "SFML/Graphics/TextUtils.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename TDerived>
template <typename TData>
TextBase<TDerived>::TextBase(const TData& data) :
    m_string(data.string),
    m_letterSpacing(data.letterSpacing),
    m_lineSpacing(data.lineSpacing),
    m_fillColor(data.fillColor),
    m_outlineColor(data.outlineColor),
    position{data.position},
    scale{data.scale},
    origin{data.origin},
    rotation{data.rotation},
    m_italic(data.italic),
    m_underlined(data.underlined),
    m_strikeThrough(data.strikeThrough),
    m_geometryNeedUpdate{true}
{
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setString(const UnicodeString& string)
{
    if (m_string == string)
        return;

    m_string             = string;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setLetterSpacing(const float spacingFactor)
{
    if (m_letterSpacing == spacingFactor)
        return;

    m_letterSpacing      = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setLineSpacing(const float spacingFactor)
{
    if (m_lineSpacing == spacingFactor)
        return;

    m_lineSpacing        = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setItalic(const bool italic)
{
    if (m_italic == italic)
        return;

    m_italic             = italic;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setUnderlined(const bool underlined)
{
    if (m_underlined == underlined)
        return;

    m_underlined         = underlined;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setStrikeThrough(const bool strikeThrough)
{
    if (m_strikeThrough == strikeThrough)
        return;

    m_strikeThrough      = strikeThrough;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setFillColor(const Color color)
{
    if (color == m_fillColor)
        return;

    m_fillColor = color;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color = m_fillColor;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setOutlineColor(const Color color)
{
    if (color == m_outlineColor)
        return;

    m_outlineColor = color;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color = m_outlineColor;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setFillColorAlpha(const base::U8 alpha)
{
    if (m_fillColor.a == alpha)
        return;

    m_fillColor.a = alpha;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color.a = m_fillColor.a;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::setOutlineColorAlpha(const base::U8 alpha)
{
    if (m_outlineColor.a == alpha)
        return;

    m_outlineColor.a = alpha;

    if (!m_geometryNeedUpdate)
        for (base::SizeT i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color.a = m_outlineColor.a;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
const UnicodeString& TextBase<TDerived>::getString() const
{
    return m_string;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
float TextBase<TDerived>::getLetterSpacing() const
{
    return m_letterSpacing;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
float TextBase<TDerived>::getLineSpacing() const
{
    return m_lineSpacing;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
bool TextBase<TDerived>::isItalic() const
{
    return m_italic;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
bool TextBase<TDerived>::isUnderlined() const
{
    return m_underlined;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
bool TextBase<TDerived>::isStrikeThrough() const
{
    return m_strikeThrough;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
Color TextBase<TDerived>::getFillColor() const
{
    return m_fillColor;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
Color TextBase<TDerived>::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
base::U8 TextBase<TDerived>::getFillColorAlpha() const
{
    return m_fillColor.a;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
base::U8 TextBase<TDerived>::getOutlineColorAlpha() const
{
    return m_outlineColor.a;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
Vec2f TextBase<TDerived>::findCharacterPos(base::SizeT index) const
{
    const auto& derived    = static_cast<const TDerived&>(*this);
    const auto& fontSource = derived.getFontSource();
    const auto  charSize   = derived.getCharacterSize();
    const bool  isBold     = derived.isBold();

    index = base::min(index, m_string.getSize());

    const auto [whitespaceWidth,
                letterSpacing,
                lineSpacing] = TextUtils::precomputeSpacingConstants(fontSource, isBold, charSize, m_letterSpacing, m_lineSpacing);

    Vec2f    characterPos;
    char32_t prevChar = 0;
    for (base::SizeT i = 0u; i < index; ++i)
    {
        const char32_t curChar = m_string[i];

        characterPos.x += fontSource.getKerning(prevChar, curChar, charSize, isBold);
        prevChar = curChar;

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

        characterPos.x += fontSource.getGlyph(curChar, charSize, isBold, 0.f).advance + letterSpacing;
    }

    return this->getTransform().transformPoint(characterPos);
}


////////////////////////////////////////////////////////////
template <typename TDerived>
const Rect2f& TextBase<TDerived>::getLocalBounds() const
{
    ensureGeometryUpdate();
    return m_bounds;
}


////////////////////////////////////////////////////////////
template <typename TDerived>
Rect2f TextBase<TDerived>::getGlobalBounds() const
{
    return this->getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::draw(RenderTarget& target, RenderStates states) const
{
    const auto& derived = static_cast<const TDerived&>(*this);

    states.transform *= this->getTransform();
    states.texture = &derived.getTexture();

    ensureGeometryUpdate();

    target.drawQuads(
        sf::DrawQuadsSettings{
            .vertexSpan    = m_vertices,
            .primitiveType = PrimitiveType::Triangles,
        },
        states);
}


////////////////////////////////////////////////////////////
template <typename TDerived>
void TextBase<TDerived>::ensureGeometryUpdate() const
{
    if (!m_geometryNeedUpdate)
        return;

    m_geometryNeedUpdate = false;

    m_vertices.clear();
    m_fillVerticesStartIndex = 0u;
    m_bounds                 = {};

    if (m_string.isEmpty())
        return;

    const auto& derived          = static_cast<const TDerived&>(*this);
    const auto& fontSource       = derived.getFontSource();
    const auto  charSize         = derived.getCharacterSize();
    const float outlineThickness = derived.getOutlineThickness();

    const auto fillQuadCount    = TextUtils::precomputeTextQuadCount(m_string, m_underlined, m_strikeThrough);
    const auto outlineQuadCount = outlineThickness == 0.f ? 0u : fillQuadCount;

    const base::SizeT outlineVertexCount = outlineQuadCount * 4u;
    const base::SizeT fillVertexCount    = fillQuadCount * 4u;

    m_vertices.resize(outlineVertexCount + fillVertexCount);
    m_fillVerticesStartIndex = outlineVertexCount;

    m_bounds = TextUtils::createTextGeometryAndGetBounds<
        true>(outlineVertexCount,
              fontSource,
              m_string,
              derived.isBold(),
              m_italic,
              m_underlined,
              m_strikeThrough,
              charSize,
              m_letterSpacing,
              m_lineSpacing,
              outlineThickness,
              m_fillColor,
              m_outlineColor,
              [this](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return TextUtils::addLine(m_vertices.data(), SFML_BASE_FORWARD(xs)...); },
              [this](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return TextUtils::addGlyphQuad(m_vertices.data(), SFML_BASE_FORWARD(xs)...); });
}

} // namespace sf
