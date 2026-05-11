// Self-contained benchmark to measure the impact of the
// `next()` micro-optimization in `ankerl::unordered_dense::map`
// (the "H1" change: replace the branching wraparound with a
// power-of-two bucket-count mask).
//
// How to use:
//   1. Build and run the benchmark with the *current* code:
//        $ cmake --build build --target ankerl_next_benchmark
//        $ ./build/bin/ankerl_next_benchmark > before.txt
//   2. Apply the H1 change to `AnkerlUnorderedDense.hpp`.
//   3. Rebuild and re-run:
//        $ cmake --build build --target ankerl_next_benchmark
//        $ ./build/bin/ankerl_next_benchmark > after.txt
//   4. `diff -u before.txt after.txt` or eyeball side-by-side.
//
// The output is whitespace-aligned columns of (workload, size, ns/op).
// Each ns/op figure is the median of `kTrials` runs over the same fixed
// workload. The benchmark pins all randomness to a fixed seed so runs are
// directly comparable.
//
// What's being measured:
//   - find_hit:    look up keys that exist in the table.
//   - find_miss:   look up keys absent from the table (probes more buckets
//                  than find_hit on average since misses follow the chain
//                  until dist exceeds).
//   - insert:      build the table from scratch (probes until empty slot).
//   - erase_all:   remove every key (probes until the slot owning the key
//                  is found, then shifts backward via next()).
//
// `next()` runs once per probe step in each of these. The branchless mask
// variant saves a compare + (predicted) branch per probe.

#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
// Small xorshift64* RNG: deterministic, fast, allocation-free.
struct RNG
{
    sf::base::U64 state;

    [[nodiscard]] explicit RNG(sf::base::U64 seed) : state(seed ? seed : sf::base::U64{0x9E3779B97F4A7C15ULL})
    {
    }

    [[nodiscard]] sf::base::U64 next() noexcept
    {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return state * sf::base::U64{0x2545F4914F6CDD1DULL};
    }
};


////////////////////////////////////////////////////////////
// Anti-DCE sink: every workload XORs its result into this so the compiler
// can't elide the loop body.
// NOLINTNEXTLINE(readability-identifier-naming)
volatile sf::base::U64 globalSink = 0;


////////////////////////////////////////////////////////////
constexpr sf::base::SizeT kTrials   = 11; // odd so median is a single sample
constexpr sf::base::SizeT kSizes[]  = {10'000u, 100'000u, 1'000'000u};


////////////////////////////////////////////////////////////
template <typename F>
[[gnu::noinline]] sf::base::U64 timeNs(F&& fn)
{
    sf::Clock clk;
    fn();
    return static_cast<sf::base::U64>(clk.getElapsedTime().asMicroseconds()) * 1000u;
}


////////////////////////////////////////////////////////////
sf::base::U64 medianOf(sf::base::Vector<sf::base::U64>& samples)
{
    sf::base::quickSort(samples.begin(), samples.end());
    return samples[samples.size() / 2u];
}


////////////////////////////////////////////////////////////
using Map = ankerl::unordered_dense::map<sf::base::U64, sf::base::U64>;


////////////////////////////////////////////////////////////
struct KeySet
{
    sf::base::Vector<sf::base::U64> present; // inserted into the map
    sf::base::Vector<sf::base::U64> missing; // disjoint from `present`
};


////////////////////////////////////////////////////////////
[[gnu::noinline]] KeySet makeKeys(sf::base::SizeT n, sf::base::U64 seed)
{
    RNG    rng(seed);
    KeySet ks;
    ks.present.reserve(n);
    ks.missing.reserve(n);

    // Use the top bit to partition the key space; this guarantees `present`
    // and `missing` never collide regardless of RNG output.
    for (sf::base::SizeT i = 0; i < n; ++i)
        ks.present.pushBack(rng.next() & sf::base::U64{0x7FFFFFFFFFFFFFFFULL});
    for (sf::base::SizeT i = 0; i < n; ++i)
        ks.missing.pushBack(rng.next() | sf::base::U64{0x8000000000000000ULL});

    return ks;
}


////////////////////////////////////////////////////////////
[[gnu::noinline]] sf::base::U64 benchInsert(const KeySet& ks)
{
    Map m;
    m.reserve(ks.present.size());
    sf::base::U64 acc = 0;

    const auto ns = timeNs(
        [&]
        {
            for (auto k : ks.present)
                acc ^= m.try_emplace(k, k).first->second;
        });

    globalSink ^= acc;
    return ns / ks.present.size();
}


////////////////////////////////////////////////////////////
[[gnu::noinline]] sf::base::U64 benchFindHit(const Map& m, const sf::base::Vector<sf::base::U64>& keys)
{
    sf::base::U64 acc = 0;
    const auto    ns  = timeNs(
        [&]
        {
            for (auto k : keys)
            {
                const auto* const it = m.find(k);
                acc ^= it == m.end() ? 0u : it->second;
            }
        });

    globalSink ^= acc;
    return ns / keys.size();
}


////////////////////////////////////////////////////////////
[[gnu::noinline]] sf::base::U64 benchFindMiss(const Map& m, const sf::base::Vector<sf::base::U64>& keys)
{
    sf::base::U64 acc = 0;
    const auto    ns  = timeNs(
        [&]
        {
            for (auto k : keys)
            {
                const auto* const it = m.find(k);
                acc ^= it == m.end() ? 1u : it->second; // 1u so miss path still mutates `acc`
            }
        });

    globalSink ^= acc;
    return ns / keys.size();
}


////////////////////////////////////////////////////////////
[[gnu::noinline]] sf::base::U64 benchEraseAll(Map m, const sf::base::Vector<sf::base::U64>& keys)
{
    // takes `m` by value so each trial gets a fresh copy.
    sf::base::U64 acc = 0;
    const auto    ns  = timeNs(
        [&]
        {
            for (auto k : keys)
                acc ^= m.erase(k);
        });

    globalSink ^= acc;
    return ns / keys.size();
}


////////////////////////////////////////////////////////////
[[gnu::noinline]] Map buildMap(const sf::base::Vector<sf::base::U64>& present)
{
    Map m;
    m.reserve(present.size());
    for (auto k : present)
        m.try_emplace(k, k);
    return m;
}


////////////////////////////////////////////////////////////
// Tab-separated for easy diffing / paste into a spreadsheet.
void printRow(const char* label, sf::base::SizeT n, sf::base::U64 nsPerOp)
{
    sf::cOut() << label << '\t' << n << '\t' << nsPerOp << " ns/op\n";
}


////////////////////////////////////////////////////////////
void runSize(sf::base::SizeT n)
{
    const auto keys = makeKeys(n, /* seed */ sf::base::U64{0xABCDEFULL} ^ n);

    sf::base::Vector<sf::base::U64> insertSamples;
    sf::base::Vector<sf::base::U64> findHitSamples;
    sf::base::Vector<sf::base::U64> findMissSamples;
    sf::base::Vector<sf::base::U64> eraseSamples;

    insertSamples.reserve(kTrials);
    findHitSamples.reserve(kTrials);
    findMissSamples.reserve(kTrials);
    eraseSamples.reserve(kTrials);

    // Warmup: one untimed pass so the allocator / branch predictor are settled.
    {
        Map warm = buildMap(keys.present);
        (void)benchFindHit(warm, keys.present);
        (void)benchFindMiss(warm, keys.missing);
    }

    for (sf::base::SizeT t = 0; t < kTrials; ++t)
        insertSamples.pushBack(benchInsert(keys));

    const Map base = buildMap(keys.present);
    for (sf::base::SizeT t = 0; t < kTrials; ++t)
        findHitSamples.pushBack(benchFindHit(base, keys.present));
    for (sf::base::SizeT t = 0; t < kTrials; ++t)
        findMissSamples.pushBack(benchFindMiss(base, keys.missing));
    for (sf::base::SizeT t = 0; t < kTrials; ++t)
        eraseSamples.pushBack(benchEraseAll(base, keys.present));

    printRow("insert", n, medianOf(insertSamples));
    printRow("find_hit", n, medianOf(findHitSamples));
    printRow("find_miss", n, medianOf(findMissSamples));
    printRow("erase_all", n, medianOf(eraseSamples));
}

} // namespace


////////////////////////////////////////////////////////////
int main()
{
    sf::cOut() << "ankerl::unordered_dense::map  next()-microbenchmark\n";
    sf::cOut() << "trials=" << kTrials << "  (reports median ns/op)\n";
    sf::cOut() << "workload\tN\tns/op\n";

    for (auto n : kSizes)
        runSize(n);

    // Print the anti-DCE sink so the compiler can't prove it's dead.
    const sf::base::U64 sinkValue = globalSink;
    sf::cOut() << "sink (ignore): " << sinkValue << '\n';

    return 0;
}
