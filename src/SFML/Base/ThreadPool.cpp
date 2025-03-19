// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SFML/Base/ThreadPool.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include <blockingconcurrentqueue.h>
#include <concurrentqueue.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>


namespace sf::base
{
namespace
{
////////////////////////////////////////////////////////////
using TaskQueue              = moodycamel::BlockingConcurrentQueue<ThreadPool::Task>;
using TaskQueueConsumerToken = moodycamel::ConsumerToken;

////////////////////////////////////////////////////////////
template <typename T>
struct [[nodiscard]] MovableAtomic : std::atomic<T>
{
    ////////////////////////////////////////////////////////////
    using std::atomic<T>::atomic;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] MovableAtomic() = default;

    ////////////////////////////////////////////////////////////
    MovableAtomic(const MovableAtomic&)            = delete;
    MovableAtomic& operator=(const MovableAtomic&) = delete;

    ////////////////////////////////////////////////////////////
    MovableAtomic(MovableAtomic&& rhs) noexcept : std::atomic<T>{rhs.load()}
    {
    }

    ////////////////////////////////////////////////////////////
    MovableAtomic& operator=(MovableAtomic&& rhs) noexcept
    {
        this->store(rhs.load());
        return *this;
    }
};


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
    void start(std::atomic<SizeT>& remainingInits)
    {
        m_thread = std::thread([this, &remainingInits]
        {
            // Set the running flag and signal to the pool that we are initialized.
            m_state.store(State::Running, std::memory_order::release);
            remainingInits.fetch_sub(1u, std::memory_order::release);

            ThreadPool::Task taskBuffer;

            while (m_state.load(std::memory_order::acquire) == State::Running)
            {
                m_queue->wait_dequeue(m_ctok, taskBuffer); // Blocking
                taskBuffer();
            }

            // Signal the thread pool to send dummy final tasks.
            SFML_BASE_ASSERT(m_state.load(std::memory_order::acquire) == State::Stopped);
            m_doneBlockingProcessing.store(true, std::memory_order::release);

            while (m_state.load(std::memory_order::acquire) == State::Stopped)
            {
                if (!m_queue->try_dequeue(m_ctok, taskBuffer)) // Non-blocking
                    break;                                     // No more tasks available

                taskBuffer();
            }
        });
    }

    ////////////////////////////////////////////////////////////
    void stop() noexcept
    {
        SFML_BASE_ASSERT(m_state.load(std::memory_order::acquire) == State::Running);
        m_state.store(State::Stopped, std::memory_order::release);
    }

    ////////////////////////////////////////////////////////////
    void join() noexcept
    {
        SFML_BASE_ASSERT(m_thread.joinable());
        SFML_BASE_ASSERT(m_state.load(std::memory_order::acquire) == State::Stopped);

        m_thread.join();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDoneBlockingProcessing() const noexcept
    {
        return m_doneBlockingProcessing.load(std::memory_order::acquire);
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
    std::thread            m_thread;                 //!< Worker thread
    TaskQueue*             m_queue;                  //!< Pointer to queue
    TaskQueueConsumerToken m_ctok;                   //!< Consumer token
    MovableAtomic<State>   m_state;                  //!< State (controlled both by the pool and internally)
    MovableAtomic<bool>    m_doneBlockingProcessing; //!< Worker is done processing tasks in blocking mode
};

} // namespace

////////////////////////////////////////////////////////////
struct ThreadPool::Impl
{
    TaskQueue           queue;
    std::vector<Worker> workers;
    std::atomic<SizeT>  remainingInits;
};


////////////////////////////////////////////////////////////
ThreadPool::ThreadPool(const SizeT workerCount)
{
    for (SizeT i = 0u; i < workerCount; ++i)
        m_impl->workers.emplace_back(m_impl->queue);

    m_impl->remainingInits.store(workerCount, std::memory_order::relaxed);

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
    while (m_impl->remainingInits.load(std::memory_order::acquire) > 0u)
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

} // namespace sf::base
