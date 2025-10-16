#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Builtin/Strncmp.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"


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

    ////////////////////////////////////////////////////////////
#ifdef __GNUC__
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr SizeT constexprStrLen(const char* const cStr) noexcept
    {
        const char* end = cStr;

        while (*end != '\0')
            ++end;

        return static_cast<SizeT>(end - cStr);
    }

    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr int constexprStrNCmp(const char* s1, const char* s2, SizeT n)
    {
        while (n && *s1 && (*s1 == *s2))
        {
            ++s1;
            ++s2;
            --n;
        }

        if (n == 0)
            return 0;

        return *s1 - *s2;
    }

    #define SFML_BASE_PRIV_CONSTEXPR_STRLEN  constexprStrLen
    #define SFML_BASE_PRIV_CONSTEXPR_STRNCMP constexprStrNCmp
#else
    #define SFML_BASE_PRIV_CONSTEXPR_STRLEN  SFML_BASE_STRLEN
    #define SFML_BASE_PRIV_CONSTEXPR_STRNCMP SFML_BASE_STRNCMP
#endif


public:
    ////////////////////////////////////////////////////////////
    enum : SizeT
    {
        nPos = static_cast<SizeT>(-1)
    };


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(decltype(nullptr)) = delete;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) StringView() noexcept = default;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(const char* cStr) noexcept :
        theData{cStr},
        theSize{SFML_BASE_PRIV_CONSTEXPR_STRLEN(cStr)}
    {
        SFML_BASE_ASSERT(cStr != nullptr);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(const char* cStr, const SizeT len) noexcept :
        theData{cStr},
        theSize{len}
    {
        SFML_BASE_ASSERT(cStr != nullptr || (cStr == nullptr && len == 0u));
    }


    ////////////////////////////////////////////////////////////
    template <typename StringLike>
    [[nodiscard, gnu::always_inline]] constexpr StringView(const StringLike& stringLike) noexcept
        requires(requires {
                    stringLike.data();
                    stringLike.size();
                })
        : theData{stringLike.data()}, theSize{stringLike.size()}
    {
        SFML_BASE_ASSERT(stringLike.data() != nullptr);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* data() const noexcept
    {
        return theData;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT size() const noexcept
    {
        return theSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool empty() const noexcept
    {
        return theSize == 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr StringView substrByPosLen(const SizeT startPos = 0u,
                                                                                     const SizeT len      = nPos) const
    {
        SFML_BASE_ASSERT(startPos <= theSize);

        const SizeT maxPossibleLength = startPos > theSize ? 0 : theSize - startPos;
        const SizeT lengthToUse       = base::min(len, maxPossibleLength);

        return {theData + startPos, lengthToUse};
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void removePrefix(const SizeT n) noexcept
    {
        SFML_BASE_ASSERT(n <= theSize);
        theData += n;
        theSize -= n;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void removeSuffix(const SizeT n) noexcept
    {
        SFML_BASE_ASSERT(n <= theSize);
        theSize -= n;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const StringView v, const SizeT startPos = 0u) const noexcept
    {
        // Exit early if substring is bigger than actual string
        if (startPos > theSize || (startPos + v.theSize) > theSize)
            return nPos;

        const SizeT offset = startPos;
        const SizeT inc    = theSize - v.theSize - offset;

        for (SizeT i = 0u; i <= inc; ++i)
        {
            const SizeT j = i + offset;

            if (substrByPosLen(j, v.theSize) == v)
                return j;
        }

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(char c, SizeT startPos = 0u) const noexcept
    {
        return find(StringView{&c, 1}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return find(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const char* cStr, SizeT startPos = 0u) const noexcept
    {
        return find(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const StringView v, SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return v.empty() ? SizeT{0u} : static_cast<SizeT>(nPos);

        if (v.empty())
            return base::min(theSize - 1, startPos);

        if (v.theSize > theSize)
            return nPos;

        SizeT pos = base::min(startPos, theSize - v.theSize);

        do
        {
            if (substrByPosLen(pos, v.theSize) == v)
                return pos;
        } while (pos-- > 0);

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(char c, SizeT startPos = nPos) const noexcept
    {
        return rfind(StringView{&c, 1}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return rfind(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const char* cStr, SizeT startPos = nPos) const noexcept
    {
        return rfind(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(StringView v, SizeT startPos = 0u) const noexcept
    {
        const SizeT maxIdx = theSize;

        for (SizeT i = startPos; i < maxIdx; ++i)
            if (containsChar(theData[i], v))
                return i;

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(char c, SizeT startPos = 0u) const noexcept
    {
        return findFirstOf(StringView{&c, 1}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findFirstOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const char* cStr, SizeT startPos = 0u) const noexcept
    {
        return findFirstOf(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(StringView v, SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        const SizeT maxIdx = base::min(theSize - 1, startPos);

        for (SizeT i = 0u; i <= maxIdx; ++i)
        {
            const SizeT j = maxIdx - i;

            if (containsChar(theData[j], v))
                return j;
        }

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(char c, SizeT startPos = nPos) const noexcept
    {
        return findLastOf(StringView{&c, 1}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findLastOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const char* cStr, SizeT startPos = nPos) const noexcept
    {
        return findLastOf(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(StringView v, SizeT startPos = 0u) const noexcept
    {
        const SizeT maxIdx = theSize;

        for (SizeT i = startPos; i < maxIdx; ++i)
            if (!containsChar(theData[i], v))
                return i;

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(char c, SizeT startPos = 0u) const noexcept
    {
        return findFirstNotOf(StringView{&c, 1}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findFirstNotOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const char* cStr, SizeT startPos = 0u) const noexcept
    {
        return findFirstNotOf(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(StringView v, SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        const SizeT maxIdx = base::min(theSize - 1, startPos);

        for (SizeT i = 0u; i <= maxIdx; ++i)
        {
            const SizeT j = maxIdx - i;

            if (!containsChar(theData[j], v))
                return j;
        }

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(char c, SizeT startPos = nPos) const noexcept
    {
        return findLastNotOf(StringView{&c, 1}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const char* cStr, SizeT startPos, SizeT count) const noexcept
    {
        return findLastNotOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const char* cStr, SizeT startPos = nPos) const noexcept
    {
        return findLastNotOf(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* begin() const noexcept
    {
        return theData;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* cbegin() const noexcept
    {
        return theData;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* end() const noexcept
    {
        return theData + theSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char* cend() const noexcept
    {
        return theData + theSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char& operator[](SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(theData != nullptr);
        SFML_BASE_ASSERT(i < theSize);

        return theData[i];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator==(const StringView& lhs,
                                                                                         const StringView& rhs) noexcept
    {
        if (lhs.theSize != rhs.theSize)
            return false;

        return SFML_BASE_PRIV_CONSTEXPR_STRNCMP(lhs.theData, rhs.theData, lhs.theSize) == 0;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator!=(const StringView& lhs,
                                                                                         const StringView& rhs) noexcept
    {
        return !(lhs == rhs);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr int compare(const StringView& rhs) const noexcept
    {
        const SizeT minSize = base::min(theSize, rhs.theSize);
        const int   result  = SFML_BASE_PRIV_CONSTEXPR_STRNCMP(theData, rhs.theData, minSize);

        if (result != 0)
            return result;

        if (theSize < rhs.theSize)
            return -1;

        if (theSize > rhs.theSize)
            return 1;

        return 0;
    }


    ////////////////////////////////////////////////////////////
    template <typename StringLike>
    [[nodiscard, gnu::always_inline]] StringLike toString() const
    {
        return StringLike{theData, theSize};
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend void swap(StringView& lhs, StringView& rhs) noexcept
    {
        base::swap(lhs.theData, rhs.theData);
        base::swap(lhs.theSize, rhs.theSize);
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const char* theData{nullptr};
    SizeT       theSize{0u};
};


////////////////////////////////////////////////////////////
#undef SFML_BASE_PRIV_CONSTEXPR_STRNCMP
#undef SFML_BASE_PRIV_CONSTEXPR_STRLEN


////////////////////////////////////////////////////////////
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
[[nodiscard]] consteval StringView operator""_sv(const char* cStr, SizeT len) noexcept
{
    return StringView{cStr, len};
}

} // namespace sf::base::literals
