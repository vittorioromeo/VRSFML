#pragma once

#include <doctest/parts/doctest_fwd.h>

#include <string>

namespace doctest
{

template <>
struct StringMaker<char8_t>
{
    static doctest::String convert(const char8_t&)
    {
        return ""; // TODO P2:
    }
};

template <>
struct StringMaker<char16_t>
{
    static doctest::String convert(const char16_t&)
    {
        return ""; // TODO P2:
    }
};

template <typename T>
struct StringMaker<std::basic_string<T>>
{
    static doctest::String convert(const std::basic_string<T>&)
    {
        return ""; // TODO P2:
    }
};

template <typename T>
struct StringMaker<std::basic_string_view<T>>
{
    static doctest::String convert(const std::basic_string_view<T>&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
