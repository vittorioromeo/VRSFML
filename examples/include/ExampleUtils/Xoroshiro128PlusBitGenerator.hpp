#pragma once

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
/// \brief A fast pseudo-random number generator using the xoroshiro128+ algorithm.
///
/// Provides methods for generating integers, floats, vectors,
/// and random directions/points within shapes. Can be seeded.
///
/// Satisfies the C++ `UniformRandomBitGenerator` concept.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] Xoroshiro128PlusBitGenerator
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
    enum [[nodiscard]] DefaultSeed : sf::base::U64
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
    explicit Xoroshiro128PlusBitGenerator() noexcept : m_state{DefaultSeed::State0, DefaultSeed::State1}
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
    explicit Xoroshiro128PlusBitGenerator(const SeedType seed) noexcept
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
    [[nodiscard, gnu::always_inline]] static constexpr result_type min() noexcept
    {
        return 0u;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Returns the maximum value potentially generated (UniformRandomBitGenerator interface).
    ///
    /// \return Maximum value of `result_type` (`sf::base::U64`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] static constexpr result_type max() noexcept
    {
        return static_cast<sf::base::U64>(-1);
    }
};
