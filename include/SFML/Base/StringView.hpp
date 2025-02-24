#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Strlen.hpp"
#include "SFML/Base/Builtins/Strncmp.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"

namespace sf::base
{
////////////////////////////////////////////////////////////
class StringView
{
private:
    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] static constexpr bool containsChar(const char        needle,
                                                                                    const StringView& haystack) noexcept
    {
        for (char c : haystack)
            if (c == needle)
                return true;

        return false;
    }

public:
    //////////////////////////////////////////
    enum : SizeT
    {
        nPos = static_cast<SizeT>(-1)
    };

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
    [[nodiscard, gnu::always_inline]] constexpr StringView(const char* cStr, const SizeT len) noexcept :
    m_data{cStr},
    m_size{len}
    {
        SFML_BASE_ASSERT(cStr != nullptr);
    }

    //////////////////////////////////////////
    template <typename StringLike>
    [[nodiscard, gnu::always_inline]] constexpr StringView(const StringLike& stringLike) noexcept
        requires(requires {
                    stringLike.data();
                    stringLike.size();
                })
    : m_data{stringLike.data()}, m_size{stringLike.size()}
    {
        SFML_BASE_ASSERT(stringLike.data() != nullptr);
    }

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
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr StringView substrByPosLen(const SizeT startPos = 0u,
                                                                                     const SizeT len      = nPos) const
    {
        SFML_BASE_ASSERT(startPos <= m_size);

        const SizeT maxPossibleLength = startPos > m_size ? 0 : m_size - startPos;
        const SizeT lengthToUse       = base::min(len, maxPossibleLength);

        return {m_data + startPos, lengthToUse};
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const StringView v, const SizeT startPos = 0u) const noexcept
    {
        // Exit early if substring is bigger than actual string
        if (startPos > m_size || (startPos + v.m_size) > m_size)
            return nPos;

        const SizeT offset = startPos;
        const SizeT inc    = m_size - v.m_size - offset;

        for (SizeT i = 0u; i <= inc; ++i)
        {
            const SizeT j = i + offset;

            if (substrByPosLen(j, v.m_size) == v)
                return j;
        }

        return nPos;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(char c, SizeT startPos = 0u) const noexcept
    {
        return find(StringView{&c, 1}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return find(StringView{cStr, count}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const char* cStr, SizeT startPos = 0u) const noexcept
    {
        return find(StringView{cStr}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(StringView v, SizeT startPos = 0u) const noexcept
    {
        if (empty())
            return v.empty() ? SizeT{0u} : static_cast<SizeT>(nPos);

        if (v.empty())
            return base::min(m_size - 1, startPos);

        if (v.m_size > m_size)
            return nPos;

        for (SizeT i = base::min(startPos, (m_size - v.m_size)); i != nPos; --i)
            if (substrByPosLen(i, v.m_size) == v)
                return i;

        return nPos;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(char c, SizeT startPos = nPos) const noexcept
    {
        return rfind(StringView{&c, 1}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return rfind(StringView{cStr, count}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const char* cStr, SizeT startPos = nPos) const noexcept
    {
        return rfind(StringView{cStr}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(StringView v, SizeT startPos = 0u) const noexcept
    {
        const SizeT maxIdx = m_size;

        for (SizeT i = startPos; i < maxIdx; ++i)
            if (containsChar(m_data[i], v))
                return i;

        return nPos;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(char c, SizeT startPos = 0u) const noexcept
    {
        return findFirstOf(StringView{&c, 1}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findFirstOf(StringView{cStr, count}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const char* cStr, SizeT startPos = 0u) const noexcept
    {
        return findFirstOf(StringView{cStr}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(StringView v, SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        const SizeT maxIdx = base::min(m_size - 1, startPos);

        for (SizeT i = 0u; i <= maxIdx; ++i)
        {
            const SizeT j = maxIdx - i;

            if (containsChar(m_data[j], v))
                return j;
        }

        return nPos;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(char c, SizeT startPos = nPos) const noexcept
    {
        return findLastOf(StringView{&c, 1}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findLastOf(StringView{cStr, count}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const char* cStr, SizeT startPos = nPos) const noexcept
    {
        return findLastOf(StringView{cStr}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(StringView v, SizeT startPos = 0u) const noexcept
    {
        const SizeT maxIdx = m_size;

        for (SizeT i = startPos; i < maxIdx; ++i)
            if (!containsChar(m_data[i], v))
                return i;

        return nPos;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(char c, SizeT startPos = 0u) const noexcept
    {
        return findFirstNotOf(StringView{&c, 1}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findFirstNotOf(StringView{cStr, count}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const char* cStr, SizeT startPos = 0u) const noexcept
    {
        return findFirstNotOf(StringView{cStr}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(StringView v, SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        const SizeT maxIdx = base::min(m_size - 1, startPos);

        for (SizeT i = 0u; i <= maxIdx; ++i)
        {
            const SizeT j = maxIdx - i;

            if (!containsChar(m_data[j], v))
                return j;
        }

        return nPos;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(char c, SizeT startPos = nPos) const noexcept
    {
        return findLastNotOf(StringView{&c, 1}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findLastNotOf(StringView{cStr, count}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const char* cStr, SizeT startPos = nPos) const noexcept
    {
        return findLastNotOf(StringView{cStr}, startPos);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* begin() const noexcept
    {
        return m_data;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* cbegin() const noexcept
    {
        return m_data;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* end() const noexcept
    {
        return m_data + m_size;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* cend() const noexcept
    {
        return m_data + m_size;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char& operator[](SizeT i) const noexcept
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

    //////////////////////////////////////////
    template <typename StringLike>
    [[nodiscard, gnu::always_inline]] StringLike toString() const
    {
        return StringLike{m_data, m_size};
    }

private:
    const char* m_data;
    SizeT       m_size;
};

//////////////////////////////////////////
template <typename StreamLike>
StreamLike& operator<<(StreamLike& stream, const StringView& stringView)
    requires(requires { stream.write(stringView.data(), static_cast<long>(stringView.size())); })
{
    stream.write(stringView.data(), static_cast<long>(stringView.size()));
    return stream;
}

} // namespace sf::base

namespace sf::base::literals
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr StringView operator""_sv(const char* cStr,
                                                                                                     SizeT len) noexcept
{
    return StringView{cStr, len};
}

} // namespace sf::base::literals
