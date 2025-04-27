#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Manages a pool of worker threads to execute tasks concurrently.
///
/// This class provides a simple way to offload work to a fixed number
/// of background threads. Tasks are submitted using the `post` method
/// and are executed by the next available worker thread.
///
////////////////////////////////////////////////////////////
class ThreadPool
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief The type of task that can be executed by the pool.
    ///
    /// Uses `sf::base::FixedFunction` for non-allocating storage.
    ///
    ////////////////////////////////////////////////////////////
    using Task = FixedFunction<void(), 128>;

    ////////////////////////////////////////////////////////////
    /// \brief Construct a thread pool with a specified number of workers.
    ///
    /// Creates and starts `workerCount` threads that will wait for tasks.
    ///
    /// \param workerCount The number of worker threads to create.
    ///
    ////////////////////////////////////////////////////////////
    explicit ThreadPool(SizeT workerCount);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor.
    ///
    /// Waits for all currently executing and pending tasks to complete,
    /// then stops and joins all worker threads.
    ///
    ////////////////////////////////////////////////////////////
    ~ThreadPool();

    ////////////////////////////////////////////////////////////
    /// \brief Post a task to be executed by a worker thread.
    ///
    /// The task will be added to the queue and executed by the next
    /// available worker thread.
    ///
    /// \param f The task (a callable object taking no arguments and returning void) to execute.
    ///
    ////////////////////////////////////////////////////////////
    void post(Task&& f);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SizeT getWorkerCount() const noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of concurrent threads supported by the hardware.
    ///
    /// Provides a hint about the optimal number of threads for CPU-bound tasks.
    /// Equivalent to `std::thread::hardware_concurrency`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SizeT getHardwareWorkerCount() noexcept;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    InPlacePImpl<Impl, 896> m_impl; //!< Implementation details
};

} // namespace sf::base
