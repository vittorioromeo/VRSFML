#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Swap.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Default less-than comparison function object.
///
/// Used by sorting algorithms when no custom comparator is provided.
///
////////////////////////////////////////////////////////////
inline constexpr auto cmpLess = [](const auto& a, const auto& b) { return a < b; };

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Sorts a range of elements using the insertion sort algorithm.
///
/// Insertion sort is efficient for small collections or nearly-sorted data.
/// It is used internally by `quickSort` for small partitions.
///
/// \tparam RandomIt The type of the iterators, must meet the requirements of a random access iterator.
///
/// \param first Iterator to the first element of the range to sort.
/// \param last  Iterator to the element following the last element of the range to sort.
/// \param comp  A binary predicate that returns `true` if the first argument should be ordered before the second.
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
constexpr void insertionSort(const RandomIt first, const RandomIt last, auto&& comp)
{
    if (first >= last)
        return;

    for (auto i = first + 1; i < last; ++i)
    {
        auto key = SFML_BASE_MOVE(*i);
        auto j   = i;

        while (j > first && comp(key, *(j - 1)))
        {
            *j = SFML_BASE_MOVE(*(j - 1));
            --j;
        }

        *j = SFML_BASE_MOVE(key);
    }
}


////////////////////////////////////////////////////////////
/// \brief Sorts a range of elements using the insertion sort algorithm with `operator<`.
///
/// This is an overload of `insertionSort` that uses the default less-than
/// operator for comparison.
///
/// \tparam RandomIt The type of the iterators, must meet the requirements of a random access iterator.
///
/// \param first Iterator to the first element of the range to sort.
/// \param last  Iterator to the element following the last element of the range to sort.
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
[[gnu::always_inline]] inline constexpr void insertionSort(const RandomIt first, const RandomIt last)
{
    insertionSort(first, last, priv::cmpLess);
}

} // namespace sf::base


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Internal implementation of the introspective quicksort algorithm.
///
/// This function implements a hybrid sorting strategy:
/// 1.  It uses median-of-three pivot selection to improve pivot choice and avoid worst-case behavior.
/// 2.  It employs a Hoare-like partitioning scheme.
/// 3.  It switches to `insertionSort` for small partitions, which is more efficient for small data sets.
/// 4.  It uses an explicit loop with tail-call optimization to recurse on the smaller partition,
///     limiting recursion depth to O(log n) and preventing stack overflow.
///
/// \tparam RandomIt The type of the iterators.
///
/// \param first Iterator to the first element of the range.
/// \param last  Iterator to the element following the last element of the range.
/// \param comp  The comparison function object.
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
void quickSortImpl(RandomIt first, RandomIt last, auto&& comp)
{
    enum : int
    {
        insertionSortCutoffThreshold = 16
    };

    while (last - first > insertionSortCutoffThreshold)
    {
        // --- Median-of-three pivot selection ---
        // 1. Choose three elements: first, middle, and last.
        const RandomIt mid = first + (last - first) / 2;

        // 2. Sort these three elements to find the median.
        if (comp(*mid, *first))
            iterSwap(first, mid);

        if (comp(*(last - 1), *first))
            iterSwap(first, last - 1);

        if (comp(*(last - 1), *mid))
            iterSwap(mid, last - 1);

        // 3. The median is now at `mid`. Place it just before the end
        //    to act as the pivot. The element at `last-1` is now a sentinel,
        //    guaranteed to be >= the pivot.
        iterSwap(mid, last - 2);
        RandomIt pivot = last - 2;

        // --- Hoare-like Partitioning ---
        RandomIt i = first;
        RandomIt j = last - 2;

        while (true)
        {
            // The elements at `first` and `last-1` act as sentinels,
            // so we don't need boundary checks inside the loops.
            while (comp(*++i, *pivot))
                ;

            while (comp(*pivot, *(--j)))
                ;

            if (i >= j)
                break;

            iterSwap(i, j);
        }

        // Restore the pivot to its final sorted position.
        iterSwap(i, pivot);

        // --- Tail-call optimization ---
        // Recurse on the smaller partition and loop on the larger one
        // to limit recursion depth to O(log n).
        if ((i - first) < (last - (i + 1)))
        {
            quickSortImpl(first, i, comp);
            first = i + 1; // loop on the larger right part
        }
        else
        {
            quickSortImpl(i + 1, last, comp);
            last = i; // loop on the larger left part
        }
    }

    // For the final small partitions, run insertion sort.
    insertionSort(first, last, comp);
}

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Sorts a range of elements into non-descending order using a hybrid quicksort.
///
/// This algorithm is not stable, meaning the relative order of equal elements
/// is not guaranteed to be preserved. It has an average time complexity of O(n log n).
///
/// \tparam RandomIt The type of the iterators, must meet the requirements of a random access iterator.
///
/// \param first Iterator to the first element of the range to sort.
/// \param last  Iterator to the element following the last element of the range to sort.
/// \param comp  A binary predicate that returns `true` if the first argument should be ordered before the second.
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
[[gnu::always_inline]] inline void quickSort(const RandomIt first, const RandomIt last, auto&& comp)
{
    if (first >= last)
        return;

    priv::quickSortImpl(first, last, comp);
}


////////////////////////////////////////////////////////////
/// \brief Sorts a range of elements into non-descending order using `operator<`.
///
/// This is an overload of `quickSort` that uses the default less-than
/// operator for comparison.
///
/// \tparam RandomIt The type of the iterators, must meet the requirements of a random access iterator.
///
/// \param first Iterator to the first element of the range to sort.
/// \param last  Iterator to the element following the last element of the range to sort.
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
[[gnu::always_inline]] inline void quickSort(const RandomIt first, const RandomIt last)
{
    quickSort(first, last, priv::cmpLess);
}

} // namespace sf::base
