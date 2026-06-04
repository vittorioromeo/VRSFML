#pragma once

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Vector.hpp"


namespace za
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
    void apply(za::Text& text);

    ////////////////////////////////////////////////////////////
    void unapply(za::Text& text);

private:
    zb::Vector<za::Vec2f> m_oldVertexPositions;
    float                 m_time = 0.f;
    float                 m_frequency;
    float                 m_amplitude;
    float                 m_phase;
};
