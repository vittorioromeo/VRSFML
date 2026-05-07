#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/AssertAndAssume.hpp"
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr U32 sinLookupIdx(const float radians) noexcept
{
    return static_cast<U32>(radians * radToIndex);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr U32 cosLookupIdx(const float radians) noexcept
{
    return sinLookupIdx(radians) + 16'384u;
}


////////////////////////////////////////////////////////////
extern const float (&sinTableData)[sinCount];
static_assert(sizeof(sinTableData) == sinCount * sizeof(float));

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Lookup-table sine for `radians` (faster than `std::sin`, less precise)
///
/// \param radians Angle in radians. Must be in the range `[0, 2*Pi]`.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float sinLookup(const float radians) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(radians >= 0.f && radians <= tau);

    if consteval
    {
        return priv::constexprSin(radians);
    }
    else
    {
        return priv::sinTableData[priv::sinLookupIdx(radians) & priv::sinMask];
    }
}


////////////////////////////////////////////////////////////
/// \brief Lookup-table cosine for `radians` (faster than `std::cos`, less precise)
///
/// \param radians Angle in radians. Must be in the range `[0, 2*Pi]`.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float cosLookup(const float radians) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(radians >= 0.f && radians <= tau);

    if consteval
    {
        return priv::constexprCos(radians);
    }
    else
    {
        return priv::sinTableData[priv::cosLookupIdx(radians) & priv::sinMask];
    }
}


////////////////////////////////////////////////////////////
/// \brief Combined sine+cosine lookup for `radians` (faster than separate calls)
///
/// \param radians Angle in radians. Must be in the range `[0, 2*Pi]`.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto sinCosLookup(const float radians) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(radians >= 0.f && radians <= tau);

    struct Result
    {
        float sin, cos;
    };

    if consteval
    {
        return Result{priv::constexprSin(radians), priv::constexprCos(radians)};
    }
    else
    {
        const auto sinIndex = static_cast<U32>(radians * priv::radToIndex);

        return Result{priv::sinTableData[sinIndex & priv::sinMask],
                      priv::sinTableData[(sinIndex + 16'384u) & priv::sinMask]};
    }
}

} // namespace sf::base
