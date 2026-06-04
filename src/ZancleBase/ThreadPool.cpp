// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "ZancleBase/ThreadPool.hpp"

#include "Zancle/System/Atomic.hpp"
#include "Zancle/System/Thread.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"

#include <blockingconcurrentqueue.h>
#include <concurrentqueue.h>

#pragma GCC diagnostic pop


namespace zb
{
namespace
{
////////////////////////////////////////////////////////////
using TaskQueue              = moodycamel::BlockingConcurrentQueue<ThreadPool::Task>;
using TaskQueueConsumerToken = moodycamel::ConsumerToken;


////////////////////////////////////////////////////////////
class [[nodiscard]] Worker
{
public:
    ////////////////////////////////////////////////////////////
    explicit Worker(TaskQueue& queue) noexcept :
        m_queue{&queue},
        m_ctok{queue},
        m_state{State::Running},
        m_doneBlockingProcessing{false}
    {
    }

    ////////////////////////////////////////////////////////////
    void start(za::Atomic<SizeT>& remainingInits)
    {
        m_thread = za::Thread{[this, &remainingInits]
        {
            // Set the running flag and signal to the pool that we are initialized.
            m_state.storeRelease(State::Running);
            remainingInits.fetchSubRelease(1u);

            ThreadPool::Task taskBuffer;

            while (m_state.loadAcquire() == State::Running)
            {
                m_queue->wait_dequeue(m_ctok, taskBuffer); // Blocking
                taskBuffer();
            }

            // Signal the thread pool to send dummy final tasks.
            ZB_ASSERT(m_state.loadAcquire() == State::Stopped);
            m_doneBlockingProcessing.storeRelease(true);

            while (m_state.loadAcquire() == State::Stopped)
            {
                if (!m_queue->try_dequeue(m_ctok, taskBuffer)) // Non-blocking
                    break;                                     // No more tasks available

                taskBuffer();
            }
        }};
    }

    ////////////////////////////////////////////////////////////
    void stop() noexcept
    {
        ZB_ASSERT(m_state.loadAcquire() == State::Running);
        m_state.storeRelease(State::Stopped);
    }

    ////////////////////////////////////////////////////////////
    void join() noexcept
    {
        ZB_ASSERT(m_thread.joinable());
        ZB_ASSERT(m_state.loadAcquire() == State::Stopped);

        m_thread.join();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDoneBlockingProcessing() const noexcept
    {
        return m_doneBlockingProcessing.loadAcquire();
    }

private:
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] State : bool
    {
        Running, //!< The worker is dequeuing and accepting tasks in blocking mode
        Stopped, //!< The worker is dequeuing and accepting tasks in non-blocking mode
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    za::Thread             m_thread;                 //!< Worker thread
    TaskQueue*             m_queue;                  //!< Pointer to queue
    TaskQueueConsumerToken m_ctok;                   //!< Consumer token
    za::Atomic<State>      m_state;                  //!< State (controlled both by the pool and internally)
    za::Atomic<bool>       m_doneBlockingProcessing; //!< Worker is done processing tasks in blocking mode
};

} // namespace


////////////////////////////////////////////////////////////
struct ThreadPool::Impl
{
    TaskQueue            queue;
    base::Vector<Worker> workers;
    za::Atomic<SizeT>    remainingInits;
};


////////////////////////////////////////////////////////////
ThreadPool::ThreadPool(const SizeT workerCount)
{
    ZB_ASSERT(workerCount > 0u);

    m_impl->workers.unsafeAllocateCapacity(workerCount);

    for (SizeT i = 0u; i < workerCount; ++i)
        m_impl->workers.unsafeEmplaceBack(m_impl->queue);

    m_impl->remainingInits.storeRelaxed(workerCount);

    for (Worker& w : m_impl->workers)
        w.start(m_impl->remainingInits);
}


////////////////////////////////////////////////////////////
ThreadPool::~ThreadPool()
{
    // Returns `true` if all workers have finished processing packets in a blocking manner.
    const auto areAllWorkersDoneBlockingProcessing = [&]
    {
        for (const Worker& w : m_impl->workers)
            if (!w.isDoneBlockingProcessing())
                return false;

        return true;
    };

    // Busy wait until all workers are initialized.
    while (m_impl->remainingInits.loadAcquire() > 0u)
        za::ThisThread::sleepFor(za::milliseconds(1));

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
void ThreadPool::post(Task&& f)
{
    [[maybe_unused]] const bool enqueued = m_impl->queue.enqueue(ZB_MOVE(f));
    ZB_ASSERT(enqueued);
}


////////////////////////////////////////////////////////////
SizeT ThreadPool::getWorkerCount() const noexcept
{
    return m_impl->workers.size();
}


////////////////////////////////////////////////////////////
SizeT ThreadPool::getHardwareWorkerCount() noexcept
{
    return static_cast<SizeT>(za::Thread::hardwareConcurrency());
}

} // namespace zb
