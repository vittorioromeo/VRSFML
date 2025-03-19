#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
struct [[nodiscard]] Span
{
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span() = default;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(decltype(nullptr), SizeT) = delete;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(T* theData, SizeT theSize) : data{theData}, size{theSize}
    {
        SFML_BASE_ASSERT(data != nullptr || (data == nullptr && size == 0u));
    }


    ////////////////////////////////////////////////////////////
    template <SizeT N>
    [[nodiscard, gnu::always_inline]] constexpr Span(T (&array)[N]) : data{array}, size{N}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* begin() const
    {
        return data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* end() const
    {
        return data + size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& operator[](SizeT i) const
    {
        SFML_BASE_ASSERT(i < size);
        return *(data + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool empty() const
    {
        return size == 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const T* rhsData, SizeT rhsSize) const
    {
        if (size != rhsSize)
            return false;

        for (SizeT i = 0u; i < size; ++i)
            if (data[i] != rhsData[i])
                return false;

        return true;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const Span& rhs) const
    {
        return valueEquals(rhs.data, rhs.size);
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T*    data{nullptr};
    SizeT size{0u};
};

} // namespace sf::base
