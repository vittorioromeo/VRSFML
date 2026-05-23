#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Atomic.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Non-recursive mutex built on `sf::Atomic` wait/notify.
///
/// Single 32-bit atomic state: `0` = unlocked, `1` = locked. The fast
/// path is one `compareExchange` with no syscall; the slow path uses
/// `wait` (futex / WaitOnAddress / `wasm.memory.atomic.wait`) -- the
/// same primitives the standard library uses for `std::mutex`. No
/// recursion, no priority inheritance, no fairness guarantees --
/// LIFO-ish under contention.
///
/// Use `sf::LockGuard` (in `SFML/System/LockGuard.hpp`) for RAII locking.
///
////////////////////////////////////////////////////////////
class AtomicMutex
{
public:
    ////////////////////////////////////////////////////////////
    AtomicMutex() = default;

    AtomicMutex(const AtomicMutex&)            = delete;
    AtomicMutex& operator=(const AtomicMutex&) = delete;

    AtomicMutex(AtomicMutex&&)            = delete;
    AtomicMutex& operator=(AtomicMutex&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Acquire the mutex, blocking if necessary
    ///
    ////////////////////////////////////////////////////////////
    void lock() noexcept
    {
        while (true)
        {
            unsigned int expected = 0u;

            // Fast path: the mutex is free -> take it.
            if (m_state.compareExchangeStrong<MemoryOrder::Acquire, MemoryOrder::Relaxed>(expected, 1u))
                return;

            // Slow path: park until someone unlocks. `wait` checks `m_state == 1`
            // atomically before sleeping, so we can't miss a wakeup -- if the
            // unlock raced past our `wait`, `m_state` is already `0` and `wait`
            // returns immediately. Spurious wakeups are absorbed by the loop.
            m_state.waitAcquire(1u);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Try to acquire the mutex without blocking
    ///
    /// \return `true` if the mutex was acquired, `false` if it was held by another thread
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool tryLock() noexcept
    {
        unsigned int expected = 0u;
        return m_state.compareExchangeStrong<MemoryOrder::Acquire, MemoryOrder::Relaxed>(expected, 1u);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Release the mutex and wake one waiting thread (if any)
    ///
    ////////////////////////////////////////////////////////////
    void unlock() noexcept
    {
        m_state.storeRelease(0u);
        m_state.notifyOne();
    }

private:
    ////////////////////////////////////////////////////////////
    // 4-byte atomic so it's compatible with `wait` (which requires 4 or 8 bytes
    // -- futex / WaitOnAddress operate on 32-bit words). 0 = unlocked, 1 = locked.
    Atomic<unsigned int> m_state{0u};
};

} // namespace sf
