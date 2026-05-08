#pragma once

#include "SFML/Base/FwdStdString.hpp"

#include <DoctestFwd.hpp>

namespace doctest
{

template <>
struct StringMaker<char8_t>
{
    static doctest::String convert(const char8_t&)
    {
        return ""; // TODO P3:
    }
};

template <>
struct StringMaker<char16_t>
{
    static doctest::String convert(const char16_t&)
    {
        return ""; // TODO P3:
    }
};

template <typename T>
struct StringMaker<std::basic_string<T>>
{
    static doctest::String convert(const std::basic_string<T>& s)
    {
        return reinterpret_cast<const char*>(s.c_str()); // TODO P3:
    }
};

} // namespace doctest
