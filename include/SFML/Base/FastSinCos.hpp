#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Array.hpp"
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtins/IsConstantEvaluated.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Priv/ConstexprSinCos.hpp"


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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr U32 fastSinIdx(const float radians) noexcept
{
    return static_cast<U32>(radians * radToIndex);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr U32 fastCosIdx(const float radians) noexcept
{
    return fastSinIdx(radians) + 16'384u;
}


////////////////////////////////////////////////////////////
extern const Array<float, sinCount> sinTableData;
static_assert(sizeof(sinTableData) == sinCount * sizeof(float));

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Fast sine calculation using a lookup table.
///
/// Calculates an approximation of `sin(radians)` using a precomputed lookup table.
/// This is faster than `std::sin` but less precise.
///
/// \param radians Angle in radians. Must be in the range `[0, 2*Pi]`.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float fastSin(const float radians) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(radians >= 0.f && radians <= tau);

    if (SFML_BASE_IS_CONSTANT_EVALUATED())
        return priv::constexprSin(radians);

    return priv::sinTableData[priv::fastSinIdx(radians) & priv::sinMask];
}


////////////////////////////////////////////////////////////
/// \brief Fast cosine calculation using a lookup table.
///
/// Calculates an approximation of `cos(radians)` using a precomputed lookup table.
/// This is faster than `std::cos` but less precise.
///
/// \param radians Angle in radians. Must be in the range `[0, 2*Pi]`.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float fastCos(const float radians) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(radians >= 0.f && radians <= tau);

    if (SFML_BASE_IS_CONSTANT_EVALUATED())
        return priv::constexprCos(radians);

    return priv::sinTableData[priv::fastCosIdx(radians) & priv::sinMask];
}


////////////////////////////////////////////////////////////
/// \brief Fast sine and cosine calculation using a lookup table.
///
/// Calculates approximations of `sin(radians)` and `cos(radians)` simultaneously
/// using a precomputed lookup table. Faster than separate calls to `fastSin` and `fastCos`.
///
/// \param radians Angle in radians. Must be in the range `[0, 2*Pi]`.
/// \return A struct containing the `sin` and `cos` results.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto fastSinCos(const float radians) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(radians >= 0.f && radians <= tau);

    struct Result
    {
        float sin, cos;
    };

    const auto sinIndex = static_cast<U32>(radians * priv::radToIndex);
    return Result{priv::sinTableData[sinIndex & priv::sinMask], priv::sinTableData[(sinIndex + 16'384u) & priv::sinMask]};
}

} // namespace sf::base
