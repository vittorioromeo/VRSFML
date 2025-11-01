// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SinCosLookup.hpp"

#include "SFML/Base/Array.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
constexpr Array<float, sinCount> sinTableData{[]
{
#pragma GCC diagnostic push

#if defined(__clang__)
    #pragma GCC diagnostic ignored "-Wimplicit-float-conversion"
#elif defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif
    Array<float, sinCount> data{
#include "SFML/Base/Priv/SinCosLookupTable.inl"
    };

#pragma GCC diagnostic pop

    data[sinLookupIdx(halfPi * 0.f) & sinMask]   = 0.f;          // sin(0)
    data[sinLookupIdx(halfPi * 0.25f) & sinMask] = 0.38268343f;  // sin(pi/8)
    data[sinLookupIdx(halfPi * 0.50f) & sinMask] = 0.70710678f;  // sin(pi/4)
    data[sinLookupIdx(halfPi * 0.75f) & sinMask] = 0.92387953f;  // sin(3pi/8)
    data[sinLookupIdx(halfPi * 1.f) & sinMask]   = 1.f;          // sin(pi/2)
    data[sinLookupIdx(halfPi * 1.25f) & sinMask] = 0.92387953f;  // sin(5pi/8)
    data[sinLookupIdx(halfPi * 1.50f) & sinMask] = 0.70710678f;  // sin(3pi/4)
    data[sinLookupIdx(halfPi * 1.75f) & sinMask] = 0.38268343f;  // sin(7pi/8)
    data[sinLookupIdx(halfPi * 2.f) & sinMask]   = 0.f;          // sin(pi)
    data[sinLookupIdx(halfPi * 2.25f) & sinMask] = -0.38268343f; // sin(9pi/8)
    data[sinLookupIdx(halfPi * 2.50f) & sinMask] = -0.70710678f; // sin(5pi/4)
    data[sinLookupIdx(halfPi * 2.75f) & sinMask] = -0.92387953f; // sin(11pi/8)
    data[sinLookupIdx(halfPi * 3.f) & sinMask]   = -1.f;         // sin(3pi/2)
    data[sinLookupIdx(halfPi * 3.25f) & sinMask] = -0.92387953f; // sin(13pi/8)
    data[sinLookupIdx(halfPi * 3.50f) & sinMask] = -0.70710678f; // sin(7pi/4)
    data[sinLookupIdx(halfPi * 3.75f) & sinMask] = -0.38268343f; // sin(15pi/8)
    data[sinLookupIdx(halfPi * 4.f) & sinMask]   = 0.f;          // sin(2pi)

    data[cosLookupIdx(halfPi * 0.f) & sinMask]   = 1.f;          // cos(0)
    data[cosLookupIdx(halfPi * 0.25f) & sinMask] = 0.92387953f;  // cos(pi/8)
    data[cosLookupIdx(halfPi * 0.50f) & sinMask] = 0.70710678f;  // cos(pi/4)
    data[cosLookupIdx(halfPi * 0.75f) & sinMask] = 0.38268343f;  // cos(3pi/8)
    data[cosLookupIdx(halfPi * 1.f) & sinMask]   = 0.f;          // cos(pi/2)
    data[cosLookupIdx(halfPi * 1.25f) & sinMask] = -0.38268343f; // cos(5pi/8)
    data[cosLookupIdx(halfPi * 1.50f) & sinMask] = -0.70710678f; // cos(3pi/4)
    data[cosLookupIdx(halfPi * 1.75f) & sinMask] = -0.92387953f; // cos(7pi/8)
    data[cosLookupIdx(halfPi * 2.f) & sinMask]   = -1.f;         // cos(pi)
    data[cosLookupIdx(halfPi * 2.25f) & sinMask] = -0.92387953f; // cos(9pi/8)
    data[cosLookupIdx(halfPi * 2.50f) & sinMask] = -0.70710678f; // cos(5pi/4)
    data[cosLookupIdx(halfPi * 2.75f) & sinMask] = -0.38268343f; // cos(11pi/8)
    data[cosLookupIdx(halfPi * 3.f) & sinMask]   = 0.f;          // cos(3pi/2)
    data[cosLookupIdx(halfPi * 3.25f) & sinMask] = 0.38268343f;  // cos(13pi/8)
    data[cosLookupIdx(halfPi * 3.50f) & sinMask] = 0.70710678f;  // cos(7pi/4)
    data[cosLookupIdx(halfPi * 3.75f) & sinMask] = 0.92387953f;  // cos(15pi/8)
    data[cosLookupIdx(halfPi * 4.f) & sinMask]   = 1.f;          // cos(2pi)

    return data;
}()};

} // namespace sf::base::priv
