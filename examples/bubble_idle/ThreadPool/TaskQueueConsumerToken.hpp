// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "concurrentqueue.h"


namespace hg::ThreadPool
{
////////////////////////////////////////////////////////////
class TaskQueueConsumerToken : public moodycamel::ConsumerToken
{
public:
    using moodycamel::ConsumerToken::ConsumerToken;
};

} // namespace hg::ThreadPool
