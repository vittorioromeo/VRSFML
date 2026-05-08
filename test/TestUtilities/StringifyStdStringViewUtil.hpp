#pragma once

#include <DoctestFwd.hpp>

#include <string_view>


namespace doctest
{

template <typename T>
struct StringMaker<std::basic_string_view<T>>
{
    static doctest::String convert(const std::basic_string_view<T>& s)
    {
        return reinterpret_cast<const char*>(s.data()); // TODO P3:
    }
};

} // namespace doctest
