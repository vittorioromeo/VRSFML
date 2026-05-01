#pragma once

#include "SFML/Base/Vector.hpp"

#include <DoctestFwd.hpp>


namespace doctest
{

template <typename T>
struct StringMaker<sf::base::Vector<T>>
{
    static doctest::String convert(const sf::base::Vector<T>&)
    {
        return ""; // TODO P3:
    }
};

} // namespace doctest
