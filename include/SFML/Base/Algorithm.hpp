#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Copy elements from one range to another
///
/// \param rangeBegin Iterator to the beginning of the source range
/// \param rangeEnd   Iterator to the end of the source range
/// \param targetIter Iterator to the beginning of the destination range
///
/// \return Iterator to the element past the last element copied
///
////////////////////////////////////////////////////////////
template <typename Iter, typename TargetIter>
[[gnu::always_inline]] constexpr TargetIter copy(Iter rangeBegin, Iter rangeEnd, TargetIter targetIter)
{
    while (rangeBegin != rangeEnd)
        *targetIter++ = *rangeBegin++;

    return targetIter;
}


////////////////////////////////////////////////////////////
/// \brief Find the first element equal to a target value in a range
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param target     Value to search for
///
/// \return Iterator to the first element equal to target, or `rangeEnd` if not found
///
////////////////////////////////////////////////////////////
template <typename Iter, typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Iter find(Iter rangeBegin, Iter rangeEnd, const T& target) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (*rangeBegin == target)
            return rangeBegin;

    return rangeEnd;
}


////////////////////////////////////////////////////////////
/// \brief Find the first element satisfying a predicate in a range
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return Iterator to the first element for which predicate returns true, or `rangeEnd` if not found
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Iter findIf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            return rangeBegin;

    return rangeEnd;
}


////////////////////////////////////////////////////////////
/// \brief Check if any element in a range satisfies a predicate
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return `true` if predicate returns true for at least one element, `false` otherwise
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool anyOf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            return true;

    return false;
}


////////////////////////////////////////////////////////////
/// \brief Count the number of elements in a range that evaluate to true
///
/// Iterates through the range and increments a counter for each element
/// that, when converted to bool, is true.
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
///
/// \return The number of elements evaluating to true
///
////////////////////////////////////////////////////////////
template <typename Iter>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT countTruthy(Iter rangeBegin, Iter rangeEnd)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (static_cast<bool>(*rangeBegin))
            ++result;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Count the number of elements in a range equal to a specific value
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param value      Value to compare elements against
///
/// \return The number of elements equal to value
///
////////////////////////////////////////////////////////////
template <typename Iter, typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT count(Iter rangeBegin, Iter rangeEnd, const T& value)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (*rangeBegin == value)
            ++result;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Count the number of elements in a range satisfying a predicate
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return The number of elements for which predicate returns true
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT countIf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            ++result;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Check if all elements in a range satisfy a predicate
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return `true` if predicate returns true for all elements, `false` otherwise
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool allOf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (!predicate(*rangeBegin))
            return false;

    return true;
}


////////////////////////////////////////////////////////////
/// \brief Get the size of a C-style array at compile time
///
/// \param Array reference
///
/// \return Size of the array (number of elements)
///
////////////////////////////////////////////////////////////
template <typename T, auto N>
[[nodiscard, gnu::always_inline, gnu::const]] consteval SizeT getArraySize(const T (&)[N]) noexcept
{
    return N;
}


////////////////////////////////////////////////////////////
/// \brief Get the size of a C-style array member at compile time
///
/// \param Pointer to array member
///
/// \return Size of the array member (number of elements)
///
////////////////////////////////////////////////////////////
template <typename S, typename T, auto N>
[[nodiscard, gnu::always_inline, gnu::const]] consteval auto getArraySize(const T (S::*)[N]) noexcept
{
    return N;
}


////////////////////////////////////////////////////////////
/// \brief Output iterator that inserts elements using `push_back`
///
/// Acts like `std::back_insert_iterator`.
///
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
        if constexpr (requires { m_container->push_back(value); })
        {
            m_container->push_back(value);
        }
        else
        {
            m_container->pushBack(value);
        }

        return *this;
    }

    [[gnu::always_inline]] BackInserter& operator=(value_type&& value)
    {
        if constexpr (requires { m_container->push_back(static_cast<value_type &&>(value)); })
        {
            m_container->push_back(static_cast<value_type&&>(value));
        }
        else
        {
            m_container->pushBack(static_cast<value_type&&>(value));
        }

        return *this;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] BackInserter& operator*() noexcept
    {
        return *this;
    }

    [[gnu::always_inline, gnu::pure]] BackInserter& operator++() noexcept
    {
        return *this;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] BackInserter operator++(int) noexcept
    {
        return *this;
    }
};


////////////////////////////////////////////////////////////
/// \brief Exchange the value of an object with a new value
///
/// Assigns `newVal` to `obj` and returns the old value of `obj`.
/// Equivalent to `std::exchange`.
///
/// \param obj    Object whose value to exchange
/// \param newVal New value to assign to `obj`
///
/// \return The old value of `obj`
///
////////////////////////////////////////////////////////////
template <typename T, typename U = T>
[[nodiscard, gnu::always_inline]] inline constexpr T exchange(T& obj, U&& newVal)
{
    T oldVal = SFML_BASE_MOVE(obj);
    obj      = SFML_BASE_FORWARD(newVal);
    return oldVal;
}


////////////////////////////////////////////////////////////
/// \brief Remove elements satisfying a predicate from a range
///
/// Moves elements for which the predicate is false to the beginning
/// of the range `[first, last)`. Returns an iterator to the new logical
/// end of the range. The elements after the returned iterator are in
/// a valid but unspecified state.
/// Equivalent to `std::remove_if`.
///
/// \param first     Iterator to the beginning of the range
/// \param last      Iterator to the end of the range
/// \param predicate Unary predicate function
///
/// \return Iterator to the new logical end of the range
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline]] inline constexpr Iter removeIf(Iter first, Iter last, Predicate&& predicate)
{
    first = findIf(first, last, predicate);

    if (first != last)
        for (Iter i = first; ++i != last;)
            if (!predicate(*i))
                *first++ = SFML_BASE_MOVE(*i);

    return first;
}


////////////////////////////////////////////////////////////
/// \brief Erase elements satisfying a predicate from a vector
///
/// Removes all elements from the vector for which the predicate
/// returns `true`. This function modifies the vector in-place.
/// Equivalent to `std::erase_if` for vectors.
///
/// \param vector    Vector to modify
/// \param predicate Unary predicate function
///
/// \return The number of elements removed
///
////////////////////////////////////////////////////////////
template <typename Vector, typename Predicate>
[[gnu::always_inline]] inline constexpr SizeT vectorEraseIf(Vector& vector, Predicate&& predicate)
{
    const auto it       = removeIf(vector.begin(), vector.end(), predicate);
    const auto nRemoved = static_cast<SizeT>(vector.end() - it);

    vector.erase(it, vector.end());
    return nRemoved;
}

// TODO P0: docs, maybe replace existing uses of erase if with this
template <typename Vector, typename Predicate>
[[gnu::always_inline]] inline constexpr SizeT vectorSwapAndPopIf(Vector& vector, Predicate&& predicate)
{
    SizeT nRemoved = 0u;

    for (SizeT i = 0u; i < vector.size();)
    {
        if (predicate(vector[i]))
        {
            vector[i] = SFML_BASE_MOVE(vector.back());

            if constexpr (requires { vector.popBack(); })
            {
                vector.popBack();
            }
            else
            {
                vector.pop_back();
            }

            ++nRemoved;
        }
        else
        {
            i++;
        }
    }

    return nRemoved;
}

////////////////////////////////////////////////////////////
/// \brief Check if a range is sorted according to a comparison function
///
/// \param first Iterator to the beginning of the range
/// \param last  Iterator to the end of the range
/// \param comp  Binary comparison function object
///
/// \return `true` if the range is sorted, `false` otherwise
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Comparer>
[[nodiscard]] constexpr bool isSorted(Iter first, Iter last, Comparer comp)
{
    if (first != last)
    {
        Iter next = first;
        while (++next != last)
        {
            if (comp(*next, *first))
                return false;

            first = next;
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
/// \brief Removes all except the first element from every consecutive group of equivalent elements
///
/// Removing is done by shifting the elements in the range in
/// such a way that the elements that are not to be removed appear
/// in the beginning of the range.
///
/// \param first Iterator to the beginning of the range
/// \param last  Iterator to the end of the range
///
/// \return An iterator to the new logical end of the range
///
////////////////////////////////////////////////////////////
template <class ForwardIt>
ForwardIt unique(ForwardIt first, ForwardIt last)
{
    if (first == last)
        return last;

    ForwardIt result = first;

    while (++first != last)
        if (!(*result == *first) && ++result != first) // NOLINT(bugprone-inc-dec-in-conditions)
            *result = SFML_BASE_MOVE(*first);

    return ++result;
}

} // namespace sf::base
