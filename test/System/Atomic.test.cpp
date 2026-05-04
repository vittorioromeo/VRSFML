#include "SFML/System/Atomic.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/StdThread.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


namespace
{
} // namespace


////////////////////////////////////////////////////////////
// Compile-time properties
////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::Atomic<bool>));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::Atomic<char>));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::Atomic<int>));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::Atomic<sf::base::U64>));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::Atomic<float>));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::Atomic<double>));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::Atomic<int*>));


////////////////////////////////////////////////////////////
static_assert(sizeof(sf::Atomic<bool>) == 1u);
static_assert(sizeof(sf::Atomic<char>) == 1u);
static_assert(sizeof(sf::Atomic<sf::base::I8>) == 1u);
static_assert(sizeof(sf::Atomic<sf::base::U8>) == 1u);
static_assert(sizeof(sf::Atomic<sf::base::I16>) == 2u);
static_assert(sizeof(sf::Atomic<sf::base::U16>) == 2u);
static_assert(sizeof(sf::Atomic<sf::base::I32>) == 4u);
static_assert(sizeof(sf::Atomic<sf::base::U32>) == 4u);
static_assert(sizeof(sf::Atomic<sf::base::I64>) == 8u);
static_assert(sizeof(sf::Atomic<sf::base::U64>) == 8u);
static_assert(sizeof(sf::Atomic<float>) == 4u);
static_assert(sizeof(sf::Atomic<double>) == 8u);
static_assert(sizeof(sf::Atomic<int*>) == sizeof(int*));


////////////////////////////////////////////////////////////
static_assert(alignof(sf::Atomic<sf::base::I64>) >= 8u);
static_assert(alignof(sf::Atomic<sf::base::U64>) >= 8u);
static_assert(alignof(sf::Atomic<double>) >= 8u);


////////////////////////////////////////////////////////////
static_assert(!__is_constructible(sf::Atomic<int>, const sf::Atomic<int>&));
static_assert(!__is_constructible(sf::Atomic<int>, sf::Atomic<int>&&));


////////////////////////////////////////////////////////////
// Floating-point and pointer atomics must NOT expose bitwise ops.
// Integral atomics MUST. (Detected via SFINAE on the requires clause.)
////////////////////////////////////////////////////////////
template <typename A, typename T>
concept HasFetchAnd = requires(A& a, T v) { a.template fetchAnd<sf::MemoryOrder::Relaxed>(v); };

static_assert(HasFetchAnd<sf::Atomic<int>, int>);
static_assert(HasFetchAnd<sf::Atomic<sf::base::U64>, sf::base::U64>);
static_assert(!HasFetchAnd<sf::Atomic<float>, float>);
static_assert(!HasFetchAnd<sf::Atomic<double>, double>);
static_assert(!HasFetchAnd<sf::Atomic<int*>, int*>);


////////////////////////////////////////////////////////////
template <typename A, typename T>
concept HasFetchAddT = requires(A& a, T v) { a.template fetchAdd<sf::MemoryOrder::Relaxed>(v); };

static_assert(HasFetchAddT<sf::Atomic<int>, int>);
static_assert(HasFetchAddT<sf::Atomic<int*>, sf::base::PtrDiffT>); // pointer takes ptrdiff_t
static_assert(!HasFetchAddT<sf::Atomic<float>, float>);            // floats have no fetchAdd
static_assert(!HasFetchAddT<sf::Atomic<double>, double>);


////////////////////////////////////////////////////////////
// `sf::Atomic<bool>` must NOT expose any arithmetic / bitwise fetch op,
// matching `std`. Bool arithmetic is nonsensical and
// the GCC/Clang `__atomic_fetch_*` builtins are not specified for it.
////////////////////////////////////////////////////////////
static_assert(!HasFetchAddT<sf::Atomic<bool>, bool>);
static_assert(!HasFetchAnd<sf::Atomic<bool>, bool>);


////////////////////////////////////////////////////////////
// Force-instantiate the wait() body for every supported size so
// pointer→integer conversion regressions are caught at compile time.
// These functions are never called at runtime.
////////////////////////////////////////////////////////////
namespace
{
[[maybe_unused]] void instantiateWaitBodies()
{
    int dummy = 0;

    sf::Atomic<sf::base::U32> a32{0u};
    a32.wait<sf::MemoryOrder::Acquire>(0u);

    sf::Atomic<sf::base::U64> a64{0u};
    a64.wait<sf::MemoryOrder::Acquire>(0u);

    sf::Atomic<float> af{0.0f};
    af.wait<sf::MemoryOrder::Acquire>(0.0f);

    sf::Atomic<double> ad{0.0};
    ad.wait<sf::MemoryOrder::Acquire>(0.0);

    sf::Atomic<int*> ap{&dummy};
    ap.wait<sf::MemoryOrder::Acquire>(&dummy); // pointer wait must compile
}
} // namespace


////////////////////////////////////////////////////////////
static_assert(static_cast<int>(sf::MemoryOrder::Relaxed) == __ATOMIC_RELAXED);
static_assert(static_cast<int>(sf::MemoryOrder::Acquire) == __ATOMIC_ACQUIRE);
static_assert(static_cast<int>(sf::MemoryOrder::Release) == __ATOMIC_RELEASE);
static_assert(static_cast<int>(sf::MemoryOrder::AcqRel) == __ATOMIC_ACQ_REL);
static_assert(static_cast<int>(sf::MemoryOrder::SeqCst) == __ATOMIC_SEQ_CST);


////////////////////////////////////////////////////////////
// Single-threaded round-trip helpers
////////////////////////////////////////////////////////////
namespace
{
template <typename T>
void singleThreadedRoundTrip(const T initial, const T other)
{
    sf::Atomic<T> a{initial};
    CHECK(a.template load<sf::MemoryOrder::Relaxed>() == initial);
    CHECK(a.template load<sf::MemoryOrder::Acquire>() == initial);
    CHECK(a.template load<sf::MemoryOrder::SeqCst>() == initial);

    a.template store<sf::MemoryOrder::Relaxed>(other);
    CHECK(a.template load<sf::MemoryOrder::Relaxed>() == other);

    a.template store<sf::MemoryOrder::Release>(initial);
    CHECK(a.template load<sf::MemoryOrder::Acquire>() == initial);

    a.template store<sf::MemoryOrder::SeqCst>(other);
    CHECK(a.template load<sf::MemoryOrder::SeqCst>() == other);
}


////////////////////////////////////////////////////////////
template <typename T>
void exchangeRoundTrip(const T initial, const T other)
{
    sf::Atomic<T> a{initial};

    const T prev = a.template exchange<sf::MemoryOrder::SeqCst>(other);
    CHECK(prev == initial);
    CHECK(a.template load<sf::MemoryOrder::Relaxed>() == other);
}


////////////////////////////////////////////////////////////
template <typename T>
void casRoundTrip(const T initial, const T other, const T third)
{
    // compareExchangeStrong success
    {
        sf::Atomic<T> a{initial};
        T             expected = initial;

        CHECK(a.template compareExchangeStrong<sf::MemoryOrder::SeqCst, sf::MemoryOrder::Relaxed>(expected, other));
        CHECK(expected == initial); // unchanged on success
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == other);
    }

    // compareExchangeStrong failure
    {
        sf::Atomic<T> a{initial};
        T             expected = other;

        CHECK(!a.template compareExchangeStrong<sf::MemoryOrder::SeqCst, sf::MemoryOrder::Acquire>(expected, third));
        CHECK(expected == initial); // updated to actual on failure
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == initial);
    }

    // compareExchangeWeak in a retry loop (handles spurious failure)
    {
        sf::Atomic<T> a{initial};
        T             expected = initial;

        while (!a.template compareExchangeWeak<sf::MemoryOrder::SeqCst, sf::MemoryOrder::Relaxed>(expected, other))
        {
            // keep retrying with refreshed `expected`
        }

        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == other);
    }

    // compareExchangeWeak hard failure (single shot, expected mismatches)
    {
        sf::Atomic<T> a{initial};
        T             expected = other;
        const bool ok = a.template compareExchangeWeak<sf::MemoryOrder::SeqCst, sf::MemoryOrder::Acquire>(expected, third);

        // weak CAS may spuriously fail; we only require: if it failed, `expected` reflects reality
        if (!ok)
            CHECK(expected == initial);
    }
}


////////////////////////////////////////////////////////////
template <typename T>
void integralFetchOps(const T zero, const T one, const T mask)
{
    {
        sf::Atomic<T> a{zero};
        const T       prev = a.template fetchAdd<sf::MemoryOrder::SeqCst>(one);
        CHECK(prev == zero);
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == static_cast<T>(zero + one));
    }

    {
        sf::Atomic<T> a{static_cast<T>(zero + one)};
        const T       prev = a.template fetchSub<sf::MemoryOrder::SeqCst>(one);
        CHECK(prev == static_cast<T>(zero + one));
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == zero);
    }

    {
        sf::Atomic<T> a{mask};
        const T       prev = a.template fetchAnd<sf::MemoryOrder::SeqCst>(static_cast<T>(zero));
        CHECK(prev == mask);
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == zero);
    }

    {
        sf::Atomic<T> a{zero};
        const T       prev = a.template fetchOr<sf::MemoryOrder::SeqCst>(mask);
        CHECK(prev == zero);
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == mask);
    }

    {
        sf::Atomic<T> a{mask};
        const T       prev = a.template fetchXor<sf::MemoryOrder::SeqCst>(mask);
        CHECK(prev == mask);
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == zero);
    }
}
} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - bool")
{
    singleThreadedRoundTrip<bool>(false, true);
    exchangeRoundTrip<bool>(false, true);
    casRoundTrip<bool>(false, true, false);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - char")
{
    singleThreadedRoundTrip<char>('a', 'z');
    exchangeRoundTrip<char>('a', 'z');
    casRoundTrip<char>('a', 'z', 'm');
    integralFetchOps<char>(static_cast<char>(0x00), static_cast<char>(0x01), static_cast<char>(0x55));
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - signed/unsigned 8-bit")
{
    singleThreadedRoundTrip<sf::base::I8>(-3, 7);
    exchangeRoundTrip<sf::base::U8>(0u, 0xFFu);
    casRoundTrip<sf::base::I8>(0, 1, 2);
    integralFetchOps<sf::base::U8>(0u, 1u, 0x55u);
    integralFetchOps<sf::base::I8>(0, 1, 0x55);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - 16-bit")
{
    singleThreadedRoundTrip<sf::base::I16>(-1234, 5678);
    exchangeRoundTrip<sf::base::U16>(0u, 0xFF'FFu);
    casRoundTrip<sf::base::U16>(0u, 1u, 2u);
    integralFetchOps<sf::base::U16>(0u, 1u, 0x55'55u);
    integralFetchOps<sf::base::I16>(0, 1, 0x55'55);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - 32-bit")
{
    singleThreadedRoundTrip<sf::base::I32>(-100'000, 200'000);
    exchangeRoundTrip<sf::base::U32>(0u, 0xDE'AD'BE'EFu);
    casRoundTrip<sf::base::U32>(1u, 2u, 3u);
    integralFetchOps<sf::base::U32>(0u, 1u, 0x55'55'55'55u);
    integralFetchOps<sf::base::I32>(0, 1, 0x55'55'55'55);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - 64-bit")
{
    singleThreadedRoundTrip<sf::base::I64>(-(static_cast<sf::base::I64>(1) << 40), static_cast<sf::base::I64>(1) << 50);
    exchangeRoundTrip<sf::base::U64>(0u, 0xDE'AD'BE'EF'CA'FE'BA'BEu);
    casRoundTrip<sf::base::U64>(1u, 2u, 3u);
    integralFetchOps<sf::base::U64>(0u, 1u, 0x55'55'55'55'55'55'55'55u);
    integralFetchOps<sf::base::I64>(0, 1, 0x55'55'55'55'55'55'55'55);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - float")
{
    singleThreadedRoundTrip<float>(1.5f, -2.5f);
    exchangeRoundTrip<float>(0.0f, 3.14f);
    casRoundTrip<float>(1.0f, 2.0f, 3.0f);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - double")
{
    singleThreadedRoundTrip<double>(1.5, -2.5);
    exchangeRoundTrip<double>(0.0, 3.14159);
    casRoundTrip<double>(1.0, 2.0, 3.0);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - pointer")
{
    int storage[8]{};

    singleThreadedRoundTrip<int*>(&storage[0], &storage[5]);
    exchangeRoundTrip<int*>(&storage[0], &storage[3]);
    casRoundTrip<int*>(&storage[0], &storage[1], &storage[2]);

    SECTION("Pointer fetchAdd / fetchSub scale by element size")
    {
        sf::Atomic<int*> a{&storage[0]};

        int* const after = a.template fetchAdd<sf::MemoryOrder::SeqCst>(3);
        CHECK(after == &storage[0]);
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == &storage[3]);

        int* const after2 = a.template fetchSub<sf::MemoryOrder::SeqCst>(2);
        CHECK(after2 == &storage[3]);
        CHECK(a.template load<sf::MemoryOrder::Relaxed>() == &storage[1]);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - default-constructed and value-constructed")
{
    sf::Atomic<int> defaulted{0};
    CHECK(defaulted.load<sf::MemoryOrder::Relaxed>() == 0);
    defaulted.store<sf::MemoryOrder::Relaxed>(7);
    CHECK(defaulted.load<sf::MemoryOrder::Relaxed>() == 7);

    constexpr sf::Atomic<int> valueCtor{42};
    CHECK(valueCtor.load<sf::MemoryOrder::Relaxed>() == 42);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - thread fences (smoke)")
{
    sf::Atomic<int> a{0};
    sf::Atomic<int> b{0};

    sf::atomicThreadFence<sf::MemoryOrder::SeqCst>();
    a.store<sf::MemoryOrder::Relaxed>(1);
    sf::atomicThreadFence<sf::MemoryOrder::Release>();
    b.store<sf::MemoryOrder::Relaxed>(2);
    sf::atomicThreadFence<sf::MemoryOrder::Acquire>();

    CHECK(a.load<sf::MemoryOrder::Relaxed>() == 1);
    CHECK(b.load<sf::MemoryOrder::Relaxed>() == 2);

    sf::atomicSignalFence<sf::MemoryOrder::SeqCst>();
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - concurrent fetchAdd is consistent")
{
    constexpr int             threadCount         = 8;
    constexpr int             incrementsPerThread = 50'000;
    sf::Atomic<sf::base::I64> counter{0};

    sf::base::Vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int t = 0; t < threadCount; ++t)
        threads.emplaceBack([&counter]
        {
            for (int i = 0; i < incrementsPerThread; ++i)
                counter.fetchAdd<sf::MemoryOrder::Relaxed>(1);
        });

    for (auto& th : threads)
        th.join();

    CHECK(counter.load<sf::MemoryOrder::SeqCst>() ==
          static_cast<sf::base::I64>(threadCount) * static_cast<sf::base::I64>(incrementsPerThread));
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - producer/consumer release-acquire handoff")
{
    sf::Atomic<int> data{0};
    sf::Atomic<int> ready{0};

    std::thread producer([&]
    {
        data.store<sf::MemoryOrder::Relaxed>(42);
        ready.store<sf::MemoryOrder::Release>(1);
    });

    std::thread consumer([&]
    {
        while (ready.load<sf::MemoryOrder::Acquire>() == 0)
            std::this_thread::yield();

        CHECK(data.load<sf::MemoryOrder::Relaxed>() == 42);
    });

    producer.join();
    consumer.join();
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - CAS-loop float increment (no fetchAdd for floats)")
{
    sf::Atomic<float> total{0.0f};

    constexpr int   threadCount         = 4;
    constexpr int   incrementsPerThread = 25'000;
    constexpr float perStep             = 0.25f;

    sf::base::Vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int t = 0; t < threadCount; ++t)
        threads.emplaceBack([&]
        {
            for (int i = 0; i < incrementsPerThread; ++i)
            {
                float current = total.load<sf::MemoryOrder::Relaxed>();

                while (!total.compareExchangeWeak<sf::MemoryOrder::Release, sf::MemoryOrder::Relaxed>(current, current + perStep))
                {
                    // keep retrying with refreshed `current`
                }
            }
        });

    for (auto& th : threads)
        th.join();

    const double expected = static_cast<double>(threadCount) * static_cast<double>(incrementsPerThread) *
                            static_cast<double>(perStep);
    CHECK(static_cast<double>(total.load<sf::MemoryOrder::SeqCst>()) == doctest::Approx(expected));
}


////////////////////////////////////////////////////////////
// wait / waitUntil are only available for sizeof(T) in {4, 8}
////////////////////////////////////////////////////////////
template <typename A, typename T>
concept HasWait = requires(A& a, T v) { a.template wait<sf::MemoryOrder::Acquire>(v); };

static_assert(HasWait<sf::Atomic<sf::base::U32>, sf::base::U32>);
static_assert(HasWait<sf::Atomic<sf::base::U64>, sf::base::U64>);
static_assert(HasWait<sf::Atomic<float>, float>);
static_assert(HasWait<sf::Atomic<double>, double>);
static_assert(HasWait<sf::Atomic<int*>, int*>);
static_assert(!HasWait<sf::Atomic<bool>, bool>);
static_assert(!HasWait<sf::Atomic<char>, char>);
static_assert(!HasWait<sf::Atomic<sf::base::U16>, sf::base::U16>);


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - waitUntil / notifyOne (latch pattern)")
{
    constexpr int             workerCount = 6;
    sf::Atomic<sf::base::U32> nRemaining{static_cast<sf::base::U32>(workerCount)};

    sf::base::Vector<std::thread> workers;
    workers.reserve(workerCount);

    for (int i = 0; i < workerCount; ++i)
        workers.emplaceBack([&]
        {
            // simulate a tiny chunk of work
            std::this_thread::sleep_for(std::chrono::microseconds(100));

            // last worker to finish notifies the latch (mirrors std::latch semantics)
            if (nRemaining.fetchSub<sf::MemoryOrder::Release>(1) == 1u)
                nRemaining.notifyOne();
        });

    nRemaining.waitUntil<sf::MemoryOrder::Acquire>([](const sf::base::U32 v) { return v == 0u; });

    CHECK(nRemaining.load<sf::MemoryOrder::Relaxed>() == 0u);

    for (auto& th : workers)
        th.join();
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - notifyAll wakes multiple waiters")
{
    sf::Atomic<sf::base::U32>     gate{0u};
    sf::Atomic<sf::base::U32>     woken{0u};
    constexpr int                 waiterCount = 4;
    sf::base::Vector<std::thread> waiters;
    waiters.reserve(waiterCount);

    for (int i = 0; i < waiterCount; ++i)
        waiters.emplaceBack([&]
        {
            gate.waitUntil<sf::MemoryOrder::Acquire>([](const sf::base::U32 v) { return v != 0u; });
            woken.fetchAdd<sf::MemoryOrder::Relaxed>(1u);
        });

    // Give the waiters a moment to actually start blocking
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    gate.store<sf::MemoryOrder::Release>(1u);
    gate.notifyAll();

    for (auto& th : waiters)
        th.join();

    CHECK(woken.load<sf::MemoryOrder::Relaxed>() == static_cast<sf::base::U32>(waiterCount));
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - waitUntil predicate already true returns immediately")
{
    sf::Atomic<sf::base::U32> a{42u};

    a.waitUntil<sf::MemoryOrder::Acquire>([](const sf::base::U32 v) { return v == 42u; });

    CHECK(a.load<sf::MemoryOrder::Relaxed>() == 42u);
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] SFML/System/Atomic.hpp - 64-bit waitUntil")
{
    sf::Atomic<sf::base::U64> counter{3u};

    std::thread decrementer([&]
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        counter.store<sf::MemoryOrder::Release>(0u);
        counter.notifyAll();
    });

    counter.waitUntil<sf::MemoryOrder::Acquire>([](const sf::base::U64 v) { return v == 0u; });
    CHECK(counter.load<sf::MemoryOrder::Relaxed>() == 0u);

    decrementer.join();
}
