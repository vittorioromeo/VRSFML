#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/SizeT.hpp"

#ifndef __GNUC__
    #include "SFML/Base/Builtin/Strlen.hpp"
    #include "SFML/Base/Builtin/Strncmp.hpp"
#endif


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
#ifndef __GNUC__
    #define SFML_BASE_PRIV_CONSTEXPR_STRLEN  SFML_BASE_STRLEN
    #define SFML_BASE_PRIV_CONSTEXPR_STRNCMP SFML_BASE_STRNCMP
#else
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
    [[nodiscard, gnu::always_inline]] constexpr StringView(const char* const cStr) noexcept :
        theData{cStr},
        theSize{SFML_BASE_PRIV_CONSTEXPR_STRLEN(cStr)}
    {
        SFML_BASE_ASSERT(cStr != nullptr);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView(const char* const cStr, const SizeT len) noexcept :
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
        const SizeT lengthToUse       = SFML_BASE_MIN(len, maxPossibleLength);

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
        if (v.theSize == 0)
            return startPos <= theSize ? startPos : nPos;

        if (startPos >= theSize || v.theSize > theSize - startPos)
            return nPos;

        const char* const lastPossibleStart = theData + theSize - v.theSize;

        for (const char* p = theData + startPos; p <= lastPossibleStart; ++p)
            if (SFML_BASE_PRIV_CONSTEXPR_STRNCMP(p, v.theData, v.theSize) == 0)
                return static_cast<SizeT>(p - theData);

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const char c, const SizeT startPos = 0u) const noexcept
    {
        if (startPos >= theSize)
            return nPos;

        for (SizeT i = startPos; i < theSize; ++i)
            if (theData[i] == c)
                return i;

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const char* cStr,
                                                                      const SizeT startPos,
                                                                      const SizeT count) const noexcept
    {
        return find(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT find(const char* cStr, const SizeT startPos = 0u) const noexcept
    {
        return find(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const StringView v, const SizeT startPos = nPos) const noexcept
    {
        if (v.theSize > theSize)
            return nPos;

        if (v.empty())
            return SFML_BASE_MIN(startPos, theSize);

        SizeT pos = SFML_BASE_MIN(startPos, theSize - v.theSize);

        do
        {
            if (SFML_BASE_PRIV_CONSTEXPR_STRNCMP(theData + pos, v.theData, v.theSize) == 0)
                return pos;
        } while (pos-- > 0);

        return nPos;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const char c, const SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        SizeT pos = SFML_BASE_MIN(startPos, theSize - 1);

        do
        {
            if (theData[pos] == c)
                return pos;
        } while (pos-- > 0);

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const char* cStr,
                                                                       const SizeT startPos,
                                                                       const SizeT count) const noexcept
    {
        return rfind(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT rfind(const char* cStr, const SizeT startPos = nPos) const noexcept
    {
        return rfind(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const StringView v,
                                                                             const SizeT startPos = 0u) const noexcept
    {
        const SizeT maxIdx = theSize;

        for (SizeT i = startPos; i < maxIdx; ++i)
            if (containsChar(theData[i], v))
                return i;

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const char c, const SizeT startPos = 0u) const noexcept
    {
        return find(c, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const char* const cStr,
                                                                             const SizeT       startPos,
                                                                             const SizeT       count) const noexcept
    {
        return findFirstOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstOf(const char* const cStr,
                                                                             const SizeT startPos = 0u) const noexcept
    {
        return findFirstOf(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const StringView v,
                                                                            const SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        const SizeT maxIdx = SFML_BASE_MIN(theSize - 1, startPos);

        for (SizeT i = 0u; i <= maxIdx; ++i)
        {
            const SizeT j = maxIdx - i;

            if (containsChar(theData[j], v))
                return j;
        }

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const char c, const SizeT startPos = nPos) const noexcept
    {
        return rfind(c, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const char* const cStr,
                                                                            const SizeT       startPos,
                                                                            const SizeT       count) const noexcept
    {
        return findLastOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastOf(const char* const cStr,
                                                                            const SizeT startPos = nPos) const noexcept
    {
        return findLastOf(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const StringView v,
                                                                                const SizeT startPos = 0u) const noexcept
    {
        const SizeT maxIdx = theSize;

        for (SizeT i = startPos; i < maxIdx; ++i)
            if (!containsChar(theData[i], v))
                return i;

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const char c, const SizeT startPos = 0u) const noexcept
    {
        for (SizeT i = startPos; i < theSize; ++i)
            if (theData[i] != c)
                return i;

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const char* const cStr,
                                                                                const SizeT       startPos,
                                                                                const SizeT       count) const noexcept
    {
        return findFirstNotOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findFirstNotOf(const char* const cStr,
                                                                                const SizeT startPos = 0u) const noexcept
    {
        return findFirstNotOf(StringView{cStr}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const StringView v,
                                                                               const SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        const SizeT maxIdx = SFML_BASE_MIN(theSize - 1, startPos);

        for (SizeT i = 0u; i <= maxIdx; ++i)
        {
            const SizeT j = maxIdx - i;

            if (!containsChar(theData[j], v))
                return j;
        }

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const char c, const SizeT startPos = nPos) const noexcept
    {
        if (empty())
            return nPos;

        for (SizeT i = SFML_BASE_MIN(startPos, theSize - 1) + 1; i > 0; --i)
            if (theData[i - 1] != c)
                return i - 1;

        return nPos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const char* const cStr,
                                                                               const SizeT       startPos,
                                                                               const SizeT       count) const noexcept
    {
        return findLastNotOf(StringView{cStr, count}, startPos);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT findLastNotOf(const char* const cStr,
                                                                               const SizeT startPos = nPos) const noexcept
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
    template <typename T>
    [[nodiscard, gnu::always_inline]] constexpr T to() const
    {
        return T{theData, theSize};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const char& operator[](const SizeT i) const noexcept
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
        const SizeT minSize = SFML_BASE_MIN(theSize, rhs.theSize);
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
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator==(const StringView& lhs,
                                                                                         const char* const rhs) noexcept
    {
        return lhs == StringView{rhs};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator!=(const StringView& lhs,
                                                                                         const char* const rhs) noexcept
    {
        return !(lhs == rhs);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator<(const StringView& lhs,
                                                                                        const StringView& rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator>(const StringView& lhs,
                                                                                        const StringView& rhs) noexcept
    {
        return rhs < lhs;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator<=(const StringView& lhs,
                                                                                         const StringView& rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator>=(const StringView& lhs,
                                                                                         const StringView& rhs) noexcept
    {
        return rhs <= lhs;
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
        alignas(StringView) char temp[sizeof(StringView)];

        SFML_BASE_MEMCPY(&temp, &lhs, sizeof(StringView));
        SFML_BASE_MEMCPY(&lhs, &rhs, sizeof(StringView));
        SFML_BASE_MEMCPY(&rhs, &temp, sizeof(StringView));
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


} // namespace sf::base


namespace sf::base::literals
{
////////////////////////////////////////////////////////////
[[nodiscard]] consteval StringView operator""_sv(const char* const cStr, const SizeT len) noexcept
{
    return StringView{cStr, len};
}

} // namespace sf::base::literals
