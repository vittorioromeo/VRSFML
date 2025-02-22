#pragma once

#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TrivialVector.hpp"

#include <algorithm>
#include <latch>

#include <cassert>
#include <cstddef>


class SweepAndPrune
{
private:
    struct AABB
    {
        float           minX, maxX, minY, maxY;
        sf::base::SizeT objIdx;
    };

    sf::base::TrivialVector<AABB> m_aabbs;

public:
    ////////////////////////////////////////////////////////////
    void forEachUniqueIndexPair(const unsigned int nWorkers, auto& pool, auto& func)
    {
        const sf::base::SizeT numObjects = m_aabbs.size();

        if (numObjects < 2)
            return;

        // Compute a chunk size for dividing the outer loop among tasks.
        const sf::base::SizeT chunkSize = (numObjects + nWorkers - 1) / nWorkers;

        const auto processChunk = [this, numObjects, &func](sf::base::SizeT start, sf::base::SizeT end)
        {
            for (sf::base::SizeT i = start; i < end; ++i)
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
                        // Call the user-supplied function with the indices in sorted order.
                        func(sf::base::min(aabb1.objIdx, aabb2.objIdx), sf::base::max(aabb1.objIdx, aabb2.objIdx));
                    }
                }
            }
        };

        // If there's only one worker, process synchronously.
        if (nWorkers <= 1)
        {
            processChunk(0u, numObjects);
            return;
        }

        // Initialize latch for the asynchronous tasks only.
        std::latch latch{static_cast<std::ptrdiff_t>(nWorkers - 1u)};

        // Process the first chunk on the main thread.
        {
            const sf::base::SizeT start = 0;
            const sf::base::SizeT end   = sf::base::min(chunkSize, numObjects);
            processChunk(start, end);
        }

        // Launch asynchronous tasks for remaining chunks.
        for (unsigned int iWorker = 1u; iWorker < nWorkers; ++iWorker)
        {
            const sf::base::SizeT start = iWorker * chunkSize;
            const sf::base::SizeT end   = sf::base::min((iWorker + 1u) * chunkSize, numObjects);

            if (start >= numObjects)
            {
                // If there is no work for this task, decrement the latch for each missing task.
                latch.count_down(nWorkers - iWorker);
                break;
            }

            pool.post([start, end, &latch, processChunk]()
            {
                processChunk(start, end);
                latch.count_down();
            });
        }

        latch.wait();
    }

    ////////////////////////////////////////////////////////////
    void clear()
    {
        m_aabbs.clear();
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
                                      i);
        }

        std::sort(m_aabbs.begin(), m_aabbs.end(), [](const AABB& a, const AABB& b) { return a.minX < b.minX; });
    }
};
