#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
enum : U32
{
    sinBits  = 16u,
    sinMask  = ~(-1u << sinBits),
    sinCount = sinMask + 1u // 65536
};


////////////////////////////////////////////////////////////
inline constexpr float radToIndex = static_cast<float>(sinCount) / tau;


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr U32 fastSinIdx(float radians) noexcept
{
    return static_cast<U32>(radians * radToIndex);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr U32 fastCosIdx(float radians) noexcept
{
    return fastSinIdx(radians) + 16384u;
}


////////////////////////////////////////////////////////////
inline constexpr struct SinTable
{
    ////////////////////////////////////////////////////////////
    float data[sinCount]{
#include "SFML/Base/FastSinCosTable.inl"
    };


    ////////////////////////////////////////////////////////////
    static_assert(sizeof(data) == (sinCount) * sizeof(float));


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] constexpr SinTable() noexcept
    {
        data[fastSinIdx(halfPi * 0.f) & sinMask]  = 0.f;
        data[fastSinIdx(halfPi * 0.5f) & sinMask] = 0.70710678118f;
        data[fastSinIdx(halfPi * 1.f) & sinMask]  = 1.f;
        data[fastSinIdx(halfPi * 1.5f) & sinMask] = 0.70710678118f;
        data[fastSinIdx(halfPi * 2.f) & sinMask]  = 0.f;
        data[fastSinIdx(halfPi * 2.5f) & sinMask] = -0.70710678118f;
        data[fastSinIdx(halfPi * 3.f) & sinMask]  = -1.f;
        data[fastSinIdx(halfPi * 3.5f) & sinMask] = -0.70710678118f;
        data[fastSinIdx(halfPi * 4.f) & sinMask]  = 0.f;

        data[fastCosIdx(halfPi * 0.f) & sinMask]  = 1.f;
        data[fastCosIdx(halfPi * 0.5f) & sinMask] = 0.70710678118f;
        data[fastCosIdx(halfPi * 1.f) & sinMask]  = 0.f;
        data[fastCosIdx(halfPi * 1.5f) & sinMask] = -0.70710678118f;
        data[fastCosIdx(halfPi * 2.f) & sinMask]  = -1.f;
        data[fastCosIdx(halfPi * 2.5f) & sinMask] = -0.70710678118f;
        data[fastCosIdx(halfPi * 3.f) & sinMask]  = 0.f;
        data[fastCosIdx(halfPi * 3.5f) & sinMask] = 0.70710678118f;
        data[fastCosIdx(halfPi * 4.f) & sinMask]  = 1.f;
    }
} sinTable;

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float fastSin(float radians) noexcept
{
    SFML_BASE_ASSERT(radians >= 0.f && radians <= tau);
    return priv::sinTable.data[priv::fastSinIdx(radians) & priv::sinMask];
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float fastCos(float radians) noexcept
{
    SFML_BASE_ASSERT(radians >= 0.f && radians <= tau);
    return priv::sinTable.data[priv::fastCosIdx(radians) & priv::sinMask];
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto fastSinCos(float radians) noexcept
{
    SFML_BASE_ASSERT(radians >= 0.f && radians <= tau);

    struct Result
    {
        float sin, cos;
    };

    const auto sinIndex = static_cast<U32>(radians * priv::radToIndex);
    return Result{priv::sinTable.data[sinIndex & priv::sinMask], priv::sinTable.data[(sinIndex + 16384u) & priv::sinMask]};
}

} // namespace sf::base
