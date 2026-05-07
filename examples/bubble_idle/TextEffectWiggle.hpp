#pragma once

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Vector.hpp"


namespace sf
{
class Text;
}


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
    void apply(sf::Text& text);

    ////////////////////////////////////////////////////////////
    void unapply(sf::Text& text);

private:
    sf::base::Vector<sf::Vec2f> m_oldVertexPositions;
    float                       m_time = 0.f;
    float                       m_frequency;
    float                       m_amplitude;
    float                       m_phase;
};
