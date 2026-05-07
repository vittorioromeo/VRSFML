// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SFML/Base/ThreadPool.hpp"

#include "SFML/System/Atomic.hpp"
#include "SFML/System/Thread.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"

#include <blockingconcurrentqueue.h>
#include <concurrentqueue.h>

#pragma GCC diagnostic pop


namespace sf::base
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
    void start(sf::Atomic<SizeT>& remainingInits)
    {
        m_thread = sf::Thread{[this, &remainingInits]
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
            SFML_BASE_ASSERT(m_state.loadAcquire() == State::Stopped);
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
        SFML_BASE_ASSERT(m_state.loadAcquire() == State::Running);
        m_state.storeRelease(State::Stopped);
    }

    ////////////////////////////////////////////////////////////
    void join() noexcept
    {
        SFML_BASE_ASSERT(m_thread.joinable());
        SFML_BASE_ASSERT(m_state.loadAcquire() == State::Stopped);

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
    sf::Thread             m_thread;                 //!< Worker thread
    TaskQueue*             m_queue;                  //!< Pointer to queue
    TaskQueueConsumerToken m_ctok;                   //!< Consumer token
    sf::Atomic<State>      m_state;                  //!< State (controlled both by the pool and internally)
    sf::Atomic<bool>       m_doneBlockingProcessing; //!< Worker is done processing tasks in blocking mode
};

} // namespace


////////////////////////////////////////////////////////////
struct ThreadPool::Impl
{
    TaskQueue            queue;
    base::Vector<Worker> workers;
    sf::Atomic<SizeT>    remainingInits;
};


////////////////////////////////////////////////////////////
ThreadPool::ThreadPool(const SizeT workerCount)
{
    SFML_BASE_ASSERT(workerCount > 0u);

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
        sf::ThisThread::sleepFor(sf::milliseconds(1));

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
    [[maybe_unused]] const bool enqueued = m_impl->queue.enqueue(SFML_BASE_MOVE(f));
    SFML_BASE_ASSERT(enqueued);
}


////////////////////////////////////////////////////////////
SizeT ThreadPool::getWorkerCount() const noexcept
{
    return m_impl->workers.size();
}


////////////////////////////////////////////////////////////
SizeT ThreadPool::getHardwareWorkerCount() noexcept
{
    return static_cast<SizeT>(sf::Thread::hardwareConcurrency());
}

} // namespace sf::base
