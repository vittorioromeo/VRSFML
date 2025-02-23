// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/FixedFunction.hpp"


namespace hg::ThreadPool
{
////////////////////////////////////////////////////////////
class Task : public sf::base::FixedFunction<void(), 128>
{
public:
    using sf::base::FixedFunction<void(), 128>::FixedFunction;
};

} // namespace hg::ThreadPool
