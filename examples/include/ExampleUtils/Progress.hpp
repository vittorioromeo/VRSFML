#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
enum class [[nodiscard]] TickResult : sf::base::U8
{
    Running,
    JustFinished,
    AlreadyFinished
};


////////////////////////////////////////////////////////////
enum class [[nodiscard]] LoopResult : bool
{
    Running,
    Looped
};


////////////////////////////////////////////////////////////
/// \brief Pure-data base of `Progress` and `Transition`: a single
///        normalized `value` in [0, 1] plus the const queries shared
///        by both. Aggregate; no virtual; trivially copyable.
////////////////////////////////////////////////////////////
struct [[nodiscard]] ProgressBase
{
    ////////////////////////////////////////////////////////////
    float value = 0.f;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr float getElapsed() const noexcept
    {
        return value;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr float getRemaining() const noexcept
    {
        return 1.f - value;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr float getBounce() const noexcept
    {
        return 1.f - sf::base::fabs(value - 0.5f) * 2.f;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr float remap(const float min, const float max) const noexcept
    {
        return min + value * (max - min);
    }


    ////////////////////////////////////////////////////////////
    template <class F>
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr float remapEased(F&&         easingFn,
                                                                                   const float min,
                                                                                   const float max) const noexcept
    {
        return min + easingFn(value) * (max - min);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr float remapBounced(const float min, const float max) const noexcept
    {
        return min + getBounce() * (max - min);
    }


    ////////////////////////////////////////////////////////////
    template <class F>
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr float remapBouncedEased(
        F&&         easingFn,
        const float min,
        const float max) const noexcept
    {
        return min + easingFn(getBounce()) * (max - min);
    }
};


////////////////////////////////////////////////////////////
/// \brief Normalized fraction-completed accumulator in [0, 1].
///
/// Use when the caller already has `delta / duration` (i.e. the per-frame
/// fraction). For absolute-time-based counters use `Countdown` instead.
////////////////////////////////////////////////////////////
struct [[nodiscard]] Progress : ProgressBase
{
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr TickResult advance(const float delta) noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(delta >= 0.f);

        if (value == 1.f)
            return TickResult::AlreadyFinished;

        SFML_BASE_ASSERT_AND_ASSUME(value < 1.f);
        value += delta;

        if (value >= 1.f)
        {
            value = 1.f;
            return TickResult::JustFinished;
        }

        return TickResult::Running;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline constexpr LoopResult advanceLooping(const float delta) noexcept
    {
        if (advance(delta) == TickResult::Running)
            return LoopResult::Running;

        value = 0.f;
        return LoopResult::Looped;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr bool isAtEnd() const noexcept
    {
        return value == 1.f;
    }
};


////////////////////////////////////////////////////////////
/// \brief A `Progress` that remembers which direction it is currently
///        moving. Used for two-state transitions with hysteresis
///        (e.g. open/closed, awake/asleep).
///
/// `tick` advances toward 1 if `!reversed`, else toward 0.
/// `getElapsed` and `getRemaining` (inherited from `ProgressBase`) are
/// direction-agnostic -- they return the absolute fraction toward
/// state-1; query `reversed` separately if direction matters.
////////////////////////////////////////////////////////////
struct [[nodiscard]] Transition : ProgressBase
{
    ////////////////////////////////////////////////////////////
    bool reversed = false;


    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr TickResult tick(const float delta) noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(delta >= 0.f);

        if (reversed)
        {
            if (value == 0.f)
                return TickResult::AlreadyFinished;

            SFML_BASE_ASSERT_AND_ASSUME(value > 0.f);
            value -= delta;

            if (value <= 0.f)
            {
                value = 0.f;
                return TickResult::JustFinished;
            }
        }
        else
        {
            if (value == 1.f)
                return TickResult::AlreadyFinished;

            SFML_BASE_ASSERT_AND_ASSUME(value < 1.f);
            value += delta;

            if (value >= 1.f)
            {
                value = 1.f;
                return TickResult::JustFinished;
            }
        }

        return TickResult::Running;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline constexpr LoopResult tickLooping(const float delta) noexcept
    {
        if (tick(delta) == TickResult::Running)
            return LoopResult::Running;

        value = reversed ? 1.f : 0.f;
        return LoopResult::Looped;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr bool isDone() const noexcept
    {
        return reversed ? value == 0.f : value == 1.f;
    }
};


////////////////////////////////////////////////////////////
/// \brief A bare ticking-toward-zero timer in `dt`-units. No
///        notion of total duration; callers pass it explicitly to
///        `tickLooping` / `asProgress` when needed.
///
/// Use for cooldowns, buff timers, animation periods, particle
/// lifetimes, and any timeout where progress queries either don't
/// matter or the duration is contextual.
///
/// For timers that carry their original duration (and want no-arg
/// `restart()` / `asProgress()`), use `TimedCountdown` instead.
////////////////////////////////////////////////////////////
struct [[nodiscard]] Countdown
{
    ////////////////////////////////////////////////////////////
    float time = 0.f;


    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr TickResult tick(const float dt) noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(dt >= 0.f);

        if (time <= 0.f)
        {
            SFML_BASE_ASSERT(time == 0.f);
            return TickResult::AlreadyFinished;
        }

        time -= dt;

        if (time <= 0.f)
        {
            time = 0.f;
            return TickResult::JustFinished;
        }

        return TickResult::Running;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr LoopResult tickLooping(const float dt, const float restartValue) noexcept
    {
        if (tick(dt) == TickResult::Running)
            return LoopResult::Running;

        time = restartValue;
        return LoopResult::Looped;
    }


    ////////////////////////////////////////////////////////////
    constexpr void restart(const float startValue) noexcept
    {
        time = startValue;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr bool isDone() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(time >= 0.f);
        return time == 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief View as `Progress` using an explicit duration.
    ///        `1 - time/dur` so `getElapsed()` reads as fraction-elapsed.
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Progress asProgress(const float dur) const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(dur > 0.f);
        return {{1.f - time / dur}};
    }
};


////////////////////////////////////////////////////////////
/// \brief A `Countdown` that carries its original `duration`,
///        enabling no-arg `restart()` / `tickLooping()` /
///        `asProgress()` and natural progress queries.
///
/// Use for one-shot UI/animation effects with a known span:
/// fades, splashes, victory sequences, shrine activation/death.
///
/// For bare cooldowns or timeouts where the duration is contextual,
/// use `Countdown` instead.
////////////////////////////////////////////////////////////
struct [[nodiscard]] TimedCountdown : Countdown
{
    ////////////////////////////////////////////////////////////
    float duration = 0.f;


    ////////////////////////////////////////////////////////////
    using Countdown::tickLooping;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline constexpr LoopResult tickLooping(const float dt) noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(duration > 0.f);
        return Countdown::tickLooping(dt, duration);
    }


    ////////////////////////////////////////////////////////////
    using Countdown::restart;


    ////////////////////////////////////////////////////////////
    constexpr void restart() noexcept
    {
        time = duration;
    }


    ////////////////////////////////////////////////////////////
    using Countdown::asProgress;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Progress asProgress() const noexcept
    {
        return Countdown::asProgress(duration);
    }
};


////////////////////////////////////////////////////////////
/// \brief Returns `opt->asProgress().getElapsed()` if present, else `fallback`.
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline float getElapsedOr(const sf::base::Optional<TimedCountdown>& opt,
                                                            const float                               fallback) noexcept
{
    return opt.hasValue() ? opt->asProgress().getElapsed() : fallback;
}


////////////////////////////////////////////////////////////
/// \brief Returns `opt->isDone()` if present, else `false`.
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline bool isDoneOr(const sf::base::Optional<TimedCountdown>& opt) noexcept
{
    return opt.hasValue() && opt->isDone();
}
