// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "Pool.hpp"

#include "TaskQueue.hpp"
#include "Worker.hpp"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>


namespace hg::ThreadPool
{
////////////////////////////////////////////////////////////
struct Pool::Impl
{
    TaskQueue                 queue;
    std::vector<Worker>       workers;
    std::atomic<unsigned int> remainingInits;
};


////////////////////////////////////////////////////////////
[[nodiscard]] bool Pool::areAllWorkersDoneBlockingProcessing() const noexcept
{
    for (const Worker& w : m_impl->workers)
        if (!w.isDoneBlockingProcessing())
            return false;

    return true;
}


////////////////////////////////////////////////////////////
Pool::Pool(unsigned int workerCount)
{
    for (unsigned int i = 0u; i < workerCount; ++i)
        m_impl->workers.emplace_back(m_impl->queue);

    m_impl->remainingInits.store(workerCount);

    for (Worker& w : m_impl->workers)
        w.start(static_cast<void*>(&m_impl->remainingInits));
}


////////////////////////////////////////////////////////////
Pool::~Pool()
{
    // Busy wait until all workers are initialized.
    while (m_impl->remainingInits.load() > 0u)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Signal all workers to exit their processing loops.
    for (Worker& w : m_impl->workers)
        w.stop();

    // Post dummy tasks until all workers have exited their loops.
    while (!areAllWorkersDoneBlockingProcessing())
        post([] {});

    // Join the workers' threads.
    for (Worker& w : m_impl->workers)
        w.join();
}


////////////////////////////////////////////////////////////
void Pool::post(Task&& f)
{
    [[maybe_unused]] const bool enqueued = m_impl->queue.enqueue(std::move(f));
    SFML_BASE_ASSERT(enqueued);
}


////////////////////////////////////////////////////////////
unsigned int Pool::getWorkerCount() const noexcept
{
    return static_cast<unsigned int>(m_impl->workers.size());
}

} // namespace hg::ThreadPool
