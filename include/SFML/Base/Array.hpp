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
template <typename T, SizeT N>
struct [[nodiscard]] Array
{
    ////////////////////////////////////////////////////////////
    static_assert(N > 0, "Zero-sized arrays are not supported");


    ////////////////////////////////////////////////////////////
    T elements[N];


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr SizeT size() noexcept
    {
        return N;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* data() noexcept
    {
        return elements;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* data() const noexcept
    {
        return elements;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T& operator[](const SizeT i) noexcept
    {
        SFML_BASE_ASSERT(i < N);
        return elements[i];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const T& operator[](const SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(i < N);
        return elements[i];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* begin() noexcept
    {
        return elements;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* begin() const noexcept
    {
        return elements;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* end() noexcept
    {
        return elements + N;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* end() const noexcept
    {
        return elements + N;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* cbegin() const noexcept
    {
        return elements;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* cend() const noexcept
    {
        return elements + N;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr bool operator==(const Array& rhs) const = default;
};


////////////////////////////////////////////////////////////
template <typename T, typename... Elements>
Array(T, Elements...) -> Array<T, 1 + sizeof...(Elements)>;

} // namespace sf::base
