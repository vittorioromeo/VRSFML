// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "Task.hpp"
#include "blockingconcurrentqueue.h"
#include "concurrentqueue.h"


namespace hg::ThreadPool
{
////////////////////////////////////////////////////////////
class TaskQueue : public moodycamel::BlockingConcurrentQueue<Task>
{
public:
    using moodycamel::BlockingConcurrentQueue<Task>::BlockingConcurrentQueue;
};

} // namespace hg::ThreadPool
