#pragma once

#include "SFML/Base/FwdStdString.hpp"
#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>

namespace tst
{

template <>
struct StringMaker<char8_t>
{
    static sf::base::String convert(const char8_t&)
    {
        return detail::defaultStringification();
    }
};

template <>
struct StringMaker<char16_t>
{
    static sf::base::String convert(const char16_t&)
    {
        return detail::defaultStringification();
    }
};

template <typename T>
struct StringMaker<std::basic_string<T>>
{
    static sf::base::String convert(const std::basic_string<T>& s)
    {
        return sf::base::String{reinterpret_cast<const char*>(s.c_str())};
    }
};

} // namespace tst
