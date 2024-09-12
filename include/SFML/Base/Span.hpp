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
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span() = default;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(const Span&) = default;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(Span&&) = default;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Span& operator=(const Span&) = default;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Span& operator=(Span&&) = default;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(decltype(nullptr), SizeT) = delete;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(T* data, SizeT size) : m_data{data}, m_size{size}
    {
        SFML_BASE_ASSERT(data != nullptr || (data == nullptr && size == 0u));
    }


    ////////////////////////////////////////////////////////////
    template <SizeT N>
    [[nodiscard, gnu::always_inline]] constexpr Span(T (&array)[N]) : m_data{array}, m_size{N}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* data() const
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT size() const
    {
        return m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* begin() const
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* end() const
    {
        return m_data + m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& operator[](SizeT i) const
    {
        SFML_BASE_ASSERT(i < m_size);
        return *(m_data + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool empty() const
    {
        return m_size == 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const T* rhsData, SizeT rhsSize) const
    {
        if (m_size != rhsSize)
            return false;

        for (SizeT i = 0u; i < m_size; ++i)
            if (m_data[i] != rhsData[i])
                return false;

        return true;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const Span& rhs) const
    {
        return valueEquals(rhs.m_data, rhs.m_size);
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T*    m_data{nullptr};
    SizeT m_size{0u};
};

} // namespace sf::base
