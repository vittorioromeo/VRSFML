#pragma once

#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>

#include <string_view>


namespace tst
{

template <typename T>
struct StringMaker<std::basic_string_view<T>>
{
    static sf::base::String convert(const std::basic_string_view<T>& s)
    {
        return sf::base::String{reinterpret_cast<const char*>(s.data())};
    }
};

} // namespace tst
