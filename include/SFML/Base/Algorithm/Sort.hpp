#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Swap.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Insertion sort using `comp` as a strict-weak less-than
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
inline constexpr void insertionSort(const RandomIt first, const RandomIt last, auto&& comp)
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
/// \brief `insertionSort` overload using `operator<`
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
[[gnu::always_inline]] inline constexpr void insertionSort(const RandomIt first, const RandomIt last)
{
    insertionSort(first, last, [](const auto& a, const auto& b) { return a < b; });
}

} // namespace sf::base


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Hybrid quicksort: median-of-three pivot, Hoare partitioning, insertion sort for small
///        partitions, and tail-recursion on the larger side to keep stack depth at `O(log n)`
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
inline constexpr void quickSortImpl(RandomIt first, RandomIt last, auto&& comp)
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
/// \brief Sort `[first, last)` in non-descending order under `comp` (unstable, hybrid quicksort)
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
[[gnu::always_inline]] inline constexpr void quickSort(const RandomIt first, const RandomIt last, auto&& comp)
{
    if (first >= last)
        return;

    priv::quickSortImpl(first, last, comp);
}


////////////////////////////////////////////////////////////
/// \brief `quickSort` overload using `operator<`
///
////////////////////////////////////////////////////////////
template <typename RandomIt>
[[gnu::always_inline]] inline constexpr void quickSort(const RandomIt first, const RandomIt last)
{
    quickSort(first, last, [](const auto& a, const auto& b) { return a < b; });
}

} // namespace sf::base
