#pragma once

#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/InterferenceSize.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#include <atomic>


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] void atomicWaitUntil(std::atomic<T>&   a,
                                                          auto&&            predicate,
                                                          std::memory_order order = std::memory_order_acquire)
{
    while (true)
    {
        const T val = a.load(order);

        if (predicate(val))
            return;

        a.wait(val, order);
    }
}


////////////////////////////////////////////////////////////
class SweepAndPrune
{
private:
    struct AABB
    {
        float         minX, maxX, minY, maxY;
        sf::base::U32 objIdx;
    };

    sf::base::Vector<AABB> m_aabbs;

public:
    ////////////////////////////////////////////////////////////
    void forEachUniqueIndexPair(const sf::base::SizeT nWorkers, auto& pool, auto& func)
    {
        const sf::base::SizeT numObjects = m_aabbs.size();

        if (numObjects < 2)
            return;

        const auto processOne = [this, numObjects, &func](sf::base::SizeT i)
        {
            const AABB& aabb1 = m_aabbs[i];

            for (sf::base::SizeT j = i + 1; j < numObjects; ++j)
            {
                const AABB& aabb2 = m_aabbs[j];

                // Early exit: since `m_aabbs` is sorted by `minX`,
                // if `aabb2.minX` is greater than `aabb1.maxX`, no further objects will overlap on the x-axis.
                if (aabb2.minX > aabb1.maxX)
                    break;

                // Since the x intervals overlap, check the y intervals.
                if (aabb1.minY <= aabb2.maxY && aabb1.maxY >= aabb2.minY)
                {
                    func(sf::base::min(aabb1.objIdx, aabb2.objIdx), sf::base::max(aabb1.objIdx, aabb2.objIdx));
                }
            }
        };

        // If there's only one worker, process synchronously.
        if (nWorkers <= 1u)
        {
            for (sf::base::SizeT i = 0; i < numObjects; ++i)
                processOne(i);

            return;
        }

        // Dynamic scheduling: each thread grabs the next row via atomic counter.
        // This naturally balances load since early rows (low i) have much more work
        // than late rows (high i) due to longer inner loops and less effective early-exit.
        alignas(sf::base::hardwareDestructiveInterferenceSize) std::atomic<sf::base::SizeT> nextI{0};
        alignas(sf::base::hardwareDestructiveInterferenceSize) std::atomic<sf::base::SizeT> nRemaining{nWorkers};

        auto worker = [&]
        {
            while (true)
            {
                const auto i = nextI.fetch_add(1, std::memory_order_relaxed);

                if (i >= numObjects)
                    break;

                processOne(i);
            }

            // Only notify when the last worker finishes (like std::latch).
            if (nRemaining.fetch_sub(1, std::memory_order_release) == 1)
                nRemaining.notify_one();
        };

        // Launch asynchronous workers.
        for (sf::base::SizeT iWorker = 1u; iWorker < nWorkers; ++iWorker)
            pool.post(worker);

        // Main thread also participates as a worker.
        worker();

        // Wait until all workers finish.
        atomicWaitUntil(nRemaining, [](sf::base::SizeT val) { return val == 0; });
    }

    ////////////////////////////////////////////////////////////
    void populate(const auto& bubbles)
    {
        m_aabbs.reserve(bubbles.size());
        m_aabbs.clear();

        for (sf::base::SizeT i = 0u; i < bubbles.size(); ++i)
        {
            const auto& b = bubbles[i];
            m_aabbs.unsafeEmplaceBack(b.position.x - b.radius,
                                      b.position.x + b.radius,
                                      b.position.y - b.radius,
                                      b.position.y + b.radius,
                                      static_cast<unsigned int>(i));
        }

        sf::base::quickSort(m_aabbs.begin(), m_aabbs.end(), [](const AABB& a, const AABB& b) { return a.minX < b.minX; });
    }
};
