#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Internal format-string carrier (data pointer + length).
///
/// Used in place of `base::StringView` so MiniFmt's transitive include
/// closure stays free of `<SFML/Base/StringView.hpp>`. Same 16-byte
/// layout, no member functions -- callers read `data` and `size` directly.
////////////////////////////////////////////////////////////
struct FmtSpan
{
    const char* data;
    SizeT       size;
};

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Per-placeholder format spec.
///
/// Grammar (subset of `std::format`):
///     spec ::= [[fill]align]? [width]? [.precision]? [type]?
///     fill      = any single character (defaults to space)
///     align     = '<' (left), '>' (right), '^' (center)
///     width     = decimal digits
///     precision = '.' followed by decimal digits (floats only)
///     type      = 'f' (float fixed) -- purely informational
///
/// Defaults: numeric args right-align, everything else left-aligns.
////////////////////////////////////////////////////////////
struct FormatSpec
{
    int  width     = 0;
    int  precision = -1;
    char align     = '\0';
    char fill      = ' ';
};


////////////////////////////////////////////////////////////
/// \brief Default alignment for a type when `spec.align` is unset.
///
/// Specialize for your type to change the default:
/// `template <> inline constexpr char sf::base::formatArgDefaultAlign<MyType> = '>';`
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr char formatArgDefaultAlign = '<';


////////////////////////////////////////////////////////////
// Forward declaration of the engine-validated format string template.
////////////////////////////////////////////////////////////
template <typename... Args>
struct FormatString;


////////////////////////////////////////////////////////////
template <typename T>
struct NonDeduced
{
    using type = T;
};


////////////////////////////////////////////////////////////
/// \brief Output sink that custom `formatArg` overloads write into.
///
/// Wraps a contiguous `[begin, end)` byte range. All writes go through
/// `ensureRoom`, which either succeeds, invokes the optional grow hook,
/// or sets a sticky `overflowed` flag. Once overflowed, all further
/// writes are no-ops -- callers don't need to short-circuit themselves.
///
/// The grow hook is a deliberate extension point: install it to make
/// the sink expand on demand (used by `formatTo`'s heap-fallback path).
/// When null, overflow is fatal-but-graceful.
///
/// \note `format(...)` recursion requires the full `<SFML/Base/MiniFmt.hpp>`
/// header. This forward header only provides the byte-level API.
////////////////////////////////////////////////////////////
class FormatSink
{
public:
    using GrowFn = bool (*)(FormatSink& sink, SizeT extra, void* ctx);

    using Mark = SizeT;

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr FormatSink(char* const begin, char* const end) noexcept :
        m_begin{begin},
        m_pos{begin},
        m_end{end}
    {
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr FormatSink(char* const begin, const SizeT n) noexcept :
        FormatSink{begin, begin + n}
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
    [[nodiscard, gnu::always_inline]] constexpr SizeT remaining() const noexcept
    {
        return static_cast<SizeT>(m_end - m_pos);
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

        if (m_grow != nullptr && m_grow(*this, n, m_growCtx) && static_cast<SizeT>(m_end - m_pos) >= n)
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
    constexpr void rebind(char* const newBegin, char* const newPos, char* const newEnd) noexcept
    {
        m_begin = newBegin;
        m_pos   = newPos;
        m_end   = newEnd;
    }

    ////////////////////////////////////////////////////////////
    constexpr void setGrowHook(const GrowFn fn, void* const ctx) noexcept
    {
        m_grow    = fn;
        m_growCtx = ctx;
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
    /// \brief Recursive `format`. Defined in `<SFML/Base/MiniFmt.hpp>` --
    /// including only this forward header gives you a link error if you
    /// call it, by design.
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    constexpr void format(typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args);

private:
    char*  m_begin;
    char*  m_pos;
    char*  m_end;
    GrowFn m_grow       = nullptr;
    void*  m_growCtx    = nullptr;
    bool   m_overflowed = false;
};


////////////////////////////////////////////////////////////
// Built-in `formatArg` overloads that don't need `ToChars` or `String`.
////////////////////////////////////////////////////////////

// String-like: anything with `.data() -> const char*` and `.size() -> SizeT`.
// Covers `base::String`, `base::StringView`, `std::string`, `std::string_view`, ...
// (StringView itself is NOT pulled in by this header; callers who hold a
// `StringView` should pass `sv.data(), sv.size()` to `sink.append`.)
template <typename T>
    requires requires(const T& x) {
        x.data();
        x.size();
    }
[[gnu::always_inline]] inline constexpr void formatArg(FormatSink& sink, const T& arg, const FormatSpec&) noexcept
{
    sink.append(arg.data(), static_cast<SizeT>(arg.size()));
}


////////////////////////////////////////////////////////////
// Null-terminated C string.
[[gnu::always_inline]] inline constexpr void formatArg(FormatSink& sink, const char* const arg, const FormatSpec&) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(arg != nullptr);
    sink.append(arg, SFML_BASE_STRLEN(arg));
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Lightweight prelude for headers that declare custom
/// `formatArg` overloads. Pulls in `FormatSink`, `FormatSpec`, and the
/// zero-scratch string-like / `const char*` built-ins -- nothing else.
///
/// Headers that only need to *declare* a `formatArg` overload should
/// include this. Files that *call* `format` / `formatTo` / `print` /
/// `formatIntoBuffer`, or whose `formatArg` recurses via `sink.format`,
/// must include the full `<SFML/Base/MiniFmt.hpp>` instead.
////////////////////////////////////////////////////////////
