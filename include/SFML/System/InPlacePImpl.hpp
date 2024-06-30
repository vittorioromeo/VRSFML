// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#if __has_builtin(__builtin_launder)
#define SFML_PRIV_LAUNDER __builtin_launder
#else
#include <new>
#define SFML_PRIV_LAUNDER ::std::launder
#endif


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct MaxAlignT
{
    alignas(alignof(long long)) long long a;
    alignas(alignof(long double)) long double b;
};

////////////////////////////////////////////////////////////
template <typename T, decltype(sizeof(int)) BufferSize>
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class InPlacePImpl
{
private:
    alignas(MaxAlignT) char m_buffer[BufferSize];

    template <typename U>
    [[nodiscard, gnu::always_inline]] U& as() noexcept
    {
        return *SFML_PRIV_LAUNDER(reinterpret_cast<U*>(m_buffer));
    }

    template <typename U>
    [[nodiscard, gnu::always_inline]] const U& as() const noexcept
    {
        return *SFML_PRIV_LAUNDER(reinterpret_cast<U*>(m_buffer));
    }

public:
    [[nodiscard, gnu::always_inline]] T* operator->() noexcept
    {
        return &as<T>();
    }

    [[nodiscard, gnu::always_inline]] const T* operator->() const noexcept
    {
        return &as<const T>();
    }

    [[nodiscard, gnu::always_inline]] T& operator*() noexcept
    {
        return as<T>();
    }

    [[nodiscard, gnu::always_inline]] const T& operator*() const noexcept
    {
        return as<const T>();
    }

    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] explicit InPlacePImpl(Args&&... args)
    {
        static_assert(sizeof(T) <= BufferSize);
        static_assert(alignof(T) <= alignof(MaxAlignT));

        new (m_buffer) T(static_cast<Args&&>(args)...);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(const InPlacePImpl& rhs)
    {
        new (m_buffer) T(rhs.as<const T>());
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(InPlacePImpl&& rhs) noexcept
    {
        new (m_buffer) T(static_cast<T&&>(rhs.as<T>()));
    }

    [[gnu::always_inline]] InPlacePImpl& operator=(const InPlacePImpl& rhs)
    {
        if (&rhs != this)
            as<T>() = rhs.as<const T>();

        return *this;
    }

    [[gnu::always_inline]] InPlacePImpl& operator=(InPlacePImpl&& rhs) noexcept
    {
        if (&rhs != this)
            as<T>() = static_cast<T&&>(rhs.as<T>());

        return *this;
    }

    [[gnu::always_inline]] ~InPlacePImpl()
    {
        as<T>().~T();
    }
};

} // namespace sf::priv

#undef SFML_PRIV_LAUNDER
