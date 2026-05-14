#include "TextShakeEffect.hpp"

#include "ExampleUtils/RNGFast.hpp"

#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"

#include "SFML/System/Angle.hpp"

#include "SFML/Base/ClampMacro.hpp"


////////////////////////////////////////////////////////////
void TextShakeEffect::bump(RNGFast& rng, const float strength)
{
    grow  = strength;
    angle = rng.getF(-grow * 0.2f, grow * 0.2f);
}

////////////////////////////////////////////////////////////
void TextShakeEffect::update(const float deltaTimeMs)
{
    if (grow > 0.f)
        grow -= deltaTimeMs * 0.0165f;

    if (angle != 0.f)
    {
        const float sign = angle > 0.f ? 1.f : -1.f;
        angle -= sign * deltaTimeMs * 0.00565f;

        if (sign * angle < 0.f)
            angle = 0.f;
    }

    grow  = SFML_BASE_CLAMP(grow, 0.f, 5.f);
    angle = SFML_BASE_CLAMP(angle, -0.5f, 0.5f);
}

////////////////////////////////////////////////////////////
void TextShakeEffect::applyToText(auto& text) const
{
    text.scale    = {1.f + grow * 0.2f, 1.f + grow * 0.2f};
    text.rotation = sf::radians(angle);
}


template void TextShakeEffect::applyToText(sf::Text& text) const;
template void TextShakeEffect::applyToText(sf::TextData& text) const;
