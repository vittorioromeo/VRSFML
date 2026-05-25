#pragma once

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <DoctestFwd.hpp>


namespace tst
{

template <typename T>
struct StringMaker<sf::base::Vector<T>>
{
    static sf::base::String convert(const sf::base::Vector<T>&)
    {
        return detail::defaultStringification();
    }
};

} // namespace tst
