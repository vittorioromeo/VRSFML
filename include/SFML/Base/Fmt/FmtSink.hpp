#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Fmt/FmtResult.hpp"
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
/// Wraps a contiguous `[begin, end)` byte range. All fallible writes return
/// a `FmtResult`; callers should propagate non-`Ok` results immediately
/// (usually via `SFML_BASE_FMT_TRY`).
///
/// \note `fmt(...)` recursion requires the full `<SFML/Base/Fmt/Fmt.hpp>`
/// header. This forward header only provides the byte-level API.
////////////////////////////////////////////////////////////
class FmtSink
{
public:
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
    [[nodiscard, gnu::always_inline]] constexpr char* position() const noexcept
    {
        return m_pos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr char* end() const noexcept
    {
        return m_end;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Bytes written so far (equivalently: a checkpoint that
    /// can be passed to `atOffset(n)` to recover the byte address it
    /// referred to). `dispatchFmtArg` uses it that way to bracket a
    /// `fmtArg` call and pad the produced range in-place.
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr SizeT size() const noexcept
    {
        return static_cast<SizeT>(m_pos - m_begin);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Address of the `n`-th byte from `begin()`, regardless of
    /// the current write position. Pair with `size()` as a checkpoint
    /// to come back to.
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr char* atOffset(const SizeT n) const noexcept
    {
        return m_begin + n;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr FmtResult ensureRoom(const SizeT n) const noexcept
    {
        if (static_cast<SizeT>(m_end - m_pos) >= n) [[likely]]
            return FmtResult::Ok;

        return FmtResult::Overflow;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void advance(const SizeT n) noexcept
    {
        m_pos += n;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr FmtResult append(const char* const data, const SizeT n) noexcept
    {
        SFML_BASE_FMT_TRY(ensureRoom(n));

        SFML_BASE_MEMCPY(m_pos, data, n);
        m_pos += n;
        return FmtResult::Ok;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr FmtResult appendChar(const char c) noexcept
    {
        SFML_BASE_FMT_TRY(ensureRoom(1u));

        *m_pos++ = c;
        return FmtResult::Ok;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Recursive `fmt`. Defined in `<SFML/Base/Fmt/Fmt.hpp>` --
    /// including only this forward header gives you a link error if you
    /// call it, by design.
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    [[nodiscard]] constexpr FmtResult fmt(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    char* m_begin;
    char* m_pos;
    char* m_end;
};

} // namespace sf::base
