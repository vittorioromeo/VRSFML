#include "SFML/System/AtomicMutex.hpp"

#include "SFML/System/LockGuard.hpp"
#include "SFML/System/Thread.hpp"

#include "SFML/Base/Trait/IsAssignable.hpp"
#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


namespace
{
} // namespace


////////////////////////////////////////////////////////////
// Compile-time properties
////////////////////////////////////////////////////////////
static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::AtomicMutex, const sf::AtomicMutex&));
static_assert(!SFML_BASE_IS_ASSIGNABLE(sf::AtomicMutex, const sf::AtomicMutex&));
static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::AtomicMutex, sf::AtomicMutex&&));

static_assert(sizeof(sf::AtomicMutex) == sizeof(unsigned int));

static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::LockGuard, const sf::LockGuard&));
static_assert(!SFML_BASE_IS_ASSIGNABLE(sf::LockGuard, const sf::LockGuard&));


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::AtomicMutex - basic lock/unlock")
{
    sf::AtomicMutex m;

    // Single-threaded sanity: lock then unlock leaves the mutex acquirable.
    m.lock();
    m.unlock();

    CHECK(m.tryLock());
    m.unlock();
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::AtomicMutex - tryLock when held returns false")
{
    sf::AtomicMutex m;

    m.lock();
    CHECK_FALSE(m.tryLock()); // already held by *this thread*; non-recursive so tryLock must refuse
    m.unlock();

    // Re-acquirable after unlock.
    CHECK(m.tryLock());
    m.unlock();
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::LockGuard - acquires on ctor, releases on dtor")
{
    sf::AtomicMutex m;

    {
        const sf::LockGuard guard{m};
        CHECK_FALSE(m.tryLock()); // held by the guard
    }

    // After the guard's destruction, the mutex is free again.
    CHECK(m.tryLock());
    m.unlock();
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::LockGuard - exception safety (early return inside locked scope)")
{
    sf::AtomicMutex m;

    const auto scopedLock = [&]
    {
        const sf::LockGuard guard{m};
        // Simulate an early-return path inside the locked region.
        return 42;
    };

    CHECK(scopedLock() == 42);

    // Mutex must be released after the lambda returns through guard's dtor.
    CHECK(m.tryLock());
    m.unlock();
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::AtomicMutex - concurrent counter (mutual exclusion stress)")
{
    constexpr int       threadCount      = 8;
    constexpr int       incrementsPerThd = 100'000;
    constexpr long long expectedTotal    = static_cast<long long>(threadCount) * incrementsPerThd;

    sf::AtomicMutex m;
    long long       counter = 0;

    sf::base::Vector<sf::Thread> threads;
    threads.reserve(threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplaceBack([&]
        {
            for (int j = 0; j < incrementsPerThd; ++j)
            {
                const sf::LockGuard guard{m};
                ++counter;
            }
        });
    }

    for (auto& t : threads)
        t.join();

    // If mutual exclusion is broken, the counter would lose increments and be < expectedTotal.
    CHECK(counter == expectedTotal);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::AtomicMutex - producers wake on unlock (wait/notify path)")
{
    // This test exercises the slow path: the main thread holds the lock for
    // long enough that worker threads have to call `wait` rather than win
    // their first CAS. The final counter must still reach `threadCount`.

    constexpr int   threadCount = 16;
    sf::AtomicMutex m;
    long long       counter = 0;

    m.lock(); // hold the lock so workers must park

    sf::base::Vector<sf::Thread> threads;
    threads.reserve(threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplaceBack([&]
        {
            const sf::LockGuard guard{m};
            ++counter;
        });
    }

    // Brief stall to make sure workers reach `wait` -- not strictly required
    // for correctness (`wait` checks the state atomically before sleeping),
    // but exercises the parked branch deliberately.
    for (int spin = 0; spin < 1'000'000; ++spin)
    {
        asm volatile("" ::: "memory");
    }

    m.unlock(); // wake one; chain of notifyOne unwinds the rest as each unlocks

    for (auto& t : threads)
        t.join();

    CHECK(counter == threadCount);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::AtomicMutex - ping-pong between two threads")
{
    // Strict alternation pattern: each thread can only increment its own
    // counter when a shared "turn" matches. Stresses the lock/unlock/wait
    // cycle in tight succession.

    sf::AtomicMutex m;
    int             turn   = 0; // 0 = A's turn, 1 = B's turn
    int             countA = 0;
    int             countB = 0;

    constexpr int rounds = 10'000;

    sf::Thread tA([&]
    {
        for (int i = 0; i < rounds; ++i)
        {
            while (true)
            {
                const sf::LockGuard guard{m};
                if (turn == 0)
                {
                    ++countA;
                    turn = 1;
                    break;
                }
            }
        }
    });

    sf::Thread tB([&]
    {
        for (int i = 0; i < rounds; ++i)
        {
            while (true)
            {
                const sf::LockGuard guard{m};
                if (turn == 1)
                {
                    ++countB;
                    turn = 0;
                    break;
                }
            }
        }
    });

    tA.join();
    tB.join();

    CHECK(countA == rounds);
    CHECK(countB == rounds);
}
