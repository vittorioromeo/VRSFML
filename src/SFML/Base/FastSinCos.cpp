#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Array.hpp"
#include "SFML/Base/FastSinCos.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
constexpr auto sinTableData{[]
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-float-conversion"
    Array<float, sinCount> data{
#include "SFML/Base/FastSinCosTable.inl"
    };
#pragma GCC diagnostic pop

    data[fastSinIdx(halfPi * 0.f) & sinMask]   = 0.f;          // sin(0)
    data[fastSinIdx(halfPi * 0.25f) & sinMask] = 0.38268343f;  // sin(pi/8)
    data[fastSinIdx(halfPi * 0.50f) & sinMask] = 0.70710678f;  // sin(pi/4)
    data[fastSinIdx(halfPi * 0.75f) & sinMask] = 0.92387953f;  // sin(3pi/8)
    data[fastSinIdx(halfPi * 1.f) & sinMask]   = 1.f;          // sin(pi/2)
    data[fastSinIdx(halfPi * 1.25f) & sinMask] = 0.92387953f;  // sin(5pi/8)
    data[fastSinIdx(halfPi * 1.50f) & sinMask] = 0.70710678f;  // sin(3pi/4)
    data[fastSinIdx(halfPi * 1.75f) & sinMask] = 0.38268343f;  // sin(7pi/8)
    data[fastSinIdx(halfPi * 2.f) & sinMask]   = 0.f;          // sin(pi)
    data[fastSinIdx(halfPi * 2.25f) & sinMask] = -0.38268343f; // sin(9pi/8)
    data[fastSinIdx(halfPi * 2.50f) & sinMask] = -0.70710678f; // sin(5pi/4)
    data[fastSinIdx(halfPi * 2.75f) & sinMask] = -0.92387953f; // sin(11pi/8)
    data[fastSinIdx(halfPi * 3.f) & sinMask]   = -1.f;         // sin(3pi/2)
    data[fastSinIdx(halfPi * 3.25f) & sinMask] = -0.92387953f; // sin(13pi/8)
    data[fastSinIdx(halfPi * 3.50f) & sinMask] = -0.70710678f; // sin(7pi/4)
    data[fastSinIdx(halfPi * 3.75f) & sinMask] = -0.38268343f; // sin(15pi/8)
    data[fastSinIdx(halfPi * 4.f) & sinMask]   = 0.f;          // sin(2pi)

    data[fastCosIdx(halfPi * 0.f) & sinMask]   = 1.f;          // cos(0)
    data[fastCosIdx(halfPi * 0.25f) & sinMask] = 0.92387953f;  // cos(pi/8)
    data[fastCosIdx(halfPi * 0.50f) & sinMask] = 0.70710678f;  // cos(pi/4)
    data[fastCosIdx(halfPi * 0.75f) & sinMask] = 0.38268343f;  // cos(3pi/8)
    data[fastCosIdx(halfPi * 1.f) & sinMask]   = 0.f;          // cos(pi/2)
    data[fastCosIdx(halfPi * 1.25f) & sinMask] = -0.38268343f; // cos(5pi/8)
    data[fastCosIdx(halfPi * 1.50f) & sinMask] = -0.70710678f; // cos(3pi/4)
    data[fastCosIdx(halfPi * 1.75f) & sinMask] = -0.92387953f; // cos(7pi/8)
    data[fastCosIdx(halfPi * 2.f) & sinMask]   = -1.f;         // cos(pi)
    data[fastCosIdx(halfPi * 2.25f) & sinMask] = -0.92387953f; // cos(9pi/8)
    data[fastCosIdx(halfPi * 2.50f) & sinMask] = -0.70710678f; // cos(5pi/4)
    data[fastCosIdx(halfPi * 2.75f) & sinMask] = -0.38268343f; // cos(11pi/8)
    data[fastCosIdx(halfPi * 3.f) & sinMask]   = 0.f;          // cos(3pi/2)
    data[fastCosIdx(halfPi * 3.25f) & sinMask] = 0.38268343f;  // cos(13pi/8)
    data[fastCosIdx(halfPi * 3.50f) & sinMask] = 0.70710678f;  // cos(7pi/4)
    data[fastCosIdx(halfPi * 3.75f) & sinMask] = 0.92387953f;  // cos(15pi/8)
    data[fastCosIdx(halfPi * 4.f) & sinMask]   = 1.f;          // cos(2pi)

    return data;
}()};

} // namespace sf::base::priv
