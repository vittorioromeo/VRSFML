#include "Tst/Tst.hpp"

#include "Zancle/Concurrency/ThreadPool.hpp"

#include "Zancle/Concurrency/Atomic.hpp"


TEST_CASE("[Base] Base/ThreadPool.hpp")
{
    SECTION("Construct with 1 worker")
    {
        za::ThreadPool pool(1u);
        REQUIRE(pool.getWorkerCount() == 1u);
    }

    SECTION("Construct with 4 workers")
    {
        za::ThreadPool pool(4u);
        REQUIRE(pool.getWorkerCount() == 4u);
    }

    SECTION("Construct with 8 workers")
    {
        za::ThreadPool pool(8u);
        REQUIRE(pool.getWorkerCount() == 8u);
    }

    SECTION("Construct with 16 workers")
    {
        za::ThreadPool pool(16u);
        REQUIRE(pool.getWorkerCount() == 16u);
    }

    SECTION("Construct with 32 workers")
    {
        za::ThreadPool pool(32u);
        REQUIRE(pool.getWorkerCount() == 32u);
    }

    const auto doJoinTest = [](za::Atomic<int>& result, const int nTasks)
    {
        za::ThreadPool pool(4u);

        for (int i = 0; i < nTasks; ++i)
            pool.post([&] { result.fetchAddRelaxed(1); });
    };

    SECTION("Join 1 task on destruction")
    {
        za::Atomic<int> result{0};
        doJoinTest(result, 1);
        REQUIRE(result.loadRelaxed() == 1);
    }

    SECTION("Join 2 tasks on destruction")
    {
        za::Atomic<int> result{0};
        doJoinTest(result, 2);
        REQUIRE(result.loadRelaxed() == 2);
    }

    SECTION("Join 4 tasks on destruction")
    {
        za::Atomic<int> result{0};
        doJoinTest(result, 4);
        REQUIRE(result.loadRelaxed() == 4);
    }

    SECTION("Join 8 tasks on destruction")
    {
        za::Atomic<int> result{0};
        doJoinTest(result, 8);
        REQUIRE(result.loadRelaxed() == 8);
    }

    SECTION("Join 256 tasks on destruction")
    {
        za::Atomic<int> result{0};
        doJoinTest(result, 256);
        REQUIRE(result.loadRelaxed() == 256);
    }
}
