#pragma once

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>

namespace tst
{

template <typename T>
struct StringMaker<sf::base::Optional<T>>
{
    static sf::base::String convert(const sf::base::Optional<T>&)
    {
        return detail::defaultStringification();
    }
};

} // namespace tst
