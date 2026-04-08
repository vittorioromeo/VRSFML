#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base::priv::swap_adl
{
////////////////////////////////////////////////////////////
// A deleted generic template. If ADL finds a generic `std::swap`, it will
// collide with this and cause an ambiguity.
template <typename T>
void swap(T&, T&) = delete;


////////////////////////////////////////////////////////////
// Niebloid (Customization Point Object)
struct SwapFn
{
    ////////////////////////////////////////////////////////////
    using SizeT = decltype(sizeof(0));


    ////////////////////////////////////////////////////////////
    template <typename T, SizeT N>
    [[gnu::always_inline]] constexpr void operator()(T (&a)[N], T (&b)[N]) const noexcept
    {
        for (SizeT i = 0; i < N; ++i)
            (*this)(a[i], b[i]);
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[gnu::always_inline]] constexpr void operator()(T& a, T& b) const noexcept
    {
        if constexpr (requires { a.swap(b); })
        {
            // Highest priority: explicit member function exists
            a.swap(b);
        }
        else if constexpr (requires { swap(a, b); }) // Fails in case of ambiguity too
        {
            // A specialized ADL swap exists and is unambiguous
            swap(a, b);
        }
        else
        {
            // No valid specialized ADL swap was found, or an ambiguity occurred
            T tempA = static_cast<T&&>(a);
            a       = static_cast<T&&>(b);
            b       = static_cast<T&&>(tempA);
        }
    }
};

} // namespace sf::base::priv::swap_adl


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Customization point object that swaps two values
///
/// Behaves like `std::ranges::swap`: prefers a member `swap`, then a
/// well-formed unqualified `swap` found via ADL, and finally falls
/// back to a manual three-step move-swap. Calling
/// `genericSwap(a, b)` is therefore the recommended way to swap
/// arbitrary objects in code that wants to honor user-provided
/// `swap` overloads without needing to write the customary
/// `using std::swap; swap(a, b);` dance.
///
////////////////////////////////////////////////////////////
inline constexpr priv::swap_adl::SwapFn genericSwap{};


////////////////////////////////////////////////////////////
/// \brief Swap the elements pointed to by two iterators
///
////////////////////////////////////////////////////////////
template <typename ForwardIt1, typename ForwardIt2>
[[gnu::always_inline]] inline constexpr void iterSwap(const ForwardIt1 a, const ForwardIt2 b)
{
    genericSwap(*a, *b);
}


////////////////////////////////////////////////////////////
/// \brief Pairwise swap two ranges of equal length
///
/// Iterates over `[first1, last1)` and `[first2, ...)`, swapping
/// corresponding elements with `iterSwap`.
///
/// \return Iterator to the element past the last element of the second range
///
////////////////////////////////////////////////////////////
template <typename ForwardIt1, typename ForwardIt2>
[[gnu::always_inline]] inline constexpr ForwardIt2 swapRanges(ForwardIt1 first1, const ForwardIt1 last1, ForwardIt2 first2)
{
    for (; first1 != last1; ++first1, ++first2)
        iterSwap(first1, first2);

    return first2;
}

} // namespace sf::base
