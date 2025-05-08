#pragma once

#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
class TextEffectWiggle
{
public:
    ////////////////////////////////////////////////////////////
    explicit TextEffectWiggle(const float frequency, const float amplitude, const float phase = 0.f) :
    m_frequency{frequency},
    m_amplitude{amplitude},
    m_phase{phase}
    {
    }

    ////////////////////////////////////////////////////////////
    void advance(float deltaTime)
    {
        m_time += deltaTime;
    }

    ////////////////////////////////////////////////////////////
    void apply(sf::Text& text)
    {
        auto textVertices           = text.getVerticesMut();
        const auto [tvData, tvSize] = textVertices;

        m_oldVertexPositions.clear();
        m_oldVertexPositions.reserve(tvSize);

        for (const sf::Vertex& v : textVertices)
            m_oldVertexPositions.pushBack(v.position);

        auto       nOutlineVertices = text.getFillVerticesStartIndex();
        const auto t                = m_time * m_frequency;

        auto func = [this](float xTime, sf::base::SizeT xIndex)
        { return static_cast<float>(sf::base::sin(xTime + float(xIndex) + m_phase) * m_amplitude); };

        for (sf::base::SizeT i = 0u; i < nOutlineVertices / 4u; ++i)
            for (sf::base::SizeT j = 0u; j < 4u; ++j)
                tvData[i * 4u + j].position.y += func(t, i);

        for (sf::base::SizeT i = nOutlineVertices / 4u; i < tvSize / 4u; ++i)
            for (sf::base::SizeT j = 0u; j < 4u; ++j)
                tvData[i * 4u + j].position.y += func(t, i - nOutlineVertices / 4u);
    }

    ////////////////////////////////////////////////////////////
    void unapply(sf::Text& text)
    {
        const auto [tvData, tvSize] = text.getVerticesMut();

        for (auto i = 0u; i < tvSize; ++i)
            tvData[i].position = m_oldVertexPositions[i];
    }

private:
    sf::base::Vector<sf::Vec2f> m_oldVertexPositions;
    float                       m_time = 0.f;
    float                       m_frequency;
    float                       m_amplitude;
    float                       m_phase;
};
