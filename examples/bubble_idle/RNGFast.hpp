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
/// \brief A fast pseudo-random number generator using the xoroshiro128+ algorithm.
///
/// Provides methods for generating integers, floats, vectors,
/// and random directions/points within shapes. Can be seeded.
///
/// Satisfies the C++ `UniformRandomBitGenerator` concept.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] RNGFast
{
public:
    using result_type = sf::base::U64; //!< Type returned by `operator()` and `next()`
    using SeedType    = sf::base::U64; //!< Type used for seeding

private:
    ////////////////////////////////////////////////////////////
    /// \brief Rotates the bits of `x` left by `k` positions.
    ///
    /// \param x Value to rotate
    /// \param k Number of positions to rotate
    ///
    /// \return Rotated value
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static inline constexpr sf::base::U64 rotl(
        const sf::base::U64 x,
        const int           k) noexcept
    {
        return (x << k) | (x >> (64 - k));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Implements the SplitMix64 algorithm to initialize state.
    ///
    /// Used internally for seeding the main generator state from a single seed value.
    ///
    /// \param seed Input seed value
    ///
    /// \return A 64-bit pseudo-random number derived from the seed
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] static inline sf::base::U64 splitmix64(sf::base::U64& seed) noexcept
    {
        seed += 0x9e'37'79'b9'7f'4a'7c'15ULL;

        sf::base::U64 z = seed;

        z = (z ^ (z >> 30)) * 0xbf'58'47'6d'1c'e4'e5'b9ULL;
        z = (z ^ (z >> 27)) * 0x94'd0'49'bb'13'31'11'ebULL;

        return z ^ (z >> 31);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Initializes the generator state from a single seed value.
    ///
    /// \param seed The seed value to use.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] inline void seedInternal(SeedType seedValue) noexcept
    {
        // Use SplitMix64 to generate the initial 128-bit state
        m_state[0] = splitmix64(seedValue);
        m_state[1] = splitmix64(seedValue);

        // Ensure the initial state is not all zeros, which is invalid for xoroshiro128+
        if (m_state[0] == 0ULL && m_state[1] == 0ULL)
        {
            m_state[0] = DefaultSeed::State0; // Fallback to default non-zero state
            m_state[1] = DefaultSeed::State1;
        }
    }

    ////////////////////////////////////////////////////////////
    // Constants for the default seed if none is provided
    enum DefaultSeed : sf::base::U64
    {
        State0 = 123'456'789'123'456'789ULL,
        State1 = 987'654'321'987'654'321ULL
    };

    ////////////////////////////////////////////////////////////
    sf::base::U64 m_state[2]{}; //!< Internal state of the generator

public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor. Initializes with a fixed internal seed.
    ///
    ////////////////////////////////////////////////////////////
    explicit RNGFast() noexcept : m_state{DefaultSeed::State0, DefaultSeed::State1}
    {
        // Ensure default state isn't all zeros (though these constants aren't)
        SFML_BASE_ASSERT_AND_ASSUME(m_state[0] != 0 || m_state[1] != 0);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Constructor that initializes the generator with a specific seed.
    ///
    /// \param seed The seed value.
    ///
    ////////////////////////////////////////////////////////////
    explicit RNGFast(SeedType seed) noexcept
    {
        seedInternal(seed);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates the next 64-bit pseudo-random number.
    ///
    /// \return A 64-bit unsigned integer.
    ///
    /// Implements the core xoroshiro128+ algorithm step.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline result_type next() noexcept
    {
        const sf::base::U64 s0 = m_state[0];
        sf::base::U64       s1 = m_state[1];

        const sf::base::U64 result = s0 + s1; // The '+' part of xoroshiro128+

        s1 ^= s0;

        m_state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        m_state[1] = rotl(s1, 37);

        return result;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates the next 64-bit pseudo-random number (UniformRandomBitGenerator interface).
    ///
    /// \return A 64-bit unsigned integer.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline result_type operator()() noexcept
    {
        return next();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Returns the minimum value potentially generated (UniformRandomBitGenerator interface).
    ///
    /// \return `0`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr result_type min() noexcept
    {
        return 0;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Returns the maximum value potentially generated (UniformRandomBitGenerator interface).
    ///
    /// \return Maximum value of `result_type` (`sf::base::U64`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr result_type max() noexcept
    {
        return static_cast<sf::base::U64>(-1);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a pseudo-random integer within a specified range `[min, max]`.
    ///
    /// \tparam T An integral type.
    ///
    /// \param min Minimum inclusive value.
    /// \param max Maximum inclusive value.
    ///
    /// \return A pseudo-random integer in the range `[min, max]`.
    ///
    /// \warning Uses modulo biasing, which might be unsuitable for applications
    ///          requiring perfect uniformity, especially with large ranges.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline T getI(const T min, const T max)
    {
        static_assert(sf::base::isIntegral<T>);

        SFML_BASE_ASSERT_AND_ASSUME(min <= max);

        using UnsignedT = SFML_BASE_MAKE_UNSIGNED(T);

        const auto unsignedMin = static_cast<UnsignedT>(min);
        const auto unsignedMax = static_cast<UnsignedT>(max);

        const auto range = static_cast<sf::base::U64>(unsignedMax - unsignedMin) + sf::base::U64{1};

        return min + static_cast<T>(next() % range);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a pseudo-random float within a specified range `[min, max]`.
    ///
    /// \param min Minimum inclusive value.
    /// \param max Maximum inclusive value.
    ///
    /// \return A pseudo-random float in the range `[min, max]`.
    ///
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
    /// \brief Generates a random 2D vector with components in specified ranges.
    ///
    /// \param mins Vector containing minimum inclusive values `(x, y)`.
    /// \param maxs Vector containing maximum inclusive values `(x, y)`.
    ///
    /// \return A random sf::Vector2f within the specified bounds.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getVec2f(const sf::Vector2f mins, const sf::Vector2f maxs)
    {
        return {getF(mins.x, maxs.x), getF(mins.y, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a random 2D vector with components between 0 and specified maximums.
    ///
    /// \param maxs Vector containing maximum inclusive values `(x, y)`.
    ///
    /// \return A random sf::Vector2f within the range `[0, maxs.x]` and `[0, maxs.y]`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getVec2f(const sf::Vector2f maxs)
    {
        return {getF(0.f, maxs.x), getF(0.f, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a random point uniformly distributed within a circle.
    ///
    /// \param center Center of the circle.
    /// \param radius Radius of the circle.
    ///
    /// \return A random `sf::Vector2f` inside the specified circle.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getPointInCircle(const sf::Vector2f center,
                                                                                         const float        radius)
    {
        const float angle    = getF(0.f, sf::base::tau);
        const float distance = radius * SFML_BASE_MATH_SQRTF(getF(0.f, 1.f));

        // Compute the point's coordinates using polar-to-Cartesian conversion.
        return {center.x + distance * SFML_BASE_MATH_COSF(angle), center.y + distance * SFML_BASE_MATH_SINF(angle)};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a random 2D unit vector (direction).
    ///
    /// \return A random `sf::Vector2f` with magnitude `1`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2f getRandomDirection()
    {
        const float angle = getF(0.f, sf::base::tau);
        return {SFML_BASE_MATH_COSF(angle), SFML_BASE_MATH_SINF(angle)};
    }
};
