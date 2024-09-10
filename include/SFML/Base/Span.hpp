#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
struct [[nodiscard]] Span
{
    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span() = default;

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(const Span&) = default;

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(Span&&) = default;

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr Span& operator=(const Span&) = default;

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr Span& operator=(Span&&) = default;

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(decltype(nullptr), SizeT) = delete;

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(T* data, SizeT size) : m_data{data}, m_size{size}
    {
        SFML_BASE_ASSERT(data != nullptr || (data == nullptr && size == 0u));
    }

    //////////////////////////////////////////
    template <SizeT N>
    [[nodiscard, gnu::always_inline]] constexpr Span(T (&array)[N]) : m_data{array}, m_size{N}
    {
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T* data() const
    {
        return m_data;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr SizeT size() const
    {
        return m_size;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T* begin() const
    {
        return m_data;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T* end() const
    {
        return m_data + m_size;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T& operator[](SizeT i) const
    {
        SFML_BASE_ASSERT(i < m_size);
        return *(m_data + i);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr bool empty() const
    {
        return m_size == 0u;
    }

    T*    m_data{nullptr};
    SizeT m_size{0u};
};

} // namespace sf::base
