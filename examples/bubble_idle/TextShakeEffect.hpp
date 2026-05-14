#pragma once


class RNGFast;


////////////////////////////////////////////////////////////
struct [[nodiscard]] TextShakeEffect
{
    float grow  = 0.f;
    float angle = 0.f;

    void bump(RNGFast& rng, float strength);
    void update(float deltaTimeMs);
    void applyToText(auto& text) const;
};
