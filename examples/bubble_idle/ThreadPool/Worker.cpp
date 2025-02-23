// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "Worker.hpp"

#include "ConsumerQueuePtr.hpp"
#include "Task.hpp"
#include "TaskQueue.hpp"

#include "SFML/Base/Assert.hpp"

#include <atomic>
#include <thread>


namespace
{
////////////////////////////////////////////////////////////
/// \brief Wrapper around `std::atomic<T>` that allows move operations.
///
/// \details Moves are handled by using `store(rhs.load())`.
///
////////////////////////////////////////////////////////////
template <typename T>
struct [[nodiscard]] MovableAtomic : std::atomic<T>
{
    using std::atomic<T>::atomic;

    [[nodiscard]] MovableAtomic() = default;

    MovableAtomic(const MovableAtomic&)            = delete;
    MovableAtomic& operator=(const MovableAtomic&) = delete;

    MovableAtomic(MovableAtomic&& rhs) noexcept : std::atomic<T>{rhs.load()}
    {
    }

    MovableAtomic& operator=(MovableAtomic&& rhs) noexcept
    {
        this->store(rhs.load());
        return *this;
    }
};

} // namespace


namespace hg::ThreadPool
{
////////////////////////////////////////////////////////////
struct Worker::Impl
{
    /// \brief Worker thread.
    std::thread thread;

    /// \brief Pointer to queue + consumer token.
    ConsumerQueuePtr queue;

    /// \brief State of the worker, controlled both by the pool and internally.
    MovableAtomic<State> state;

    /// \brief Signals when the worker is done processing tasks in blocking
    /// mode. Controlled internally, checked by the pool to start posting dummy
    /// tasks.
    MovableAtomic<bool> doneBlockingProcessing;

    ////////////////////////////////////////////////////////////
    explicit Impl(TaskQueue& q) noexcept : queue{q}, state{State::Uninitialized}, doneBlockingProcessing{false}
    {
    }
};


////////////////////////////////////////////////////////////
Worker::Worker(TaskQueue& queue) noexcept : m_impl{queue}
{
}


////////////////////////////////////////////////////////////
Worker::~Worker() = default;


////////////////////////////////////////////////////////////
Worker::Worker(Worker&&) noexcept            = default;
Worker& Worker::operator=(Worker&&) noexcept = default;


////////////////////////////////////////////////////////////
void Worker::start(void* remainingInitsPtr)
{
    SFML_BASE_ASSERT(m_impl->state.load() == State::Uninitialized);

    m_impl->thread = std::thread([this, remainingInitsPtr]
    {
        // Set the running flag and signal to the pool that we are initialized.
        m_impl->state.store(State::Running);
        static_cast<std::atomic<unsigned int>*>(remainingInitsPtr)->fetch_sub(1u);

        Task taskBuffer;

        while (m_impl->state.load() == State::Running)
        {
            m_impl->queue->wait_dequeue(m_impl->queue.ctok(), taskBuffer); // Blocking
            taskBuffer();
        }

        // Signal the thread pool to send dummy final tasks.
        SFML_BASE_ASSERT(m_impl->state.load() == State::Stopped);
        m_impl->doneBlockingProcessing.store(true);

        while (m_impl->state.load() == State::Stopped)
        {
            if (!m_impl->queue->try_dequeue(m_impl->queue.ctok(), taskBuffer)) // Non-blocking
                break;                                                         // No more tasks available

            taskBuffer();
        }
    });
}


////////////////////////////////////////////////////////////
void Worker::stop() noexcept
{
    SFML_BASE_ASSERT(m_impl->state.load() == State::Running);

    m_impl->state.store(State::Stopped);
}


////////////////////////////////////////////////////////////
void Worker::join() noexcept
{
    SFML_BASE_ASSERT(m_impl->thread.joinable());
    SFML_BASE_ASSERT(m_impl->state.load() == State::Stopped);

    m_impl->thread.join();
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Worker::isDoneBlockingProcessing() const noexcept
{
    return m_impl->doneBlockingProcessing.load();
}

} // namespace hg::ThreadPool
