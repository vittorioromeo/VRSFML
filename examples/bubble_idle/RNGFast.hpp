#pragma once

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/Traits/IsIntegral.hpp"
#include "SFML/Base/Traits/MakeUnsigned.hpp"


////////////////////////////////////////////////////////////
class [[nodiscard]] RNGFast
{
private:
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static inline constexpr sf::base::U64 rotl(
        const sf::base::U64 x,
        const int           k) noexcept
    {
        return (x << k) | (x >> (64 - k));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::base::U64 next() noexcept
    {
        // xoroshiro128+ generator function

        const sf::base::U64 s0 = m_state[0];
        sf::base::U64       s1 = m_state[1];

        const sf::base::U64 result = s0 + s1;

        s1 ^= s0;

        m_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
        m_state[1] = rotl(s1, 36);                   // c

        return result;
    }

public:
    using SeedType = unsigned int;

private:
    sf::base::U64 m_state[2] = {123'456'789ULL, 362'436'069ULL};

public:
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline T getI(const T min, const T max)
    {
        static_assert(sf::base::isIntegral<T>);

        SFML_BASE_ASSERT_AND_ASSUME(min <= max);

        const auto range = static_cast<SFML_BASE_MAKE_UNSIGNED(T)>(max - min) + T(1);
        return min + static_cast<T>(next() % range);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline float getF(const float min, const float max)
    {
        SFML_BASE_ASSERT_AND_ASSUME(min <= max);

        // Returns a float in the inclusive range [min, max].

        // We extract 24 random bits, which is enough to fill the 23-bit mantissa of a float,
        // and normalize by dividing by (2^24 - 1).

        const auto  randomBits = static_cast<sf::base::U32>(next() >> (64u - 24u));     // Extract 24 bits.
        const float normalized = static_cast<float>(randomBits) / float((1 << 24) - 1); // Normalize to [0, 1].

        return min + normalized * (max - min);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getVec2f(const sf::Vector2f mins, const sf::Vector2f maxs)
    {
        return {getF(mins.x, maxs.x), getF(mins.y, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getVec2f(const sf::Vector2f maxs)
    {
        return {getF(0.f, maxs.x), getF(0.f, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getPointInCircle(const sf::Vector2f center,
                                                                                         const float        radius)
    {
        const float angle    = getF(0.f, sf::base::tau);
        const float distance = radius * sf::base::sqrt(getF(0.f, 1.f));

        // Compute the point's coordinates using polar-to-Cartesian conversion.
        return {center.x + distance * sf::base::cos(angle), center.y + distance * sf::base::sin(angle)};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getRandomDirection()
    {
        const float angle = getF(0.f, sf::base::tau);
        return {sf::base::cos(angle), sf::base::sin(angle)};
    }
};
