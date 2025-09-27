#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
struct Span
{
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span() = default;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(T* data, SizeT size) : theData{data}, theSize{size}
    {
        SFML_BASE_ASSERT(theData != nullptr || (theData == nullptr && theSize == 0u));
    }


    ////////////////////////////////////////////////////////////
    template <SizeT N>
    [[nodiscard, gnu::always_inline]] constexpr Span(T (&array)[N]) : theData{array}, theSize{N}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] constexpr operator Span<const T>() const
    {
        return Span<const T>{theData, theSize};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* begin() const
    {
        return theData;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* end() const
    {
        return theData + theSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& operator[](SizeT i) const
    {
        SFML_BASE_ASSERT(i < theSize);
        return *(theData + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* data() const noexcept
    {
        return theData;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT size() const noexcept
    {
        return theSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool empty() const
    {
        return theSize == 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const T* rhsData, SizeT rhsSize) const
    {
        if (theSize != rhsSize)
            return false;

        for (SizeT i = 0u; i < theSize; ++i)
            if (theData[i] != rhsData[i])
                return false;

        return true;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const Span& rhs) const
    {
        return valueEquals(rhs.theData, rhs.theSize);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend void swap(Span& lhs, Span& rhs) noexcept
    {
        const auto swapImpl = []<typename U>(U& a, U& b)
        {
            U tempA = a;
            a       = b;
            b       = tempA;
        };

        swapImpl(lhs.theData, rhs.theData);
        swapImpl(lhs.theSize, rhs.theSize);
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T*    theData{nullptr};
    SizeT theSize{0u};
};

} // namespace sf::base
