#include "SFML/Base/ThreadPool.hpp"

#include <Doctest.hpp>

#include <atomic>


TEST_CASE("[Base] Base/ThreadPool.hpp")
{
    SECTION("Construct with 0 workers")
    {
        sf::base::ThreadPool pool(0u);
        REQUIRE(pool.getWorkerCount() == 0u);
    }

    SECTION("Construct with 1 worker")
    {
        sf::base::ThreadPool pool(1u);
        REQUIRE(pool.getWorkerCount() == 1u);
    }

    SECTION("Construct with 4 workers")
    {
        sf::base::ThreadPool pool(4u);
        REQUIRE(pool.getWorkerCount() == 4u);
    }

    SECTION("Construct with 8 workers")
    {
        sf::base::ThreadPool pool(8u);
        REQUIRE(pool.getWorkerCount() == 8u);
    }

    SECTION("Construct with 16 workers")
    {
        sf::base::ThreadPool pool(16u);
        REQUIRE(pool.getWorkerCount() == 16u);
    }

    SECTION("Construct with 32 workers")
    {
        sf::base::ThreadPool pool(32u);
        REQUIRE(pool.getWorkerCount() == 32u);
    }

    const auto doJoinTest = [](std::atomic<int>& result, const int nTasks)
    {
        sf::base::ThreadPool pool(4u);

        for (int i = 0; i < nTasks; ++i)
            pool.post([&] { result.fetch_add(1, std::memory_order::relaxed); });
    };

    SECTION("Join 1 task on destruction")
    {
        std::atomic<int> result = 0;
        doJoinTest(result, 1);
        REQUIRE(result.load(std::memory_order::relaxed) == 1);
    }

    SECTION("Join 2 tasks on destruction")
    {
        std::atomic<int> result = 0;
        doJoinTest(result, 2);
        REQUIRE(result.load(std::memory_order::relaxed) == 2);
    }

    SECTION("Join 4 tasks on destruction")
    {
        std::atomic<int> result = 0;
        doJoinTest(result, 4);
        REQUIRE(result.load(std::memory_order::relaxed) == 4);
    }

    SECTION("Join 8 tasks on destruction")
    {
        std::atomic<int> result = 0;
        doJoinTest(result, 8);
        REQUIRE(result.load(std::memory_order::relaxed) == 8);
    }

    SECTION("Join 256 tasks on destruction")
    {
        std::atomic<int> result = 0;
        doJoinTest(result, 256);
        REQUIRE(result.load(std::memory_order::relaxed) == 256);
    }
}
