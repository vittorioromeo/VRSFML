#pragma once

#include "RNGSeedType.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Diagnostic/AssertAndAssume.hpp"

#include "Zancle/Math/Constants.hpp"
#include "Zancle/Math/Cos.hpp"
#include "Zancle/Math/Sin.hpp"
#include "Zancle/Math/Sqrt.hpp"

#include <random>


////////////////////////////////////////////////////////////
class [[nodiscard]] RNG
{
public:
    using SeedType = RNGSeedType;

private:
    SeedType          m_seed;
    std::minstd_rand0 m_engine;

public:
    ////////////////////////////////////////////////////////////
    explicit RNG(const SeedType seed) : m_seed{seed}, m_engine{seed}
    {
        m_engine.discard(1);
    }

    ////////////////////////////////////////////////////////////
    void reseed(const SeedType seed) noexcept
    {
        m_seed = seed;
        m_engine.seed(seed);
        m_engine.discard(1);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SeedType getSeed() const noexcept
    {
        return m_seed;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::minstd_rand0& getEngine() noexcept
    {
        return m_engine;
    }

    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline T getI(const T min, const T max)
    {
        ZA_ASSERT_AND_ASSUME(min <= max);
        return std::uniform_int_distribution<T>{min, max}(m_engine);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline float getF(const float min, const float max)
    {
        ZA_ASSERT_AND_ASSUME(min <= max);
        return std::uniform_real_distribution<float>{min, max}(m_engine);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline za::Vec2f getVec2f(const za::Vec2f mins, const za::Vec2f maxs)
    {
        return {getF(mins.x, maxs.x), getF(mins.y, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline za::Vec2f getVec2f(const za::Vec2f maxs)
    {
        return {getF(0.f, maxs.x), getF(0.f, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline za::Vec2f getPointInCircle(const za::Vec2f center, const float radius)
    {
        const float angle    = getF(0.f, za::tau);
        const float distance = radius * za::sqrt(getF(0.f, 1.f));

        // Compute the point's coordinates using polar-to-Cartesian conversion.
        return {center.x + distance * za::cos(angle), center.y + distance * za::sin(angle)};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline za::Vec2f getDirVec2f()
    {
        const float angle = getF(0.f, za::tau);
        return {za::cos(angle), za::sin(angle)};
    }
};
