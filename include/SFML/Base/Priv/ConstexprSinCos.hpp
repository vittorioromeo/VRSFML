#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Constants.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const, gnu::flatten]] inline constexpr float normalizeAngle(float x)
{
    while (x > tau)
        x -= tau;

    while (x < -tau)
        x += tau;

    if (x > pi)
        x -= tau; // Map `(pi, 2pi]` to `(-pi, 0]`

    if (x <= -pi)
        x += tau; // Map `(-2pi, -pi]` to `(0, pi]`

    return x;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const, gnu::flatten]] inline constexpr float sinTaylor(const float x)
{
    if (x == 0.f)
        return 0.f;

    const float xSquared = x * x;

    float term   = x;
    float result = x;

    constexpr int iterations = 8;

    for (int i = 1; i <= iterations; ++i)
    {
        const int d = 2 * i * (2 * i + 1);

        term *= -xSquared / static_cast<float>(d);
        result += term;
    }

    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const, gnu::flatten]] inline constexpr float cosTaylor(const float x)
{
    if (x == 0)
        return 1.f;

    const float xSquared = x * x;

    float term   = 1.f;
    float result = 1.f;

    constexpr int iterations = 8;

    for (int i = 1; i <= iterations; ++i)
    {
        const int d = (2 * i - 1) * (2 * i);

        term *= -xSquared / static_cast<float>(d);
        result += term;
    }

    return result;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const, gnu::flatten]] constexpr float constexprSin(const float x)
{
    if (x != x)
        return x;

    if (x == halfPi * 0.f)
        return 0.f; // sin(0)

    if (x == halfPi * 0.25f)
        return 0.38268343f; // sin(pi/8)

    if (x == halfPi * 0.50f)
        return 0.70710678f; // sin(pi/4)

    if (x == halfPi * 0.75f)
        return 0.92387953f; // sin(3pi/8)

    if (x == halfPi * 1.f)
        return 1.f; // sin(pi/2)

    if (x == halfPi * 1.25f)
        return 0.92387953f; // sin(5pi/8)

    if (x == halfPi * 1.50f)
        return 0.70710678f; // sin(3pi/4)

    if (x == halfPi * 1.75f)
        return 0.38268343f; // sin(7pi/8)

    if (x == halfPi * 2.f)
        return 0.f; // sin(pi)

    if (x == halfPi * 2.25f)
        return -0.38268343f; // sin(9pi/8)

    if (x == halfPi * 2.50f)
        return -0.70710678f; // sin(5pi/4)

    if (x == halfPi * 2.75f)
        return -0.92387953f; // sin(11pi/8)

    if (x == halfPi * 3.f)
        return -1.f; // sin(3pi/2)

    if (x == halfPi * 3.25f)
        return -0.92387953f; // sin(13pi/8)

    if (x == halfPi * 3.50f)
        return -0.70710678f; // sin(7pi/4)

    if (x == halfPi * 3.75f)
        return -0.38268343f; // sin(15pi/8)

    if (x == halfPi * 4.f)
        return 0.f; // sin(2pi)

    const float xNormalized = normalizeAngle(x);

    if (xNormalized > halfPi)
        return sinTaylor(pi - xNormalized);

    if (xNormalized < -halfPi)
        return -sinTaylor(xNormalized + pi);

    return sinTaylor(xNormalized);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const, gnu::flatten]] constexpr float constexprCos(const float x)
{
    if (x != x)
        return x;

    if (x == halfPi * 0.f)
        return 1.f; // cos(0)

    if (x == halfPi * 0.25f)
        return 0.92387953f; // cos(pi/8)

    if (x == halfPi * 0.50f)
        return 0.70710678f; // cos(pi/4)

    if (x == halfPi * 0.75f)
        return 0.38268343f; // cos(3pi/8)

    if (x == halfPi * 1.f)
        return 0.f; // cos(pi/2)

    if (x == halfPi * 1.25f)
        return -0.38268343f; // cos(5pi/8)

    if (x == halfPi * 1.50f)
        return -0.70710678f; // cos(3pi/4)

    if (x == halfPi * 1.75f)
        return -0.92387953f; // cos(7pi/8)

    if (x == halfPi * 2.f)
        return -1.f; // cos(pi)

    if (x == halfPi * 2.25f)
        return -0.92387953f; // cos(9pi/8)

    if (x == halfPi * 2.50f)
        return -0.70710678f; // cos(5pi/4)

    if (x == halfPi * 2.75f)
        return -0.38268343f; // cos(11pi/8)

    if (x == halfPi * 3.f)
        return 0.f; // cos(3pi/2)

    if (x == halfPi * 3.25f)
        return 0.38268343f; // cos(13pi/8)

    if (x == halfPi * 3.50f)
        return 0.70710678f; // cos(7pi/4)

    if (x == halfPi * 3.75f)
        return 0.92387953f; // cos(15pi/8)

    if (x == halfPi * 4.f)
        return 1.f; // cos(2pi)

    const float xNormalized = normalizeAngle(x);

    if (xNormalized > halfPi)
        return -cosTaylor(pi - xNormalized);

    if (xNormalized < -halfPi)
        return -cosTaylor(xNormalized + pi);

    return cosTaylor(xNormalized);
}

} // namespace sf::base::priv
