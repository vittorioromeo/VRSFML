#include "Tst/Tst.hpp"

#include "SFML/System/Atomic.hpp"

#include "SFML/System/Thread.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"
#include "SFML/Base/Vector.hpp"


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
static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Atomic<int>, const sf::Atomic<int>&));
static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Atomic<int>, sf::Atomic<int>&&));


////////////////////////////////////////////////////////////
// Floating-point and pointer atomics must NOT expose bitwise ops.
// Integral atomics MUST. (Detected via SFINAE on the requires clause.)
////////////////////////////////////////////////////////////
template <typename A, typename T>
concept HasFetchAnd = requires(A& a, T v) { a.fetchAndRelaxed(v); };

static_assert(HasFetchAnd<sf::Atomic<int>, int>);
static_assert(HasFetchAnd<sf::Atomic<sf::base::U64>, sf::base::U64>);
static_assert(!HasFetchAnd<sf::Atomic<float>, float>);
static_assert(!HasFetchAnd<sf::Atomic<double>, double>);
static_assert(!HasFetchAnd<sf::Atomic<int*>, int*>);


////////////////////////////////////////////////////////////
template <typename A, typename T>
concept HasFetchAddT = requires(A& a, T v) { a.fetchAddRelaxed(v); };

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
    a32.waitOnceAcquire(0u);

    sf::Atomic<sf::base::U64> a64{0u};
    a64.waitOnceAcquire(0u);

    sf::Atomic<float> af{0.0f};
    af.waitOnceAcquire(0.0f);

    sf::Atomic<double> ad{0.0};
    ad.waitOnceAcquire(0.0);

    sf::Atomic<int*> ap{&dummy};
    ap.waitOnceAcquire(&dummy); // pointer wait must compile
}


////////////////////////////////////////////////////////////
// Force-instantiate every per-order alias so the macro-generated
// bodies are actually compiled. None of these calls execute at
// runtime; this function exists purely to lock in compile-time
// coverage of the alias surface.
////////////////////////////////////////////////////////////
[[maybe_unused]] void instantiateMemoryOrderAliases()
{
    int dummy = 0;

    // load: 3 orders
    sf::Atomic<int> ai{0};
    (void)ai.loadRelaxed();
    (void)ai.loadAcquire();
    (void)ai.loadSeqCst();

    // store: 3 orders
    ai.storeRelaxed(1);
    ai.storeRelease(2);
    ai.storeSeqCst(3);

    // exchange: 5 orders
    (void)ai.exchangeRelaxed(1);
    (void)ai.exchangeAcquire(1);
    (void)ai.exchangeRelease(1);
    (void)ai.exchangeAcqRel(1);
    (void)ai.exchangeSeqCst(1);

    // fetchAdd / fetchSub / fetchAnd / fetchOr / fetchXor (integer) -- 5 orders x 5 ops
    (void)ai.fetchAddRelaxed(1);
    (void)ai.fetchAddAcquire(1);
    (void)ai.fetchAddRelease(1);
    (void)ai.fetchAddAcqRel(1);
    (void)ai.fetchAddSeqCst(1);
    (void)ai.fetchSubRelaxed(1);
    (void)ai.fetchSubAcquire(1);
    (void)ai.fetchSubRelease(1);
    (void)ai.fetchSubAcqRel(1);
    (void)ai.fetchSubSeqCst(1);
    (void)ai.fetchAndRelaxed(1);
    (void)ai.fetchAndAcquire(1);
    (void)ai.fetchAndRelease(1);
    (void)ai.fetchAndAcqRel(1);
    (void)ai.fetchAndSeqCst(1);
    (void)ai.fetchOrRelaxed(1);
    (void)ai.fetchOrAcquire(1);
    (void)ai.fetchOrRelease(1);
    (void)ai.fetchOrAcqRel(1);
    (void)ai.fetchOrSeqCst(1);
    (void)ai.fetchXorRelaxed(1);
    (void)ai.fetchXorAcquire(1);
    (void)ai.fetchXorRelease(1);
    (void)ai.fetchXorAcqRel(1);
    (void)ai.fetchXorSeqCst(1);

    // fetchAdd / fetchSub (pointer) -- 5 orders x 2 ops
    sf::Atomic<int*> ap2{&dummy};
    (void)ap2.fetchAddRelaxed(1);
    (void)ap2.fetchAddAcquire(1);
    (void)ap2.fetchAddRelease(1);
    (void)ap2.fetchAddAcqRel(1);
    (void)ap2.fetchAddSeqCst(1);
    (void)ap2.fetchSubRelaxed(1);
    (void)ap2.fetchSubAcquire(1);
    (void)ap2.fetchSubRelease(1);
    (void)ap2.fetchSubAcqRel(1);
    (void)ap2.fetchSubSeqCst(1);

    // wait / waitUntil (4-byte) -- 3 orders x 2 ops
    sf::Atomic<sf::base::U32> w32{0u};
    w32.waitOnceRelaxed(0u);
    w32.waitOnceAcquire(0u);
    w32.waitOnceSeqCst(0u);
    w32.waitUntilRelaxed([](sf::base::U32) { return true; });
    w32.waitUntilAcquire([](sf::base::U32) { return true; });
    w32.waitUntilSeqCst([](sf::base::U32) { return true; });

    // wait / waitUntil (8-byte) -- 3 orders x 2 ops
    sf::Atomic<sf::base::U64> w64{0u};
    w64.waitOnceRelaxed(0u);
    w64.waitOnceAcquire(0u);
    w64.waitOnceSeqCst(0u);
    w64.waitUntilRelaxed([](sf::base::U64) { return true; });
    w64.waitUntilAcquire([](sf::base::U64) { return true; });
    w64.waitUntilSeqCst([](sf::base::U64) { return true; });
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
    CHECK(a.loadRelaxed() == initial);
    CHECK(a.loadAcquire() == initial);
    CHECK(a.loadSeqCst() == initial);

    a.storeRelaxed(other);
    CHECK(a.loadRelaxed() == other);

    a.storeRelease(initial);
    CHECK(a.loadAcquire() == initial);

    a.storeSeqCst(other);
    CHECK(a.loadSeqCst() == other);
}


////////////////////////////////////////////////////////////
template <typename T>
void exchangeRoundTrip(const T initial, const T other)
{
    sf::Atomic<T> a{initial};

    const T prev = a.exchangeSeqCst(other);
    CHECK(prev == initial);
    CHECK(a.loadRelaxed() == other);
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
        CHECK(a.loadRelaxed() == other);
    }

    // compareExchangeStrong failure
    {
        sf::Atomic<T> a{initial};
        T             expected = other;

        CHECK(!a.template compareExchangeStrong<sf::MemoryOrder::SeqCst, sf::MemoryOrder::Acquire>(expected, third));
        CHECK(expected == initial); // updated to actual on failure
        CHECK(a.loadRelaxed() == initial);
    }

    // compareExchangeWeak in a retry loop (handles spurious failure)
    {
        sf::Atomic<T> a{initial};
        T             expected = initial;

        while (!a.template compareExchangeWeak<sf::MemoryOrder::SeqCst, sf::MemoryOrder::Relaxed>(expected, other))
        {
            // keep retrying with refreshed `expected`
        }

        CHECK(a.loadRelaxed() == other);
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
        const T       prev = a.fetchAddSeqCst(one);
        CHECK(prev == zero);
        CHECK(a.loadRelaxed() == static_cast<T>(zero + one));
    }

    {
        sf::Atomic<T> a{static_cast<T>(zero + one)};
        const T       prev = a.fetchSubSeqCst(one);
        CHECK(prev == static_cast<T>(zero + one));
        CHECK(a.loadRelaxed() == zero);
    }

    {
        sf::Atomic<T> a{mask};
        const T       prev = a.fetchAndSeqCst(static_cast<T>(zero));
        CHECK(prev == mask);
        CHECK(a.loadRelaxed() == zero);
    }

    {
        sf::Atomic<T> a{zero};
        const T       prev = a.fetchOrSeqCst(mask);
        CHECK(prev == zero);
        CHECK(a.loadRelaxed() == mask);
    }

    {
        sf::Atomic<T> a{mask};
        const T       prev = a.fetchXorSeqCst(mask);
        CHECK(prev == mask);
        CHECK(a.loadRelaxed() == zero);
    }
}
} // namespace

TEST_CASE("[System] SFML/System/Atomic.hpp - bool")
{
    singleThreadedRoundTrip<bool>(false, true);
    exchangeRoundTrip<bool>(false, true);
    casRoundTrip<bool>(false, true, false);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - char")
{
    singleThreadedRoundTrip<char>('a', 'z');
    exchangeRoundTrip<char>('a', 'z');
    casRoundTrip<char>('a', 'z', 'm');
    integralFetchOps<char>(static_cast<char>(0x00), static_cast<char>(0x01), static_cast<char>(0x55));
}

TEST_CASE("[System] SFML/System/Atomic.hpp - signed/unsigned 8-bit")
{
    singleThreadedRoundTrip<sf::base::I8>(-3, 7);
    exchangeRoundTrip<sf::base::U8>(0u, 0xFFu);
    casRoundTrip<sf::base::I8>(0, 1, 2);
    integralFetchOps<sf::base::U8>(0u, 1u, 0x55u);
    integralFetchOps<sf::base::I8>(0, 1, 0x55);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - 16-bit")
{
    singleThreadedRoundTrip<sf::base::I16>(-1234, 5678);
    exchangeRoundTrip<sf::base::U16>(0u, 0xFF'FFu);
    casRoundTrip<sf::base::U16>(0u, 1u, 2u);
    integralFetchOps<sf::base::U16>(0u, 1u, 0x55'55u);
    integralFetchOps<sf::base::I16>(0, 1, 0x55'55);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - 32-bit")
{
    singleThreadedRoundTrip<sf::base::I32>(-100'000, 200'000);
    exchangeRoundTrip<sf::base::U32>(0u, 0xDE'AD'BE'EFu);
    casRoundTrip<sf::base::U32>(1u, 2u, 3u);
    integralFetchOps<sf::base::U32>(0u, 1u, 0x55'55'55'55u);
    integralFetchOps<sf::base::I32>(0, 1, 0x55'55'55'55);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - 64-bit")
{
    singleThreadedRoundTrip<sf::base::I64>(-(static_cast<sf::base::I64>(1) << 40), static_cast<sf::base::I64>(1) << 50);
    exchangeRoundTrip<sf::base::U64>(0u, 0xDE'AD'BE'EF'CA'FE'BA'BEu);
    casRoundTrip<sf::base::U64>(1u, 2u, 3u);
    integralFetchOps<sf::base::U64>(0u, 1u, 0x55'55'55'55'55'55'55'55u);
    integralFetchOps<sf::base::I64>(0, 1, 0x55'55'55'55'55'55'55'55);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - float")
{
    singleThreadedRoundTrip<float>(1.5f, -2.5f);
    exchangeRoundTrip<float>(0.0f, 3.14f);
    casRoundTrip<float>(1.0f, 2.0f, 3.0f);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - double")
{
    singleThreadedRoundTrip<double>(1.5, -2.5);
    exchangeRoundTrip<double>(0.0, 3.14159);
    casRoundTrip<double>(1.0, 2.0, 3.0);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - pointer")
{
    int storage[8]{};

    singleThreadedRoundTrip<int*>(&storage[0], &storage[5]);
    exchangeRoundTrip<int*>(&storage[0], &storage[3]);
    casRoundTrip<int*>(&storage[0], &storage[1], &storage[2]);

    SECTION("Pointer fetchAdd / fetchSub scale by element size")
    {
        sf::Atomic<int*> a{&storage[0]};

        int* const after = a.fetchAddSeqCst(3);
        CHECK(after == &storage[0]);
        CHECK(a.loadRelaxed() == &storage[3]);

        int* const after2 = a.fetchSubSeqCst(2);
        CHECK(after2 == &storage[3]);
        CHECK(a.loadRelaxed() == &storage[1]);
    }
}

TEST_CASE("[System] SFML/System/Atomic.hpp - default-constructed and value-constructed")
{
    sf::Atomic<int> defaulted{0};
    CHECK(defaulted.loadRelaxed() == 0);
    defaulted.storeRelaxed(7);
    CHECK(defaulted.loadRelaxed() == 7);

    constexpr sf::Atomic<int> valueCtor{42};
    CHECK(valueCtor.loadRelaxed() == 42);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - thread fences (smoke)")
{
    sf::Atomic<int> a{0};
    sf::Atomic<int> b{0};

    sf::atomicThreadFence<sf::MemoryOrder::SeqCst>();
    a.storeRelaxed(1);
    sf::atomicThreadFence<sf::MemoryOrder::Release>();
    b.storeRelaxed(2);
    sf::atomicThreadFence<sf::MemoryOrder::Acquire>();

    CHECK(a.loadRelaxed() == 1);
    CHECK(b.loadRelaxed() == 2);

    sf::atomicSignalFence<sf::MemoryOrder::SeqCst>();
}

TEST_CASE("[System] SFML/System/Atomic.hpp - concurrent fetchAdd is consistent")
{
    constexpr int             threadCount         = 8;
    constexpr int             incrementsPerThread = 50'000;
    sf::Atomic<sf::base::I64> counter{0};

    sf::base::Vector<sf::Thread> threads;
    threads.reserve(threadCount);

    for (int t = 0; t < threadCount; ++t)
        threads.emplaceBack([&counter]
        {
            for (int i = 0; i < incrementsPerThread; ++i)
                counter.fetchAddRelaxed(1);
        });

    for (auto& th : threads)
        th.join();

    CHECK(counter.loadSeqCst() == static_cast<sf::base::I64>(threadCount) * static_cast<sf::base::I64>(incrementsPerThread));
}

TEST_CASE("[System] SFML/System/Atomic.hpp - producer/consumer release-acquire handoff")
{
    sf::Atomic<int> data{0};
    sf::Atomic<int> ready{0};

    sf::Thread producer([&]
    {
        data.storeRelaxed(42);
        ready.storeRelease(1);
    });

    sf::Thread consumer([&]
    {
        while (ready.loadAcquire() == 0)
            sf::ThisThread::yield();

        CHECK(data.loadRelaxed() == 42);
    });

    producer.join();
    consumer.join();
}

TEST_CASE("[System] SFML/System/Atomic.hpp - CAS-loop float increment (no fetchAdd for floats)")
{
    sf::Atomic<float> total{0.0f};

    constexpr int   threadCount         = 4;
    constexpr int   incrementsPerThread = 25'000;
    constexpr float perStep             = 0.25f;

    sf::base::Vector<sf::Thread> threads;
    threads.reserve(threadCount);

    for (int t = 0; t < threadCount; ++t)
        threads.emplaceBack([&]
        {
            for (int i = 0; i < incrementsPerThread; ++i)
            {
                float current = total.loadRelaxed();

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
    CHECK(static_cast<double>(total.loadSeqCst()) == tst::Approx(expected));
}


////////////////////////////////////////////////////////////
// wait / waitUntil are only available for sizeof(T) in {4, 8}
////////////////////////////////////////////////////////////
template <typename A, typename T>
concept HasWait = requires(A& a, T v) { a.waitOnceAcquire(v); };

static_assert(HasWait<sf::Atomic<sf::base::U32>, sf::base::U32>);
static_assert(HasWait<sf::Atomic<sf::base::U64>, sf::base::U64>);
static_assert(HasWait<sf::Atomic<float>, float>);
static_assert(HasWait<sf::Atomic<double>, double>);
static_assert(HasWait<sf::Atomic<int*>, int*>);
static_assert(!HasWait<sf::Atomic<bool>, bool>);
static_assert(!HasWait<sf::Atomic<char>, char>);
static_assert(!HasWait<sf::Atomic<sf::base::U16>, sf::base::U16>);

TEST_CASE("[System] SFML/System/Atomic.hpp - waitUntil / notifyOne (latch pattern)")
{
    constexpr int             workerCount = 6;
    sf::Atomic<sf::base::U32> nRemaining{static_cast<sf::base::U32>(workerCount)};

    sf::base::Vector<sf::Thread> workers;
    workers.reserve(workerCount);

    for (int i = 0; i < workerCount; ++i)
        workers.emplaceBack([&]
        {
            // simulate a tiny chunk of work
            sf::ThisThread::sleepFor(sf::microseconds(100));

            // last worker to finish notifies the latch (mirrors std::latch semantics)
            if (nRemaining.fetchSubRelease(1) == 1u)
                nRemaining.notifyOne();
        });

    nRemaining.waitUntilAcquire([](const sf::base::U32 v) { return v == 0u; });

    CHECK(nRemaining.loadRelaxed() == 0u);

    for (auto& th : workers)
        th.join();
}

TEST_CASE("[System] SFML/System/Atomic.hpp - notifyAll wakes multiple waiters")
{
    sf::Atomic<sf::base::U32>    gate{0u};
    sf::Atomic<sf::base::U32>    woken{0u};
    constexpr int                waiterCount = 4;
    sf::base::Vector<sf::Thread> waiters;
    waiters.reserve(waiterCount);

    for (int i = 0; i < waiterCount; ++i)
        waiters.emplaceBack([&]
        {
            gate.waitUntilAcquire([](const sf::base::U32 v) { return v != 0u; });
            woken.fetchAddRelaxed(1u);
        });

    // Give the waiters a moment to actually start blocking
    sf::ThisThread::sleepFor(sf::milliseconds(50));

    gate.storeRelease(1u);
    gate.notifyAll();

    for (auto& th : waiters)
        th.join();

    CHECK(woken.loadRelaxed() == static_cast<sf::base::U32>(waiterCount));
}

TEST_CASE("[System] SFML/System/Atomic.hpp - waitUntil predicate already true returns immediately")
{
    sf::Atomic<sf::base::U32> a{42u};

    a.waitUntilAcquire([](const sf::base::U32 v) { return v == 42u; });

    CHECK(a.loadRelaxed() == 42u);
}

TEST_CASE("[System] SFML/System/Atomic.hpp - 64-bit waitUntil")
{
    sf::Atomic<sf::base::U64> counter{3u};

    sf::Thread decrementer([&]
    {
        sf::ThisThread::sleepFor(sf::milliseconds(20));
        counter.storeRelease(0u);
        counter.notifyAll();
    });

    counter.waitUntilAcquire([](const sf::base::U64 v) { return v == 0u; });
    CHECK(counter.loadRelaxed() == 0u);

    decrementer.join();
}
