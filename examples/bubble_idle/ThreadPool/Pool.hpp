// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "Task.hpp"

#include "SFML/Base/InPlacePImpl.hpp"


namespace hg::ThreadPool
{
class Worker;

class Pool
{
private:
    struct Impl;
    sf::base::InPlacePImpl<Impl, 896> m_impl;

    /// \brief  Returns `true` if all workers have finished processing packets
    /// in a blocking manner.
    [[nodiscard]] bool areAllWorkersDoneBlockingProcessing() const noexcept;

public:
    explicit Pool(unsigned int workerCount);
    ~Pool();

    void post(Task&& f);

    [[nodiscard]] unsigned int getWorkerCount() const noexcept;
};

} // namespace hg::ThreadPool
