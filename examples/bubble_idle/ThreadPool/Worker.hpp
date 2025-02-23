// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"


namespace hg::ThreadPool
{
class TaskQueue;

////////////////////////////////////////////////////////////
/// \brief Wraps an `std::thread`, `ConsumerQueuePtr` and atomic control
/// variables.
class [[nodiscard]] Worker
{
private:
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] State : sf::base::U8
    {
        // Initial state of the worker.
        Uninitialized = 0u,

        // The worker is dequeuing and accepting tasks in blocking mode.
        Running = 1u,

        // The worker is dequeuing and accepting tasks in non-blocking mode.
        // Will transition to `State::Finished` automatically when there are no
        // more tasks.
        Stopped = 2u,
    };

    ////////////////////////////////////////////////////////////
    struct Impl;
    sf::base::InPlacePImpl<Impl, 64> m_impl; //!< Implementation details

public:
    ////////////////////////////////////////////////////////////
    Worker(TaskQueue& queue) noexcept;
    ~Worker();

    ////////////////////////////////////////////////////////////
    Worker(Worker&&) noexcept;
    Worker& operator=(Worker&&) noexcept;

    void start(void* remainingInitsPtr); // points to atomic uint

    /// \brief Sets the running flag to false, preventing the worker to
    /// accept tasks.
    void stop() noexcept;

    /// \brief Assuming the worker is not running, tries to join the
    /// underlying thread.
    void join() noexcept;

    /// \brief Return `true` if the worker has finished processing tasks in a
    /// blocking manner.
    [[nodiscard]] bool isDoneBlockingProcessing() const noexcept;
};

} // namespace hg::ThreadPool
