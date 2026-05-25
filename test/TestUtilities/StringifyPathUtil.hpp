#pragma once

#include "SFML/System/Path.hpp"

#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>

namespace tst
{

template <>
struct StringMaker<sf::Path>
{
    static sf::base::String convert(const sf::Path&)
    {
        return detail::defaultStringification();
    }
};

} // namespace tst
