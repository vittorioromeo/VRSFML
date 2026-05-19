#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memmove.hpp"
#include "SFML/Base/Builtin/Memset.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Fmt/FmtArgDefaultAlign.hpp" // IWYU pragma: export
#include "SFML/Base/Fmt/FmtCString.hpp"         // IWYU pragma: export
#include "SFML/Base/Fmt/FmtSink.hpp"            // IWYU pragma: export
#include "SFML/Base/Fmt/FmtSpan.hpp"            // IWYU pragma: export
#include "SFML/Base/Fmt/FmtSpec.hpp"            // IWYU pragma: export
#include "SFML/Base/Fmt/FmtStringLike.hpp"      // IWYU pragma: export
#include "SFML/Base/NonDeduced.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Public-facing names live in `sf::base`.
// Engine internals (out-of-line helpers, type-erased dispatch, parsing,
// padding) live in `sf::base::priv` so they don't pollute lookup.
//
// This header intentionally OMITS the numeric `fmtArg` overloads
// (int, float) -- they live in `<SFML/Base/Fmt/FmtNumeric.hpp>` and pull
// in `ToChars` + integer/floating-point traits. Callers that want to
// format numeric arguments should include `FmtNumeric.hpp` (or one
// of the umbrella headers that re-exports it, such as `FmtToString.hpp`
// or `<SFML/System/Err.hpp>`).
////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Default float precision when a spec omits `.N` (matches `std::format` / `printf`).
////////////////////////////////////////////////////////////
inline constexpr int defaultFloatPrecision = 6;


////////////////////////////////////////////////////////////
/// \brief Initial stack-staging size for `fmtTo` / `print`. NOT a
/// hard limit: an overflow falls back to a doubling heap buffer, so
/// any output size is supported.
////////////////////////////////////////////////////////////
inline constexpr SizeT fmtScratchSize = 512u;

} // namespace sf::base


////////////////////////////////////////////////////////////
namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Parse a format spec body (everything between '{:' and '}').
///
/// `p` is advanced to the position of the closing '}'. Caller verifies '}' presence.
/// At consteval, throws on width/precision overflow for a clear compile-time
/// diagnostic; at runtime, the corresponding asserts fire (debug only).
///
/// Grammar:
///     spec ::= [[fill]align]? [width]? [.precision]? [type]?
////////////////////////////////////////////////////////////
constexpr void parseFmtSpec(const char*& p, const char* const end, FmtSpec& spec)
{
    // Char-classification helpers -- local to avoid colliding with
    // `priv::isDigit` from <SFML/Base/FromChars.hpp>.
    constexpr auto isAlignChar = [](const char c) { return c == '<' || c == '>' || c == '^'; };
    constexpr auto isDigit     = [](const char c) { return c >= '0' && c <= '9'; };

    if (p + 1 < end && isAlignChar(p[1]))
    {
        spec.fill  = *p++;
        spec.align = *p++;
    }
    else if (p < end && isAlignChar(*p))
    {
        spec.align = *p++;
    }

    while (p < end && isDigit(*p))
    {
        spec.width = spec.width * 10 + (*p - '0');
        if (spec.width >= 65'536)
        {
            if consteval
            {
                throw "Format spec width out of range (>= 65536)";
            }
            SFML_BASE_ASSERT(false);
        }
        ++p;
    }

    if (p < end && *p == '.')
    {
        ++p;
        spec.precision = 0;
        while (p < end && isDigit(*p))
        {
            spec.precision = spec.precision * 10 + (*p - '0');
            // Upper bound matches the float backend (`toChars` asserts precision <= 10).
            if (spec.precision > 10)
            {
                if consteval
                {
                    throw "Format spec precision out of range (> 10)";
                }
                SFML_BASE_ASSERT(false);
            }
            ++p;
        }
    }

    // Type tag. Accepted set: 'd' (decimal int), 'x' / 'X' (hex), 'o' (octal),
    // 'b' (binary), 'f' (float fixed). Anything else is malformed.
    if (p < end && *p != '}')
    {
        const char t = *p;
        if (t != 'd' && t != 'x' && t != 'X' && t != 'o' && t != 'b' && t != 'f')
        {
            if consteval
            {
                throw "Format spec type tag must be one of d/x/X/o/b/f or omitted";
            }
            SFML_BASE_ASSERT(false);
        }
        spec.type = t;
        ++p;
    }
}


////////////////////////////////////////////////////////////
/// \brief Walk the format string, validating each placeholder and counting them.
///
/// Throws (at consteval) on:
///   - unclosed `{`
///   - unescaped `}`
///   - malformed spec body (anything that leaves the parser short of `}`)
///   - width >= 65536 or precision > 10
///   - unknown type tag (anything outside `d` / `x` / `X` / `o` / `b` / `f`)
////////////////////////////////////////////////////////////
[[nodiscard]] consteval SizeT countPlaceholders(const FmtSpan fmtStr)
{
    const auto*       p   = fmtStr.data;
    const auto* const end = p + fmtStr.size;

    SizeT count = 0u;

    while (p < end)
    {
        if (*p == '{')
        {
            // Escaped '{{' -> '{'
            if (p + 1 < end && p[1] == '{')
            {
                p += 2;
                continue;
            }

            ++p; // skip '{'

            // Optional spec body
            if (p < end && *p == ':')
            {
                ++p; // skip ':'
                FmtSpec spec;
                parseFmtSpec(p, end, spec);
            }

            if (p >= end)
                throw "Invalid format string: Unclosed '{'";

            if (*p != '}')
                throw "Invalid format string: malformed spec (expected '}')";

            ++p; // skip '}'
            ++count;
        }
        else if (*p == '}')
        {
            // Escaped '}}' -> '}'
            if (p + 1 < end && p[1] == '}')
            {
                p += 2;
                continue;
            }
            throw "Invalid format string: unescaped '}'";
        }
        else
        {
            ++p;
        }
    }

    return count;
}

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Compile-time-validated format string: ensures placeholder
/// count matches `sizeof...(Args)`.
////////////////////////////////////////////////////////////
template <typename... Args>
struct [[nodiscard]] FmtString
{
    priv::FmtSpan str;

    consteval FmtString(const char* const s) : str{s, SFML_BASE_STRLEN(s)}
    {
        if (priv::countPlaceholders(str) != sizeof...(Args))
            throw "Mismatch between number of '{}' and number of arguments.";
    }
};

} // namespace sf::base


////////////////////////////////////////////////////////////
namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Apply per-placeholder padding around a `fmtArg` call.
///
/// Saves the sink position, runs the ADL `fmtArg`, measures content
/// length, and if `spec.width` exceeds it, applies fill+align in-place
/// via a single `memmove` + two `memset`s. The destination is the sink
/// itself -- no auxiliary scratch.
///
/// Body is in the header so user types still instantiate correctly via
/// ADL. Common numeric instantiations are pre-emitted in
/// `FmtNumeric.cpp` and declared `extern template` in
/// `FmtNumeric.hpp` so consumer TUs skip the per-TU codegen.
////////////////////////////////////////////////////////////
template <typename T>
constexpr void dispatchFmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec) noexcept
{
    if (sink.overflowed())
        return;

    const auto startMark = sink.mark();

    fmtArg(sink, arg, spec); // ADL

    if (sink.overflowed() || spec.width <= 0)
        return;

    const auto contentSize = sink.mark() - startMark;
    if (static_cast<SizeT>(spec.width) <= contentSize)
        return;

    const auto padTotal = static_cast<SizeT>(spec.width) - contentSize;
    if (!sink.ensureRoom(padTotal))
        return;

    constexpr char defAlign = fmtArgDefaultAlign<T>;
    const char     align    = spec.align == '\0' ? defAlign : spec.align;

    const SizeT padLeft  = (align == '<') ? 0u : (align == '^') ? padTotal / 2u : padTotal;
    const SizeT padRight = padTotal - padLeft;

    char* const start = sink.atMark(startMark);

    if (padLeft != 0u)
    {
        SFML_BASE_MEMMOVE(start + padLeft, start, contentSize);
        SFML_BASE_MEMSET(start, spec.fill, padLeft);
    }

    if (padRight != 0u)
        SFML_BASE_MEMSET(start + padLeft + contentSize, spec.fill, padRight);

    sink.advance(padTotal);
}


////////////////////////////////////////////////////////////
using ErasedDispatchFn = void (*)(FmtSink&, const void*, const FmtSpec&);


////////////////////////////////////////////////////////////
template <typename T>
constexpr void dispatchFmtArgErased(FmtSink& sink, const void* const erasedArg, const FmtSpec& spec) noexcept
{
    dispatchFmtArg(sink, *static_cast<const T*>(erasedArg), spec);
}


////////////////////////////////////////////////////////////
/// \brief Type-erased core. Walks `fmtStr` once, copying literals and
/// dispatching `{...}` placeholders via `dispatchers`.
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void fmtAssembleImpl(FmtSink&                sink,
                                     FmtSpan                 fmtStr,
                                     const void* const*      args,
                                     const ErasedDispatchFn* dispatchers,
                                     SizeT                   argCount);


////////////////////////////////////////////////////////////
/// \brief Heap-fallback for `fmtTo` / `print`. Allocates a doubling
/// `base::String` until the format succeeds, then flushes once through
/// `appendFn`.
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void fmtToHeapFallback(
    void* userSink,
    void (*appendFn)(void*, const char*, SizeT),
    FmtSpan                 fmtStr,
    const void* const*      args,
    const ErasedDispatchFn* dispatchers,
    SizeT                   argCount);


////////////////////////////////////////////////////////////
SFML_SYSTEM_API void fmtWriteStdout(const char* data, SizeT size);
SFML_SYSTEM_API void fmtWriteStdoutNewline();
SFML_SYSTEM_API void fmtFlushStdout();

SFML_SYSTEM_API void fmtWriteStderr(const char* data, SizeT size);
SFML_SYSTEM_API void fmtWriteStderrNewline();
SFML_SYSTEM_API void fmtFlushStderr();


////////////////////////////////////////////////////////////
struct StdoutSink
{
    void append(const char* data, SizeT n)
    {
        fmtWriteStdout(data, n);
    }
};


////////////////////////////////////////////////////////////
struct StderrSink
{
    void append(const char* data, SizeT n)
    {
        fmtWriteStderr(data, n);
    }
};

////////////////////////////////////////////////////////////
template <typename... Args>
constexpr void fmtAssemble(FmtSink& sink, const FmtSpan fmtStr, const Args&... args)
{
    // Size both arrays at `sizeof...(Args) + 1` so the empty-pack case still
    // produces a 1-element array (zero-initialized, never indexed because
    // `argCount == 0`). The trailing slot is wasted but unconditional --
    // collapses the dispatch into a single code path with no branch.
    const void* const          erasedArgs[sizeof...(Args) + 1]  = {&args...};
    constexpr ErasedDispatchFn dispatchers[sizeof...(Args) + 1] = {&dispatchFmtArgErased<Args>...};

    fmtAssembleImpl(sink, fmtStr, erasedArgs, dispatchers, sizeof...(Args));
}

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Args>
constexpr void FmtSink::fmt(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    priv::fmtAssemble(*this, fmtStr.str, args...);
}


////////////////////////////////////////////////////////////
/// \brief Format into a fixed buffer.
///
/// Returns a pointer one past the last written character on success, or
/// `nullptr` on overflow. The buffer is **not** null-terminated -- callers
/// that need a C string must reserve an extra byte and write `'\0'` at
/// the returned pointer themselves. On overflow the buffer contents are
/// indeterminate (may hold a partial write).
///
/// Matches `fmt::format_to` / `std::format_to` semantics; differs from
/// `snprintf` in that overflow is hard-fail rather than truncate.
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] constexpr char* fmtIntoBuffer(char* const                                         buffer,
                                            const SizeT                                         bufferSize,
                                            typename NonDeduced<const FmtString<Args...>>::type fmtStr,
                                            const Args&... args)
{
    FmtSink sink{buffer, buffer + bufferSize};
    priv::fmtAssemble(sink, fmtStr.str, args...);

    if (sink.overflowed())
        return nullptr;

    return sink.position();
}


////////////////////////////////////////////////////////////
template <SizeT N, typename... Args>
[[nodiscard]] constexpr char* fmtIntoBuffer(char (&buffer)[N],
                                            typename NonDeduced<const FmtString<Args...>>::type fmtStr,
                                            const Args&... args)
{
    return fmtIntoBuffer(buffer, N, fmtStr, args...);
}


////////////////////////////////////////////////////////////
/// \brief Sink that accepts `append(const char*, SizeT)`.
////////////////////////////////////////////////////////////
template <typename T>
concept AppendSink = requires(T& sink, const char* p, SizeT n) { sink.append(p, n); };


////////////////////////////////////////////////////////////
/// \brief Format into any sink with `append(const char*, SizeT)`.
///
/// Tries to fit into a `fmtScratchSize`-byte stack buffer first;
/// on overflow falls back to a doubling heap buffer (so there is no
/// hard size limit). Either way, the user sink receives the formatted
/// output in a single `append` call.
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename... Args>
void fmtTo(Sink& userSink, typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    char    scratch[fmtScratchSize];
    FmtSink sink{scratch, scratch + sizeof(scratch)};

    // See `priv::fmtAssemble`: trailing-sentinel sizing avoids the empty-pack branch.
    const void* const                erasedArgs[sizeof...(Args) + 1]  = {&args...};
    constexpr priv::ErasedDispatchFn dispatchers[sizeof...(Args) + 1] = {&priv::dispatchFmtArgErased<Args>...};

    priv::fmtAssembleImpl(sink, fmtStr.str, erasedArgs, dispatchers, sizeof...(Args));

    if (!sink.overflowed()) [[likely]]
    {
        userSink.append(scratch, sink.size());
        return;
    }

    const auto appendFn = +[](void* s, const char* data, SizeT n) { static_cast<Sink*>(s)->append(data, n); };

    priv::fmtToHeapFallback(&userSink, appendFn, fmtStr.str, erasedArgs, dispatchers, sizeof...(Args));
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stdout (no trailing newline; flushes
/// stdout). Use this for prompts and partial-line output.
////////////////////////////////////////////////////////////
template <typename... Args>
void print(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    priv::StdoutSink sink;
    fmtTo(sink, fmtStr, args...);
    priv::fmtFlushStdout();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stdout, followed by a newline.
/// No explicit flush -- on a TTY, '\n' triggers line-buffer flush;
/// on a piped stream, block buffering is preserved for throughput.
////////////////////////////////////////////////////////////
template <typename... Args>
void printLn(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    priv::StdoutSink sink;
    fmtTo(sink, fmtStr, args...);
    priv::fmtWriteStdoutNewline();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stderr (no trailing newline; flushes
/// stderr). Use this for partial-line error output.
////////////////////////////////////////////////////////////
template <typename... Args>
void printErr(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    priv::StderrSink sink;
    fmtTo(sink, fmtStr, args...);
    priv::fmtFlushStderr();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stderr, followed by a newline + flush.
////////////////////////////////////////////////////////////
template <typename... Args>
void printErrLn(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    priv::StderrSink sink;
    fmtTo(sink, fmtStr, args...);
    priv::fmtWriteStderrNewline();
    priv::fmtFlushStderr();
}


////////////////////////////////////////////////////////////
/// \brief Format and append to an arbitrary sink (alias for `fmtTo`).
///
/// Exists for symmetry with the stdout/stderr `print` overloads and
/// with `std::print(FILE*, ...)`. No flush -- the sink decides its own
/// buffering policy.
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename... Args>
void print(Sink& sink, typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    fmtTo(sink, fmtStr, args...);
}


////////////////////////////////////////////////////////////
/// \brief Format, append to a sink, then append a trailing '\n'.
///
/// No flush; matches `std::println(FILE*, ...)` semantics. The newline
/// is emitted via a single byte `append` -- block-buffered sinks like
/// `FILE*` keep their throughput characteristics.
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename... Args>
void printLn(Sink& sink, typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    fmtTo(sink, fmtStr, args...);
    sink.append("\n", 1u);
}

} // namespace sf::base
