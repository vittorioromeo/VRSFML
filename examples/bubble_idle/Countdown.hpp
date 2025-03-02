#pragma once

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
enum class [[nodiscard]] CountdownStatusLoop : bool
{
    Running,
    Looping
};

////////////////////////////////////////////////////////////
enum class [[nodiscard]] CountdownStatusStop : sf::base::U8
{
    Running,
    JustFinished,
    AlreadyFinished
};

////////////////////////////////////////////////////////////
struct [[nodiscard]] Countdown // TODO P2: turn to free funcs, or remove for Timer
{
    float value = 0.f;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] CountdownStatusLoop updateAndLoop(const float deltaTimeMs, const float startingValue)
    {
        if (updateAndStop(deltaTimeMs) == CountdownStatusStop::Running)
            return CountdownStatusLoop::Running;

        value = startingValue;
        return CountdownStatusLoop::Looping;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] CountdownStatusStop updateAndStop(const float deltaTimeMs)
    {
        if (value > 0.f)
        {
            value -= deltaTimeMs;

            if (value <= 0.f)
            {
                value = 0.f;
                return CountdownStatusStop::JustFinished;
            }

            return CountdownStatusStop::Running;
        }

        SFML_BASE_ASSERT(value == 0.f);
        return CountdownStatusStop::AlreadyFinished;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAndIsActive(const float deltaTimeMs)
    {
        return updateAndStop(deltaTimeMs) == CountdownStatusStop::Running;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool isDone() const
    {
        SFML_BASE_ASSERT(value >= 0.f);
        return value == 0.f;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] float getInvProgress(const float startingValue) const
    {
        return value / startingValue;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] float getProgressBounced(const float startingValue) const
    {
        return 1.f - sf::base::fabs(getInvProgress(startingValue) - 0.5f) * 2.f;
    }
};

////////////////////////////////////////////////////////////
struct [[nodiscard]] TargetedCountdown : Countdown
{
    float startingValue;

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void restart()
    {
        Countdown::value = startingValue;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] CountdownStatusLoop updateAndLoop(const float deltaTimeMs)
    {
        return Countdown::updateAndLoop(deltaTimeMs, startingValue);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] CountdownStatusStop updateAndStop(const float deltaTimeMs)
    {
        return Countdown::updateAndStop(deltaTimeMs);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] float getInvProgress() const
    {
        return value / startingValue;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] float getProgress() const
    {
        return 1.f - getInvProgress();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] float getProgressBounced() const
    {
        return 1.f - sf::base::fabs(getInvProgress() - 0.5f) * 2.f;
    }
};

////////////////////////////////////////////////////////////
struct [[nodiscard]] OptionalTargetedCountdown : private sf::base::Optional<TargetedCountdown>
{
    using BaseType = sf::base::Optional<TargetedCountdown>;
    using BaseType::BaseType;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] float getProgress() const
    {
        return hasValue() ? (*this)->getProgress() : 0.f;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] bool isDone() const
    {
        return hasValue() && (*this)->isDone();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] BaseType& asBase()
    {
        return *this;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const BaseType& asBase() const
    {
        return *this;
    }

    ////////////////////////////////////////////////////////////
    using BaseType::emplace;
    using BaseType::hasValue;
    using BaseType::reset;
    using BaseType::value;
    using BaseType::operator->;
};
