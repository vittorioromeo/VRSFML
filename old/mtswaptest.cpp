

//
//
// Get hardware constants
inline const auto     nMaxWorkers   = static_cast<sf::base::U64>(sf::base::ThreadPool::getHardwareWorkerCount());
inline constexpr auto cacheLineSize = static_cast<sf::base::SizeT>(sf::base::hardwareDestructiveInterferenceSize);

sf::base::U64 nWorkers = nMaxWorkers;

sf::base::ThreadPool pool(nMaxWorkers);

void doInBatches(const sf::base::SizeT nParticlesTotal, auto&& f)
{
    const sf::base::SizeT particlesPerBatch = nParticlesTotal / nWorkers;

    std::latch latch{static_cast<sf::base::PtrDiffT>(nWorkers)};

    for (sf::base::SizeT i = 0u; i < nWorkers; ++i)
    {
        pool.post([&, i]
        {
            const sf::base::SizeT batchStartIdx = i * particlesPerBatch;
            const sf::base::SizeT batchEndIdx   = (i == nWorkers - 1u) ? nParticlesTotal : (i + 1u) * particlesPerBatch;

            f(i, batchStartIdx, batchEndIdx);

            latch.count_down();
        });
    }

    latch.wait();
};

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    void eraseIfBySwappingTestMT(auto& latch, auto& pool, auto&& f)
    {
        const auto  s    = getSize();
        const auto& pred = SOA_AS_BASE(4).data;

        auto g = [this, s, f, &latch, &pred](auto& base)
        {
            sf::base::SizeT n = s;
            sf::base::SizeT i = 0u;

            // Process elements, swapping out removed ones.
            while (i < n)
            {
                if (!f(pred[i]))
                {
                    ++i;
                    continue;
                }

                // Swap the current element with the last one, then reduce the container size.
                do
                {
                    --n;
                } while (n > i && f(pred[n]));
                // (..., sf::base::swap(SOA_ALL_BASES().data[i], SOA_ALL_BASES().data[n]));
                (base.data[i] = SFML_BASE_MOVE(base.data[n]));

                // Do not increment `i`; check the new element at `i`.
                ++i;
            }

            base.data.resize(n);
            latch.count_down();
        };

        pool.post([this, g] { g(SOA_AS_BASE(0)); });
        pool.post([this, g] { g(SOA_AS_BASE(1)); });
        pool.post([this, g] { g(SOA_AS_BASE(2)); });
        pool.post([this, g] { g(SOA_AS_BASE(3)); });
        pool.post([this, g] { g(SOA_AS_BASE(5)); });
        pool.post([this, g] { g(SOA_AS_BASE(6)); });
        pool.post([this, g] { g(SOA_AS_BASE(7)); });
        pool.post([this, g] { g(SOA_AS_BASE(8)); });

        latch.wait();

        g(SOA_AS_BASE(4));
    }
