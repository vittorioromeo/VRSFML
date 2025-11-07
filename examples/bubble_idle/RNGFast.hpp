#pragma once

#include "Xoroshiro128PlusBitGenerator.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/MakeUnsigned.hpp"


////////////////////////////////////////////////////////////
class [[nodiscard]] RNGFast
{
private:
    Xoroshiro128PlusBitGenerator m_engine;

public:
    using SeedType = sf::base::U64; //!< Type used for seeding

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor. Initializes with a fixed internal seed.
    ///
    ////////////////////////////////////////////////////////////
    explicit RNGFast() noexcept = default;

    ////////////////////////////////////////////////////////////
    /// \brief Constructor that initializes the generator with a specific seed.
    ///
    /// \param seed The seed value.
    ///
    ////////////////////////////////////////////////////////////
    explicit RNGFast(const SeedType seed) noexcept : m_engine{seed}
    {
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
        static_assert(SFML_BASE_IS_INTEGRAL(T));

        SFML_BASE_ASSERT_AND_ASSUME(min <= max);

        using UnsignedT = SFML_BASE_MAKE_UNSIGNED(T);

        const auto unsignedMin = static_cast<UnsignedT>(min);
        const auto unsignedMax = static_cast<UnsignedT>(max);

        const auto range = static_cast<sf::base::U64>(unsignedMax - unsignedMin) + sf::base::U64{1};

        return min + static_cast<T>(m_engine.next() % range);
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

        const auto  randomBits = static_cast<sf::base::U32>(m_engine.next() >> (64u - 24u)); // Extract 24 bits.
        const float normalized = static_cast<float>(randomBits) / float{(1 << 24) - 1};      // Normalize to [0, 1].

        return min + normalized * (max - min);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a random 2D vector with components in specified ranges.
    ///
    /// \param mins Vec2 containing minimum inclusive values `(x, y)`.
    /// \param maxs Vec2 containing maximum inclusive values `(x, y)`.
    ///
    /// \return A random sf::Vec2f within the specified bounds.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vec2f getVec2f(const sf::Vec2f mins, const sf::Vec2f maxs)
    {
        return {getF(mins.x, maxs.x), getF(mins.y, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a random 2D vector with components between 0 and specified maximums.
    ///
    /// \param maxs Vec2 containing maximum inclusive values `(x, y)`.
    ///
    /// \return A random sf::Vec2f within the range `[0, maxs.x]` and `[0, maxs.y]`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vec2f getVec2f(const sf::Vec2f maxs)
    {
        return {getF(0.f, maxs.x), getF(0.f, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a random point uniformly distributed within a circle.
    ///
    /// \param center Center of the circle.
    /// \param radius Radius of the circle.
    ///
    /// \return A random `sf::Vec2f` inside the specified circle.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vec2f getPointInCircle(const sf::Vec2f center, const float radius)
    {
        const float angle    = getF(0.f, sf::base::tau);
        const float distance = radius * SFML_BASE_MATH_SQRTF(getF(0.f, 1.f));

        // Compute the point's coordinates using polar-to-Cartesian conversion.
        return {center.x + distance * SFML_BASE_MATH_COSF(angle), center.y + distance * SFML_BASE_MATH_SINF(angle)};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Generates a random 2D unit vector (direction).
    ///
    /// \return A random `sf::Vec2f` with magnitude `1`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vec2f getRandomDirection()
    {
        const float angle = getF(0.f, sf::base::tau);
        return {SFML_BASE_MATH_COSF(angle), SFML_BASE_MATH_SINF(angle)};
    }
};
