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
/// \brief Result of a Fmt operation.
////////////////////////////////////////////////////////////
enum class [[nodiscard]] FmtResult
{
    ok,       //!< Formatting/writing succeeded
    overflow, //!< Destination buffer was too small; retrying with more room may succeed
    failed    //!< Formatter could not produce output; retrying with more room will not help
};


////////////////////////////////////////////////////////////
#define SFML_BASE_FMT_TRY(...)                                        \
    do                                                                \
    {                                                                 \
        const ::sf::base::FmtResult sfmlFmtTryResult = (__VA_ARGS__); \
        if (sfmlFmtTryResult != ::sf::base::FmtResult::ok)            \
            return sfmlFmtTryResult;                                  \
    } while (false)


////////////////////////////////////////////////////////////
/// \brief Output sink that custom `fmtArg` overloads write into.
///
/// Wraps a contiguous `[begin, end)` byte range. All fallible writes return
/// a `FmtResult`; callers should propagate non-`ok` results immediately
/// (usually via `SFML_BASE_FMT_TRY`).
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
    [[nodiscard, gnu::always_inline]] constexpr FmtResult ensureRoom(const SizeT n) const noexcept
    {
        if (static_cast<SizeT>(m_end - m_pos) >= n) [[likely]]
            return FmtResult::ok;

        return FmtResult::overflow;
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
        return FmtResult::ok;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr FmtResult appendChar(const char c) noexcept
    {
        SFML_BASE_FMT_TRY(ensureRoom(1u));

        *m_pos++ = c;
        return FmtResult::ok;
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
