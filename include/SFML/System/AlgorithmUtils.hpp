////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Macros.hpp>
#include <SFML/System/SizeT.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline]] constexpr const T& min(const T& a, const T& b) noexcept
{
    return a < b ? a : b;
}


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline]] constexpr const T& max(const T& a, const T& b) noexcept
{
    return a < b ? b : a;
}


////////////////////////////////////////////////////////////
template <typename Iter, typename TargetIter>
[[gnu::always_inline]] constexpr TargetIter copy(Iter rangeBegin, Iter rangeEnd, TargetIter targetIter)
{
    while (rangeBegin != rangeEnd)
        *targetIter++ = *rangeBegin++;

    return targetIter;
}


////////////////////////////////////////////////////////////
template <typename T, typename Vector>
[[gnu::always_inline]] inline void appendRangeIntoVector(const T* rangeBegin, const T* rangeEnd, Vector& target)
{
    target.reserve(target.size() + static_cast<SizeT>(rangeEnd - rangeBegin));

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        target.push_back(*rangeBegin);
}


////////////////////////////////////////////////////////////
template <typename Iter, typename T>
[[gnu::always_inline]] constexpr Iter find(Iter rangeBegin, Iter rangeEnd, const T& target)
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (*rangeBegin == target)
            return rangeBegin;

    return rangeEnd;
}


////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[gnu::always_inline]] constexpr Iter findIf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate)
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            return rangeBegin;

    return rangeEnd;
}


////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[gnu::always_inline]] constexpr bool anyOf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate)
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            return true;

    return false;
}


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline]] constexpr const T& clamp(const T& value, const T& minValue, const T& maxValue)
{
    if (value < minValue)
        return minValue;

    if (value > maxValue)
        return maxValue;

    return value;
}


////////////////////////////////////////////////////////////
template <typename T, SizeT N>
[[nodiscard, gnu::always_inline]] constexpr SizeT getArraySize(const T (&)[N])
{
    return N;
}


////////////////////////////////////////////////////////////
template <typename T>
class BackInserter
{
private:
    T* m_container;

public:
    using container_type = T;
    using value_type     = T::value_type;

    [[nodiscard, gnu::always_inline]] explicit BackInserter(T& container) noexcept : m_container(&container)
    {
    }

    [[gnu::always_inline]] BackInserter& operator=(const value_type& value)
    {
        m_container->push_back(value);
        return *this;
    }

    [[gnu::always_inline]] BackInserter& operator=(value_type&& value)
    {
        m_container->push_back(static_cast<value_type&&>(value));
        return *this;
    }

    [[nodiscard, gnu::always_inline]] BackInserter& operator*()
    {
        return *this;
    }

    [[gnu::always_inline]] BackInserter& operator++()
    {
        return *this;
    }

    [[nodiscard, gnu::always_inline]] BackInserter operator++(int)
    {
        return *this;
    }
};


////////////////////////////////////////////////////////////
template <typename T, typename U = T>
[[nodiscard, gnu::always_inline]] inline constexpr T exchange(T& obj, U&& newVal)
{
    T oldVal = SFML_MOVE(obj);
    obj      = SFML_FORWARD(newVal);
    return oldVal;
}

} // namespace sf::priv
