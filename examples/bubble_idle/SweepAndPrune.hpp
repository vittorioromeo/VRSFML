#pragma once

#include "Zancle/Concurrency/Atomic.hpp"

#include "Zancle/Algorithm/Sort.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/InterferenceSize.hpp"
#include "Zancle/Math/MinMax.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Container/Vector.hpp"


////////////////////////////////////////////////////////////
class SweepAndPrune
{
private:
    struct AABB
    {
        float   minX, maxX, minY, maxY;
        za::U32 objIdx;
    };

    za::Vector<AABB> m_aabbs;

public:
    ////////////////////////////////////////////////////////////
    void forEachUniqueIndexPair(const za::SizeT nWorkers, auto& pool, auto& func)
    {
        const za::SizeT numObjects = m_aabbs.size();

        if (numObjects < 2)
            return;

        const auto processOne = [this, numObjects, &func](za::SizeT i)
        {
            const AABB& aabb1 = m_aabbs[i];

            for (za::SizeT j = i + 1; j < numObjects; ++j)
            {
                const AABB& aabb2 = m_aabbs[j];

                // Early exit: since `m_aabbs` is sorted by `minX`,
                // if `aabb2.minX` is greater than `aabb1.maxX`, no further objects will overlap on the x-axis.
                if (aabb2.minX > aabb1.maxX)
                    break;

                // Since the x intervals overlap, check the y intervals.
                if (aabb1.minY <= aabb2.maxY && aabb1.maxY >= aabb2.minY)
                {
                    func(za::min(aabb1.objIdx, aabb2.objIdx), za::max(aabb1.objIdx, aabb2.objIdx));
                }
            }
        };

        // If there's only one worker, process synchronously.
        if (nWorkers <= 1u)
        {
            for (za::SizeT i = 0; i < numObjects; ++i)
                processOne(i);

            return;
        }

        // Dynamic scheduling: each thread grabs the next row via atomic counter.
        // This naturally balances load since early rows (low i) have much more work
        // than late rows (high i) due to longer inner loops and less effective early-exit.
        alignas(za::hardwareDestructiveInterferenceSize) za::Atomic<za::SizeT> nextI{0};
        alignas(za::hardwareDestructiveInterferenceSize) za::Atomic<za::SizeT> nRemaining{nWorkers};

        auto worker = [&]
        {
            while (true)
            {
                const auto i = nextI.fetchAddRelaxed(1);

                if (i >= numObjects)
                    break;

                processOne(i);
            }

            // Only notify when the last worker finishes (like std::latch).
            if (nRemaining.fetchSubRelease(1) == 1)
                nRemaining.notifyOne();
        };

        // Launch asynchronous workers.
        for (za::SizeT iWorker = 1u; iWorker < nWorkers; ++iWorker)
            pool.post(worker);

        // Main thread also participates as a worker.
        worker();

        // Wait until all workers finish.
        nRemaining.waitUntilAcquire([](za::SizeT val) { return val == 0; });
    }

    ////////////////////////////////////////////////////////////
    void populate(const auto& bubbles)
    {
        m_aabbs.reserve(bubbles.size());
        m_aabbs.clear();

        for (za::SizeT i = 0u; i < bubbles.size(); ++i)
        {
            const auto& b = bubbles[i];
            m_aabbs.unsafeEmplaceBack(b.position.x - b.radius,
                                      b.position.x + b.radius,
                                      b.position.y - b.radius,
                                      b.position.y + b.radius,
                                      static_cast<unsigned int>(i));
        }

        za::quickSort(m_aabbs.begin(), m_aabbs.end(), [](const AABB& a, const AABB& b) { return a.minX < b.minX; });
    }
};
