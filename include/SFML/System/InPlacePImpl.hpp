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

// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SFML_PRIV_LAUNDER_CAST(type, buffer) SFML_PRIV_LAUNDER(reinterpret_cast<type>(buffer))


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct MaxAlignTInPlacePImpl
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
    alignas(MaxAlignTInPlacePImpl) char m_buffer[BufferSize];

public:
    [[nodiscard, gnu::always_inline]] T* operator->() noexcept
    {
        return SFML_PRIV_LAUNDER_CAST(T*, m_buffer);
    }

    [[nodiscard, gnu::always_inline]] const T* operator->() const noexcept
    {
        return SFML_PRIV_LAUNDER_CAST(const T*, m_buffer);
    }

    [[nodiscard, gnu::always_inline]] T& operator*() noexcept
    {
        return *SFML_PRIV_LAUNDER_CAST(T*, m_buffer);
    }

    [[nodiscard, gnu::always_inline]] const T& operator*() const noexcept
    {
        return *SFML_PRIV_LAUNDER_CAST(const T*, m_buffer);
    }

    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] explicit InPlacePImpl(Args&&... args)
    {
        static_assert(sizeof(T) <= BufferSize);
        new (m_buffer) T(static_cast<Args&&>(args)...);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(const InPlacePImpl& rhs)
    {
        new (m_buffer) T(*SFML_PRIV_LAUNDER_CAST(const T*, rhs.m_buffer));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(InPlacePImpl&& rhs) noexcept
    {
        new (m_buffer) T(static_cast<T&&>(*SFML_PRIV_LAUNDER_CAST(T*, rhs.m_buffer)));
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
    [[gnu::always_inline]] InPlacePImpl& operator=(const InPlacePImpl& rhs)
    {
        // Rely on the inner type for self-assignment check.
        *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) = *SFML_PRIV_LAUNDER_CAST(const T*, rhs.m_buffer);
        return *this;
    }

    [[gnu::always_inline]] InPlacePImpl& operator=(InPlacePImpl&& rhs) noexcept
    {
        // Rely on the inner type for self-assignment check.
        *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) = static_cast<T&&>(*SFML_PRIV_LAUNDER_CAST(T*, rhs.m_buffer));
        return *this;
    }

    [[gnu::always_inline]] ~InPlacePImpl()
    {
        SFML_PRIV_LAUNDER_CAST(T*, m_buffer)->~T();
    }
};

} // namespace sf::priv

#undef SFML_PRIV_LAUNDER_CAST
#undef SFML_PRIV_LAUNDER
