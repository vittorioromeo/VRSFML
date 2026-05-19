#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/NonDeduced.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::base
{
template <typename... Args>
struct FmtString;
}


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Output sink that custom `fmtArg` overloads write into.
///
/// Wraps a contiguous `[begin, end)` byte range. All writes go through
/// `ensureRoom`, which either succeeds or sets a sticky `overflowed`
/// flag. Once overflowed, all further writes are no-ops -- callers don't
/// need to short-circuit themselves.
///
/// \note `fmt(...)` recursion requires the full `<SFML/Base/Fmt/Fmt.hpp>`
/// header. This forward header only provides the byte-level API.
////////////////////////////////////////////////////////////
class FmtSink
{
public:
    ////////////////////////////////////////////////////////////
    using Mark = SizeT;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr FmtSink(char* const begin, char* const end) noexcept :
        m_begin{begin},
        m_pos{begin},
        m_end{end}
    {
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr FmtSink(char* const begin, const SizeT n) noexcept : FmtSink{begin, begin + n}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr bool overflowed() const noexcept
    {
        return m_overflowed;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr char* position() const noexcept
    {
        return m_pos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr SizeT size() const noexcept
    {
        return static_cast<SizeT>(m_pos - m_begin);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Mark mark() const noexcept
    {
        return static_cast<SizeT>(m_pos - m_begin);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr char* atMark(const Mark m) const noexcept
    {
        return m_begin + m;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr bool ensureRoom(const SizeT n) noexcept
    {
        if (static_cast<SizeT>(m_end - m_pos) >= n) [[likely]]
            return true;

        m_overflowed = true;
        return false;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void advance(const SizeT n) noexcept
    {
        m_pos += n;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void append(const char* const data, const SizeT n) noexcept
    {
        if (!ensureRoom(n))
            return;

        SFML_BASE_MEMCPY(m_pos, data, n);
        m_pos += n;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void appendChar(const char c) noexcept
    {
        if (!ensureRoom(1u))
            return;

        *m_pos++ = c;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Recursive `fmt`. Defined in `<SFML/Base/Fmt/Fmt.hpp>` --
    /// including only this forward header gives you a link error if you
    /// call it, by design.
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    constexpr void fmt(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    char* m_begin;
    char* m_pos;
    char* m_end;

    bool m_overflowed = false;
};

} // namespace sf::base
