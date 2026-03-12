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
inline constexpr priv::swap_adl::SwapFn genericSwap{};


////////////////////////////////////////////////////////////
template <typename ForwardIt1, typename ForwardIt2>
[[gnu::always_inline]] inline constexpr void iterSwap(const ForwardIt1 a, const ForwardIt2 b)
{
    genericSwap(*a, *b);
}


////////////////////////////////////////////////////////////
template <typename ForwardIt1, typename ForwardIt2>
[[gnu::always_inline]] inline constexpr ForwardIt2 swapRanges(ForwardIt1 first1, const ForwardIt1 last1, ForwardIt2 first2)
{
    for (; first1 != last1; ++first1, ++first2)
        iterSwap(first1, first2);

    return first2;
}

} // namespace sf::base
