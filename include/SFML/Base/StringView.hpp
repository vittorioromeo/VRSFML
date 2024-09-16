#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Strlen.hpp"
#include "SFML/Base/Strncmp.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
class StringView
{
public:
    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(decltype(nullptr)) = delete;


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) StringView() noexcept : m_data{nullptr}, m_size{0u}
    {
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(const char* cStr) noexcept :
    m_data{cStr},
    m_size{SFML_BASE_STRLEN(cStr)}
    {
        SFML_BASE_ASSERT(cStr != nullptr);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(const char* cStr, SizeT len) noexcept :
    m_data{cStr},
    m_size{len}
    {
        SFML_BASE_ASSERT(cStr != nullptr);
    }


    //////////////////////////////////////////
    template <typename StringLike>
    [[nodiscard, gnu::always_inline]] constexpr StringView(const StringLike& stringLike) noexcept requires(requires {
        stringLike.data();
        stringLike.size();
    }) :
    m_data{stringLike.data()},
    m_size{stringLike.size()}
    {
        SFML_BASE_ASSERT(stringLike.data() != nullptr);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(const StringView&) noexcept = default;


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(StringView&&) noexcept = default;


    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr StringView& operator=(const StringView&) noexcept = default;


    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr StringView& operator=(StringView&&) noexcept = default;


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* data() const noexcept
    {
        return m_data;
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT size() const noexcept
    {
        return m_size;
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool empty() const noexcept
    {
        return m_size == 0u;
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr char operator[](SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(i < m_size);

        return m_data[i];
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator==(const StringView& lhs,
                                                                                         const StringView& rhs) noexcept
    {
        if (lhs.m_size != rhs.m_size)
            return false;

        return SFML_BASE_STRNCMP(lhs.m_data, rhs.m_data, lhs.m_size) == 0;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator!=(const StringView& lhs,
                                                                                         const StringView& rhs) noexcept
    {
        return !(lhs == rhs);
    }

private:
    const char* m_data;
    SizeT       m_size;
};

} // namespace sf::base


namespace sf::base::literals
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr StringView operator""_sv(const char* str, SizeT len) noexcept
{
    return StringView{str, len};
}

} // namespace sf::base::literals
