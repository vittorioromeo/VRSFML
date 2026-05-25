#pragma once

#include "SFML/Base/Array.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>

namespace tst
{

template <typename T, sf::base::SizeT N>
struct StringMaker<sf::base::Array<T, N>>
{
    static sf::base::String convert(const sf::base::Array<T, N>&)
    {
        return detail::defaultStringification();
    }
};

} // namespace tst
