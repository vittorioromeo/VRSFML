#pragma once

////////////////////////////////////////////////////////////
// Header-only macro-based stackless coroutine library.
//
// API surface:
//   sfex::Coroutine                      -- base class for coroutines
//   sfex::CoroutineParallelMixin<Time>   -- mixin for parallel composition
//   sfex::tickInParallel                 -- one-frame tick of one child
//
//   SFEX_CO_BEGIN / END / YIELD / RETURN -- core control flow
//   SFEX_CO_AWAIT                        -- run a child to completion
//   SFEX_CO_WAIT_UNTIL / WAIT_WHILE      -- conditional yield loops
//   SFEX_CO_AWAIT_ALL / AWAIT_ANY        -- N-way parallel composition
//
// User-provided hooks (found via ADL on the yield type):
//   `bool isFinished(YieldType)`
//       Required by `SFEX_CO_AWAIT`. Returns `true` for the "finished"
//       kind of yield (typically `Done{}`).
//
//   `void yieldApply(YieldType, Child&)`
//       Required by parallel composition. Interprets the child's yield
//       by setting `child.parallelWait` for a "wait N seconds" kind, or
//       `child.parallelDone` for a "child is done" kind.
//
// Hard rule:
//   Any local whose value must survive a yield must be a struct member
//   of the coroutine, NOT a function-local. Function-locals declared
//   before a yield are silently re-initialized every call, or rejected
//   by the compiler ("jump into protected scope") inside nested scopes.
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Builtin/Pragma.hpp"
#include "SFML/Base/Builtin/Unreachable.hpp"
#include "SFML/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
#define SFEX_PRIV_CO_CASE_WARNINGS_PUSH    \
    SFML_BASE_PRAGMA(GCC diagnostic push); \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wimplicit-fallthrough")

#if defined(__clang__)
    #define SFEX_PRIV_CO_C2Y_PUSH SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wc2y-extensions")
#else
    #define SFEX_PRIV_CO_C2Y_PUSH static_assert(true)
#endif

#define SFEX_PRIV_CO_CASE_WARNINGS_POP SFML_BASE_PRAGMA(GCC diagnostic pop)


namespace sfex
{
////////////////////////////////////////////////////////////
/// \brief Base class for stackless coroutines
///
/// Inherit from `Coroutine` to make a struct behave as a coroutine,
/// then define `Yield operator()(Args...)` with `SFEX_CO_BEGIN` /
/// `SFEX_CO_END` bracketing the body.
///
/// The `state` field tracks the current resumption point and is the
/// only piece of bookkeeping the library adds to the user's struct.
/// Saving and restoring a coroutine is just a matter of saving and
/// restoring `state` along with whatever members the user has added.
///
/// \see `sfex::CoroutineParallelMixin`, `SFEX_CO_BEGIN`, `SFEX_CO_END`
///
////////////////////////////////////////////////////////////
struct Coroutine
{
    sf::base::U32 state = 0;
};


////////////////////////////////////////////////////////////
/// \brief Mixin enabling participation in parallel composition
///
/// Inherit alongside `sfex::Coroutine` to use a coroutine as a child of
/// `SFEX_CO_AWAIT_ALL` / `SFEX_CO_AWAIT_ANY`. The two members are read
/// and written by the parallel drivers between ticks: `parallelWait`
/// is the per-child wait timer (decremented every frame the child is
/// asleep), and `parallelDone` is set when the child has finished.
///
/// \tparam Time The caller's time domain (`float` for real-time
///              seconds, `int` for tick counts, etc.).
///
/// \see `sfex::Coroutine`, `SFEX_CO_AWAIT_ALL`, `SFEX_CO_AWAIT_ANY`
///
////////////////////////////////////////////////////////////
template <typename Time = float>
struct CoroutineParallelMixin
{
    Time parallelWait{};
    bool parallelDone = false;
};


////////////////////////////////////////////////////////////
/// \brief Advance one parallel-composition child by one frame
///
/// Calls the child's `operator()(args...)` once unless the child is in
/// a wait window or already finished. Use this to write a custom
/// parallel driver; `SFEX_CO_AWAIT_ALL` / `SFEX_CO_AWAIT_ANY` use it
/// internally.
///
/// Requires (via ADL on the yield type) a free function
/// `void yieldApply(YieldType, Child&)` that interprets the child's
/// yielded value -- typically by setting `child.parallelWait` for a
/// "wait N seconds" yield and `child.parallelDone` for a "child is
/// done" yield.
///
/// \param child The child coroutine (must inherit `CoroutineParallelMixin`).
/// \param dt    Elapsed time this frame.
/// \param args  Arguments forwarded to the child's `operator()`.
///
/// \return `true` while the child is still running, `false` once it
///         has finished.
///
////////////////////////////////////////////////////////////
template <typename Child, typename Time, typename... CallArgs>
[[nodiscard]] bool tickInParallel(Child& child, Time dt, CallArgs&&... args)
{
    if (child.parallelDone)
        return false;

    if (child.parallelWait > Time{})
    {
        child.parallelWait -= dt;

        if (child.parallelWait < Time{})
            child.parallelWait = Time{};

        return true;
    }

    yieldApply(child(static_cast<CallArgs&&>(args)...), child);
    return !child.parallelDone;
}


namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Internal: tick every child once, return `true` once all are finished
///
/// Backs `SFEX_CO_AWAIT_ALL`. Not intended to be called directly.
///
////////////////////////////////////////////////////////////
template <typename Time, typename Ctx, typename... Children>
[[nodiscard]] bool tickAllInParallel(Time dt, Ctx& ctx, Children&... children)
{
    for (const bool running : {tickInParallel(children, dt, ctx)...})
        if (running)
            return false;

    return true;
}


////////////////////////////////////////////////////////////
/// \brief Internal: tick every child once, return `true` as soon as any finishes
///
/// Backs `SFEX_CO_AWAIT_ANY`. Not intended to be called directly.
///
////////////////////////////////////////////////////////////
template <typename Time, typename Ctx, typename... Children>
[[nodiscard]] bool tickAnyInParallel(Time dt, Ctx& ctx, Children&... children)
{
    for (const bool running : {tickInParallel(children, dt, ctx)...})
        if (!running)
            return true;

    return false;
}

} // namespace priv

} // namespace sfex


////////////////////////////////////////////////////////////
/// \brief Open a coroutine body
///
/// Place at the top of `Yield operator()(...)`. Sets up the dispatch
/// that resumes the coroutine at the correct yield point on every
/// call. Must be matched by `SFEX_CO_END` at the end of the body.
///
/// \see `SFEX_CO_END`
///
////////////////////////////////////////////////////////////
#define SFEX_CO_BEGIN                                          \
    SFEX_PRIV_CO_CASE_WARNINGS_PUSH;                           \
    SFEX_PRIV_CO_C2Y_PUSH;                                     \
                                                               \
    static constexpr ::sf::base::U32 _sfex_base = __COUNTER__; \
                                                               \
    switch (state)                                             \
    {                                                          \
        case 0:;                                               \
            SFEX_PRIV_CO_CASE_WARNINGS_POP


////////////////////////////////////////////////////////////
/// \brief Suspend the coroutine and yield a value to the caller
///
/// Saves the resumption point, returns `value`, and on the next call
/// resumes execution immediately after the macro.
///
/// \param value The value to yield to the driver this tick.
///
/// \see `SFEX_CO_RETURN`, `SFEX_CO_AWAIT`
///
////////////////////////////////////////////////////////////

// Push must precede the `__COUNTER__` use (clang flags it as a C2y
// extension), so it spans the whole `do { ... }` block.
#define SFEX_CO_YIELD(value)                    \
    do                                          \
    {                                           \
        SFEX_PRIV_CO_CASE_WARNINGS_PUSH;        \
        SFEX_PRIV_CO_C2Y_PUSH;                  \
                                                \
        state = (__COUNTER__ + 1) - _sfex_base; \
        return value;                           \
                                                \
        case (__COUNTER__ - _sfex_base):;       \
            SFEX_PRIV_CO_CASE_WARNINGS_POP;     \
    } while (0)


////////////////////////////////////////////////////////////
/// \brief Finish the coroutine and yield a final value
///
/// Resets `state` to its initial position so the next call will start
/// over from `SFEX_CO_BEGIN`, and returns `value`. Typically used with
/// a "done" yield value (e.g. `Done{}`) so callers and parallel
/// drivers can detect completion.
///
/// \param value The final yield value.
///
/// \see `SFEX_CO_YIELD`
///
////////////////////////////////////////////////////////////
#define SFEX_CO_RETURN(value) \
    do                        \
    {                         \
        state = 0;            \
        return value;         \
    } while (0)


////////////////////////////////////////////////////////////
/// \brief Close a coroutine body
///
/// Place at the bottom of `Yield operator()(...)`, after any
/// `SFEX_CO_RETURN`. Matches `SFEX_CO_BEGIN`.
///
/// \see `SFEX_CO_BEGIN`
///
////////////////////////////////////////////////////////////
#define SFEX_CO_END \
    }               \
                    \
    SFML_BASE_UNREACHABLE();


////////////////////////////////////////////////////////////
/// \brief Run a sub-coroutine to completion
///
/// Every tick of the parent invokes `sub_call` once. While the
/// sub-coroutine is still running, its yields propagate up to the
/// driver; once it finishes, the parent falls through to the next
/// statement.
///
/// Requires (via ADL on the yield type) a free function
/// `bool isFinished(YieldType)` that returns `true` for the "this
/// coroutine has finished" yield kind.
///
/// \param sub_call An expression invoking the child coroutine, e.g.
///                 `child(world)` where `child` is a coroutine member.
///
/// \see `SFEX_CO_AWAIT_ALL`, `SFEX_CO_AWAIT_ANY`
///
////////////////////////////////////////////////////////////
// Push must precede the `__COUNTER__` use (clang flags it as a C2y
// extension), so it spans the whole `do { ... }` block.
#define SFEX_CO_AWAIT(sub_call)                 \
    do                                          \
    {                                           \
        SFEX_PRIV_CO_CASE_WARNINGS_PUSH;        \
        SFEX_PRIV_CO_C2Y_PUSH;                  \
                                                \
        state = (__COUNTER__ + 1) - _sfex_base; \
                                                \
        case (__COUNTER__ - _sfex_base):        \
        {                                       \
            auto _r = (sub_call);               \
                                                \
            if (!isFinished(_r))                \
                return _r;                      \
        }                                       \
                                                \
            SFEX_PRIV_CO_CASE_WARNINGS_POP;     \
    } while (0)


////////////////////////////////////////////////////////////
/// \brief Yield repeatedly until a condition becomes true
///
/// On every tick, evaluates `cond` and yields `yield_value` if it is
/// false; falls through once it becomes true.
///
/// \param yield_value The value to yield while waiting.
/// \param cond        The condition to wait for.
///
/// \see `SFEX_CO_WAIT_WHILE`
///
////////////////////////////////////////////////////////////
#define SFEX_CO_WAIT_UNTIL(yield_value, cond) \
    while (!(cond))                           \
    {                                         \
        SFEX_CO_YIELD(yield_value);           \
    }


////////////////////////////////////////////////////////////
/// \brief Yield repeatedly while a condition holds
///
/// Inverse of `SFEX_CO_WAIT_UNTIL`. Yields `yield_value` every tick
/// for as long as `cond` is true; falls through once it becomes false.
///
/// \param yield_value The value to yield while waiting.
/// \param cond        The condition to wait while it holds.
///
/// \see `SFEX_CO_WAIT_UNTIL`
///
////////////////////////////////////////////////////////////
#define SFEX_CO_WAIT_WHILE(yield_value, cond) \
    while (cond)                              \
    {                                         \
        SFEX_CO_YIELD(yield_value);           \
    }


////////////////////////////////////////////////////////////
/// \brief Run sub-coroutines in parallel, awaiting until *all* finish
///
/// Each tick advances every child once and yields `next_yield` while
/// at least one child is still running. Falls through once they are
/// all done.
///
/// All children must inherit `sfex::CoroutineParallelMixin` and must
/// have an ADL-discoverable `void yieldApply(YieldType, Child&)`.
///
/// Projects typically wrap this in a thin macro that fixes
/// `dt_expr` / `ctx_expr` / `next_yield` for their domain (e.g.
/// `BOSS_CO_AWAIT_ALL(...) -> SFEX_CO_AWAIT_ALL(ctx.world.dt, ctx,
/// NextFrame{}, __VA_ARGS__)`).
///
/// \param dt_expr    Elapsed time this frame.
/// \param ctx_expr   Context forwarded to every child's `operator()`.
/// \param next_yield Value to yield while waiting.
/// \param ...        Children to advance in parallel.
///
/// \see `SFEX_CO_AWAIT_ANY`, `sfex::tickInParallel`
///
////////////////////////////////////////////////////////////
#define SFEX_CO_AWAIT_ALL(dt_expr, ctx_expr, next_yield, ...)                \
    while (!::sfex::priv::tickAllInParallel(dt_expr, ctx_expr, __VA_ARGS__)) \
    {                                                                        \
        SFEX_CO_YIELD(next_yield);                                           \
    }


////////////////////////////////////////////////////////////
/// \brief Run sub-coroutines in parallel, awaiting until *any* finishes
///
/// Same as `SFEX_CO_AWAIT_ALL` but exits as soon as the first child
/// finishes. The other children are simply not advanced any further;
/// their state is left as-is on the parent struct, so the parent can
/// inspect them afterwards (e.g. to find out which one won the race).
///
/// \param dt_expr    Elapsed time this frame.
/// \param ctx_expr   Context forwarded to every child's `operator()`.
/// \param next_yield Value to yield while waiting.
/// \param ...        Children to advance in parallel.
///
/// \see `SFEX_CO_AWAIT_ALL`, `sfex::tickInParallel`
///
////////////////////////////////////////////////////////////
#define SFEX_CO_AWAIT_ANY(dt_expr, ctx_expr, next_yield, ...)                \
    while (!::sfex::priv::tickAnyInParallel(dt_expr, ctx_expr, __VA_ARGS__)) \
    {                                                                        \
        SFEX_CO_YIELD(next_yield);                                           \
    }
