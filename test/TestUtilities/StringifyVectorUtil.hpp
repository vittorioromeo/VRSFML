#pragma once

#include "SFML/Base/Vector.hpp"

#include <DoctestFwd.hpp>

#include <vector>


namespace doctest
{

template <typename T>
struct StringMaker<std::vector<T>>
{
    static doctest::String convert(const std::vector<T>&)
    {
        return ""; // TODO P3:
    }
};

template <typename T>
struct StringMaker<sf::base::Vector<T>>
{
    static doctest::String convert(const sf::base::Vector<T>&)
    {
        return ""; // TODO P3:
    }
};

} // namespace doctest
