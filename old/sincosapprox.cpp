
// https://publik-void.github.io/sin-cos-approximations/

#include <cmath>

namespace
{

[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float approxsin(float x1)
{
    float x2 = x1 * x1;
    return x1 * (0.999999060898976336474926982596043563 +
                 x2 * (-0.166655540927576933646197607200949732 +
                       x2 * (0.00831189980138987918776159520367912155 - 0.000184881402886071911033139680005197992 * x2)));
}

[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float approxcos(float x1)
{
    float x2 = x1 * x1;
    return 0.999970210689953068626323587055728078 +
           x2 * (-0.499782706704688809140466617726333455 +
                 x2 * (0.0413661149638482252569383872576459943 - 0.0012412397582398600702129604944720102 * x2));
}

[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float mysin(float x)
{
    float sign = 1.0f - 2.0f * (x > sf::base::pi);
    x -= sf::base::pi * (x > sf::base::pi);

    float mirror = sf::base::pi - x;
    x            = (x > ::sf::base::halfPi) * mirror + (x <= ::sf::base::halfPi) * x;

    return sign * approxsin(x);
}

[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float mycos(float x)
{
    float sign = 1.0f - 2.0f * (x >= sf::base::pi);
    x -= sf::base::pi * (x >= sf::base::pi);

    float mirror     = sf::base::pi - x;
    float use_mirror = x > ::sf::base::halfPi;
    x                = use_mirror * mirror + (1.0f - use_mirror) * x;
    sign *= 1.0f - 2.0f * use_mirror;

    return sign * approxcos(x);
}

} // namespace
