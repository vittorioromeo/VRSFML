#include "TextEffectWiggle.hpp"

#include "Zancle/Graphics/Text.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/Math/Sin.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
void TextEffectWiggle::apply(za::Text& text)
{
    auto textVertices           = text.getVerticesMut();
    const auto [tvData, tvSize] = textVertices;

    m_oldVertexPositions.clear();
    m_oldVertexPositions.reserve(tvSize);

    for (const za::Vertex& v : textVertices)
        m_oldVertexPositions.pushBack(v.position);

    auto       nOutlineVertices = text.getFillVerticesStartIndex();
    const auto t                = m_time * m_frequency;

    auto func = [this](float xTime, za::SizeT xIndex)
    { return static_cast<float>(za::sin(xTime + float(xIndex) + m_phase) * m_amplitude); };

    for (za::SizeT i = 0u; i < nOutlineVertices / 4u; ++i)
        for (za::SizeT j = 0u; j < 4u; ++j)
            tvData[i * 4u + j].position.y += func(t, i);

    for (za::SizeT i = nOutlineVertices / 4u; i < tvSize / 4u; ++i)
        for (za::SizeT j = 0u; j < 4u; ++j)
            tvData[i * 4u + j].position.y += func(t, i - nOutlineVertices / 4u);
}


////////////////////////////////////////////////////////////
void TextEffectWiggle::unapply(za::Text& text)
{
    const auto [tvData, tvSize] = text.getVerticesMut();

    for (auto i = 0u; i < tvSize; ++i)
        tvData[i].position = m_oldVertexPositions[i];
}
