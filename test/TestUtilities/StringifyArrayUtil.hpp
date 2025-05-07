#pragma once

#include "SFML/Base/Array.hpp"
#include "SFML/Base/SizeT.hpp"

#include <DoctestFwd.hpp>

namespace doctest
{

template <typename T, sf::base::SizeT N>
struct StringMaker<sf::base::Array<T, N>>
{
    static doctest::String convert(const sf::base::Array<T, N>&)
    {
        return ""; // TODO P3:
    }
};

} // namespace doctest
