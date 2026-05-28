#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/AtomicMutex.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief RAII lock guard for `sf::AtomicMutex` (substitute for `std::lock_guard`)
///
////////////////////////////////////////////////////////////
class [[nodiscard]] LockGuard
{
public:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] explicit LockGuard(AtomicMutex& mutex) noexcept : m_mutex(mutex)
    {
        m_mutex.lock();
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~LockGuard() noexcept
    {
        m_mutex.unlock();
    }

    LockGuard(const LockGuard&)            = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    LockGuard(LockGuard&&)            = delete;
    LockGuard& operator=(LockGuard&&) = delete;

private:
    AtomicMutex& m_mutex;
};

} // namespace sf
