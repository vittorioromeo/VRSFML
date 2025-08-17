#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if !defined(_LIBCPP_INITIALIZER_LIST) && !defined(_INITIALIZER_LIST) && !defined(_INITIALIZER_LIST_)

    #define _LIBCPP_INITIALIZER_LIST // libcpp
    #define _INITIALIZER_LIST        // libstdc++
    #define _INITIALIZER_LIST_       // msstl

namespace std
{
////////////////////////////////////////////////////////////
/// \brief Lightweight alternative to the standard initializer list
///
////////////////////////////////////////////////////////////
template <typename T>
// NOLINTNEXTLINE(readability-identifier-naming)
class initializer_list
{
public:
    ////////////////////////////////////////////////////////////
    using value_type      = T;
    using reference       = const T&;
    using const_reference = const T&;
    using size_type       = decltype(sizeof(0));
    using iterator        = const T*;
    using const_iterator  = const T*;


    ////////////////////////////////////////////////////////////
    constexpr initializer_list() noexcept = default;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr initializer_list(const T* b, const size_type s) noexcept :
        m_begin(b),
        m_size(s)
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* begin() const noexcept
    {
        return m_begin;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* end() const noexcept
    {
        return m_begin + m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr size_type size() const noexcept
    {
        return m_size;
    }

private:
    const T*  m_begin = nullptr;
    size_type m_size  = 0u;
};


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* begin(initializer_list<T> il) noexcept
{
    return il.begin();
}


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* end(initializer_list<T> il) noexcept
{
    return il.end();
}

} // namespace std

#endif
