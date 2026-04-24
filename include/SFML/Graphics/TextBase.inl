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
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename Self>
Vec2f TextBase::findCharacterPos(this const Self& self, base::SizeT index)
{
    const auto& fontSource = self.getFontSource();
    const auto  charSize   = self.getCharacterSize();
    const bool  isBold     = self.isBold();

    index = base::min(index, self.m_string.getSize());

    const auto [whitespaceWidth,
                letterSpacing,
                lineSpacing] = TextUtils::precomputeSpacingConstants(fontSource, isBold, charSize, self.m_letterSpacing, self.m_lineSpacing);

    Vec2f    characterPos;
    char32_t prevChar = 0;
    for (base::SizeT i = 0u; i < index; ++i)
    {
        const char32_t curChar = self.m_string[i];

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

    return self.getTransform().transformPoint(characterPos);
}


////////////////////////////////////////////////////////////
template <typename Self>
const Rect2f& TextBase::getLocalBounds(this const Self& self)
{
    self.ensureGeometryUpdate();
    return self.m_bounds;
}


////////////////////////////////////////////////////////////
template <typename Self>
Rect2f TextBase::getGlobalBounds(this const Self& self)
{
    return self.getTransform().transformRect(self.getLocalBounds());
}


////////////////////////////////////////////////////////////
template <typename Self>
void TextBase::draw(this const Self& self, RenderTarget& target, RenderStates states)
{
    states.transform *= self.getTransform();
    states.texture = &self.getTexture();

    self.ensureGeometryUpdate();

    target.drawQuads(
        sf::DrawQuadsSettings{
            .vertexSpan    = self.m_vertices,
            .primitiveType = PrimitiveType::Triangles,
        },
        states);
}


////////////////////////////////////////////////////////////
template <typename Self>
ConstVertexSpan TextBase::getVertices(this const Self& self)
{
    self.ensureGeometryUpdate();
    return {self.m_vertices.data(), self.m_vertices.size()};
}


////////////////////////////////////////////////////////////
template <typename Self>
VertexSpan TextBase::getVerticesMut(this Self& self)
{
    self.ensureGeometryUpdate();
    return {self.m_vertices.data(), self.m_vertices.size()};
}


////////////////////////////////////////////////////////////
template <typename Self>
void TextBase::ensureGeometryUpdate(this const Self& self)
{
    if (!self.m_geometryNeedUpdate)
        return;

    self.m_geometryNeedUpdate = false;

    self.m_vertices.clear();
    self.m_fillVerticesStartIndex = 0u;
    self.m_bounds                 = {};

    if (self.m_string.isEmpty())
        return;

    const auto& fontSource       = self.getFontSource();
    const auto  charSize         = self.getCharacterSize();
    const float outlineThickness = self.getOutlineThickness();

    const auto fillQuadCount = TextUtils::precomputeTextQuadCount(self.m_string, self.m_underlined, self.m_strikeThrough);
    const auto outlineQuadCount = outlineThickness == 0.f ? 0u : fillQuadCount;

    const base::SizeT outlineVertexCount = outlineQuadCount * 4u;
    const base::SizeT fillVertexCount    = fillQuadCount * 4u;

    self.m_vertices.resize(outlineVertexCount + fillVertexCount);
    self.m_fillVerticesStartIndex = outlineVertexCount;

    self.m_bounds = TextUtils::createTextGeometryAndGetBounds<
        true>(outlineVertexCount,
              fontSource,
              self.m_string,
              self.isBold(),
              self.m_italic,
              self.m_underlined,
              self.m_strikeThrough,
              charSize,
              self.m_letterSpacing,
              self.m_lineSpacing,
              outlineThickness,
              self.m_fillColor,
              self.m_outlineColor,
              [&self] [[gnu::always_inline, gnu::flatten]] (auto&&... xs)
    { return TextUtils::addLine(self.m_vertices.data(), SFML_BASE_FORWARD(xs)...); },
              [&self] [[gnu::always_inline, gnu::flatten]] (auto&&... xs)
    { return TextUtils::addGlyphQuad(self.m_vertices.data(), SFML_BASE_FORWARD(xs)...); });
}

} // namespace sf
