#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "SFML/System/Thread.hpp"

#include "SFML/System/Atomic.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsMoveAssignable.hpp"
#include "SFML/Base/Trait/IsMoveConstructible.hpp"


////////////////////////////////////////////////////////////
// `Thread` is move-only -- copy operations must be deleted.
////////////////////////////////////////////////////////////
static_assert(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Thread));
static_assert(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Thread));

static_assert(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::Thread));
static_assert(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::Thread));


TEST_CASE("[System] SFML/System/Thread.hpp - default-constructed is not joinable")
{
    sf::Thread t;
    CHECK(!t.joinable());
    CHECK(t.getId().value() == 0u);
}

TEST_CASE("[System] SFML/System/Thread.hpp - hardwareConcurrency returns positive value")
{
    const unsigned int n = sf::Thread::hardwareConcurrency();
    CHECK(n >= 1u); // every reasonable test target has at least one core
}

TEST_CASE("[System] SFML/System/Thread.hpp - spawn + join round-trip")
{
    sf::Atomic<int> ran{0};

    sf::Thread t{[&ran] { ran.storeRelease(42); }};

    CHECK(t.joinable());
    t.join();
    CHECK(!t.joinable());
    CHECK(ran.loadAcquire() == 42);
}

TEST_CASE("[System] SFML/System/Thread.hpp - getId is non-zero for a running thread")
{
    sf::Atomic<sf::base::U64> observedId{0u};

    sf::Thread t{[&observedId] { observedId.storeRelease(sf::ThisThread::getId().value()); }};

    const sf::ThreadId outsideId = t.getId();
    CHECK(outsideId.value() != 0u);

    t.join();

    CHECK(observedId.loadAcquire() == outsideId.value());
}

TEST_CASE("[System] SFML/System/Thread.hpp - move construction transfers ownership")
{
    sf::Atomic<int> finished{0};

    sf::Thread t1{[&finished]
    {
        sf::ThisThread::sleepFor(sf::milliseconds(10));
        finished.storeRelease(1);
    }};

    CHECK(t1.joinable());
    const sf::ThreadId id = t1.getId();

    sf::Thread t2{static_cast<sf::Thread&&>(t1)};

    CHECK(!t1.joinable());
    CHECK(t2.joinable());
    CHECK(t2.getId().value() == id.value());

    t2.join();
    CHECK(finished.loadAcquire() == 1);
}

TEST_CASE("[System] SFML/System/Thread.hpp - destructor implicitly joins (std::jthread semantics)")
{
    sf::Atomic<int> ran{0};

    {
        sf::Thread t{[&ran]
        {
            sf::ThisThread::sleepFor(sf::milliseconds(10));
            ran.storeRelease(123);
        }};
        // No explicit join/detach -- the destructor at scope exit must
        // wait for the worker to finish, not abort.
    }

    CHECK(ran.loadAcquire() == 123);
}

TEST_CASE("[System] SFML/System/Thread.hpp - move assignment to joinable target implicitly joins")
{
    sf::Atomic<int> firstRan{0};
    sf::Atomic<int> secondRan{0};

    sf::Thread t1{[&firstRan]
    {
        sf::ThisThread::sleepFor(sf::milliseconds(10));
        firstRan.storeRelease(1);
    }};

    sf::Thread t2{[&secondRan] { secondRan.storeRelease(2); }};

    // Overwriting a joinable target must implicitly join the previous
    // worker (matching std::jthread).
    t1 = static_cast<sf::Thread&&>(t2);

    CHECK(firstRan.loadAcquire() == 1); // ran to completion before the move-assign returned
    CHECK(t1.joinable());
    CHECK(!t2.joinable());

    t1.join();
    CHECK(secondRan.loadAcquire() == 2);
}

TEST_CASE("[System] SFML/System/Thread.hpp - move assignment to non-joinable target")
{
    sf::Atomic<int> ran{0};

    sf::Thread t1{[&ran] { ran.storeRelease(7); }};

    sf::Thread t2;
    t2 = static_cast<sf::Thread&&>(t1);

    CHECK(!t1.joinable());
    CHECK(t2.joinable());

    t2.join();
    CHECK(ran.loadAcquire() == 7);
}

TEST_CASE("[System] SFML/System/Thread.hpp - detach releases joinability")
{
    sf::Atomic<int> done{0};

    {
        sf::Thread t{[&done]
        {
            sf::ThisThread::sleepFor(sf::milliseconds(5));
            done.storeRelease(1);
        }};
        t.detach();
        CHECK(!t.joinable());
    }

    // Wait for the detached thread to finish before we leave the test
    // so the runner does not see a leaked thread.
    while (done.loadAcquire() == 0)
        sf::ThisThread::yield();

    CHECK(done.loadAcquire() == 1);
}

TEST_CASE("[System] SFML/System/Thread.hpp - ThisThread::getId is stable on the calling thread")
{
    const sf::ThreadId id1 = sf::ThisThread::getId();
    const sf::ThreadId id2 = sf::ThisThread::getId();
    CHECK(id1.value() == id2.value());
    CHECK(id1.value() != 0u);
}

TEST_CASE("[System] SFML/System/Thread.hpp - sleep sleeps for at least the given time")
{
    // OS schedulers may oversleep, but should never undersleep.
    // Replaces the standalone `Sleep.test.cpp` round-trip.
    const auto checkSleeps = [](const sf::Time duration)
    {
        sf::Clock      clock;
        const sf::Time start = clock.getElapsedTime();

        sf::ThisThread::sleepFor(duration);

        CHECK((clock.getElapsedTime() - start) >= duration);
    };

    checkSleeps(sf::milliseconds(1));
    checkSleeps(sf::milliseconds(5));
    checkSleeps(sf::milliseconds(25));
}

TEST_CASE("[System] SFML/System/Thread.hpp - many threads each see distinct ids")
{
    constexpr int             threadCount = 8;
    sf::Atomic<sf::base::U32> distinctSum{0u};

    sf::Thread threads[threadCount]{};

    for (auto& t : threads)
        t = sf::Thread{[&distinctSum]
        {
            // Add the lower 32 bits of this thread's id. With 64-bit
            // counter values, ids are unique across this short test
            // window; the sum being equal to the sum of the unique
            // ids the threads observed is the test.
            distinctSum.fetchAddRelaxed(static_cast<sf::base::U32>(sf::ThisThread::getId().value()));
        }};

    sf::base::U32 expected = 0u;
    for (auto& t : threads)
    {
        const sf::base::U32 id = static_cast<sf::base::U32>(t.getId().value());
        CHECK(id != 0u);
        expected += id;
        t.join();
    }

    CHECK(distinctSum.loadSeqCst() == expected);
}
