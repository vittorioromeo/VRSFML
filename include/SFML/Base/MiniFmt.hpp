#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/Builtin/Memmove.hpp"
#include "SFML/Base/Builtin/Memset.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Priv/MiniFmtFwd.hpp" // IWYU pragma: export
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Public-facing names live in `sf::base`.
// Engine internals (out-of-line helpers, type-erased dispatch, parsing,
// padding) live in `sf::base::priv` so they don't pollute lookup.
//
// This header intentionally OMITS the numeric `formatArg` overloads
// (int, float) -- they live in `<SFML/Base/MiniFmtNumeric.hpp>` and pull
// in `ToChars` + integer/floating-point traits. Callers that want to
// format numeric arguments should include `MiniFmtNumeric.hpp` (or one
// of the umbrella headers that re-exports it, such as `MiniFmtFormat.hpp`
// or `<SFML/System/Err.hpp>`).
////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Default float precision when a spec omits `.N` (matches `std::format` / `printf`).
////////////////////////////////////////////////////////////
inline constexpr int defaultFloatPrecision = 6;


////////////////////////////////////////////////////////////
/// \brief Initial stack-staging size for `formatTo` / `print`. NOT a
/// hard limit: an overflow falls back to a doubling heap buffer, so
/// any output size is supported.
////////////////////////////////////////////////////////////
inline constexpr SizeT formatToStagingSize = 512u;

} // namespace sf::base


////////////////////////////////////////////////////////////
namespace sf::base::priv
{
////////////////////////////////////////////////////////////
[[nodiscard]] consteval SizeT countPlaceholders(const FmtSpan formatString)
{
    const auto* const fmt  = formatString.data;
    const auto        size = formatString.size;

    SizeT count = 0u;

    for (SizeT i = 0u; i < size; ++i)
    {
        if (fmt[i] == '{')
        {
            if (i + 1u < size && fmt[i + 1u] == '{')
            {
                ++i;
                continue;
            }

            ++i;
            while (i < size && fmt[i] != '}')
                ++i;

            if (i >= size)
                throw "Invalid format string: Unclosed '{'";

            ++count;
        }
        else if (fmt[i] == '}')
        {
            if (i + 1u < size && fmt[i + 1u] == '}')
                ++i;
            else
                throw "Invalid format string: Unescaped '}'";
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
struct [[nodiscard]] FormatString
{
    priv::FmtSpan str;

    consteval FormatString(const char* const s) : str{s, SFML_BASE_STRLEN(s)}
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
/// \brief Apply per-placeholder padding around a `formatArg` call.
///
/// Saves the sink position, runs the ADL `formatArg`, measures content
/// length, and if `spec.width` exceeds it, applies fill+align in-place
/// via a single `memmove` + two `memset`s. The destination is the sink
/// itself -- no auxiliary scratch.
///
/// Body is in the header so user types still instantiate correctly via
/// ADL. Common numeric instantiations are pre-emitted in
/// `MiniFmtNumeric.cpp` and declared `extern template` in
/// `MiniFmtNumeric.hpp` so consumer TUs skip the per-TU codegen.
////////////////////////////////////////////////////////////
template <typename T>
constexpr void dispatchFormatArg(FormatSink& sink, const T& arg, const FormatSpec& spec) noexcept
{
    if (sink.overflowed())
        return;

    const auto startMark = sink.mark();

    formatArg(sink, arg, spec); // ADL

    if (sink.overflowed() || spec.width <= 0)
        return;

    const auto contentSize = sink.mark() - startMark;
    if (static_cast<SizeT>(spec.width) <= contentSize)
        return;

    const auto padTotal = static_cast<SizeT>(spec.width) - contentSize;
    if (!sink.ensureRoom(padTotal))
        return;

    constexpr char defAlign = formatArgDefaultAlign<T>;
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
using ErasedDispatchFn = void (*)(FormatSink&, const void*, const FormatSpec&);


////////////////////////////////////////////////////////////
template <typename T>
constexpr void erasedDispatch(FormatSink& sink, const void* const erasedArg, const FormatSpec& spec) noexcept
{
    dispatchFormatArg(sink, *static_cast<const T*>(erasedArg), spec);
}


////////////////////////////////////////////////////////////
/// \brief Type-erased core. Walks `fmtStr` once, copying literals and
/// dispatching `{...}` placeholders via `dispatchers`.
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void formatAssembleImpl(FormatSink&             sink,
                                        FmtSpan                 fmtStr,
                                        const void* const*      args,
                                        const ErasedDispatchFn* dispatchers,
                                        SizeT                   argCount);


////////////////////////////////////////////////////////////
/// \brief Heap-fallback for `formatTo` / `print`. Allocates a doubling
/// `base::String` until the format succeeds, then flushes once through
/// `appendFn`.
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void formatToHeapFallback(
    void* userSink,
    void (*appendFn)(void*, const char*, SizeT),
    FmtSpan                 fmtStr,
    const void* const*      args,
    const ErasedDispatchFn* dispatchers,
    SizeT                   argCount);


////////////////////////////////////////////////////////////
SFML_SYSTEM_API void formatWriteStdout(const char* data, SizeT size);
SFML_SYSTEM_API void formatWriteStdoutNewline();
SFML_SYSTEM_API void formatFlushStdout();

SFML_SYSTEM_API void formatWriteStderr(const char* data, SizeT size);
SFML_SYSTEM_API void formatWriteStderrNewline();
SFML_SYSTEM_API void formatFlushStderr();


////////////////////////////////////////////////////////////
struct StdoutSink
{
    void append(const char* data, SizeT n)
    {
        formatWriteStdout(data, n);
    }
};


////////////////////////////////////////////////////////////
struct StderrSink
{
    void append(const char* data, SizeT n)
    {
        formatWriteStderr(data, n);
    }
};

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Args>
constexpr void assemble(FormatSink& sink, const priv::FmtSpan fmtStr, const Args&... args)
{
    // Size both arrays at `sizeof...(Args) + 1` so the empty-pack case still
    // produces a 1-element array (zero-initialized, never indexed because
    // `argCount == 0`). The trailing slot is wasted but unconditional --
    // collapses the dispatch into a single code path with no branch.
    const void* const                erasedArgs[sizeof...(Args) + 1]  = {&args...};
    constexpr priv::ErasedDispatchFn dispatchers[sizeof...(Args) + 1] = {&priv::erasedDispatch<Args>...};

    priv::formatAssembleImpl(sink, fmtStr, erasedArgs, dispatchers, sizeof...(Args));
}


////////////////////////////////////////////////////////////
template <typename... Args>
constexpr void FormatSink::format(typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args)
{
    assemble(*this, fmt.str, args...);
}


////////////////////////////////////////////////////////////
/// \brief Format into a fixed buffer.
///
/// Returns a pointer one past the last written character on success, or
/// `nullptr` on overflow. The buffer is **not** null-terminated -- callers
/// that need a C string must reserve an extra byte and write `'\0'` at
/// the returned pointer themselves.
///
/// Matches `fmt::format_to` / `std::format_to` semantics; differs from
/// `snprintf` in that overflow is hard-fail rather than truncate.
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] constexpr char* formatIntoBuffer(char* const                                            buffer,
                                               const SizeT                                            bufferSize,
                                               typename NonDeduced<const FormatString<Args...>>::type fmt,
                                               const Args&... args)
{
    FormatSink sink{buffer, buffer + bufferSize};
    assemble(sink, fmt.str, args...);

    if (sink.overflowed())
        return nullptr;

    return sink.position();
}


////////////////////////////////////////////////////////////
template <SizeT N, typename... Args>
[[nodiscard]] constexpr char* formatIntoBuffer(char (&buffer)[N],
                                               typename NonDeduced<const FormatString<Args...>>::type fmt,
                                               const Args&... args)
{
    return formatIntoBuffer(buffer, N, fmt, args...);
}


////////////////////////////////////////////////////////////
/// \brief Sink that accepts `append(const char*, SizeT)`.
////////////////////////////////////////////////////////////
template <typename T>
concept AppendSink = requires(T& sink, const char* p, SizeT n) { sink.append(p, n); };


////////////////////////////////////////////////////////////
/// \brief Format into any sink with `append(const char*, SizeT)`.
///
/// Tries to fit into a `formatToStagingSize`-byte stack buffer first;
/// on overflow falls back to a doubling heap buffer (so there is no
/// hard size limit). Either way, the user sink receives the formatted
/// output in a single `append` call.
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename... Args>
void formatTo(Sink& userSink, typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args)
{
    char       scratch[formatToStagingSize];
    FormatSink sink{scratch, scratch + sizeof(scratch)};

    // See `assemble`: trailing-sentinel sizing avoids the empty-pack branch.
    const void* const                erasedArgs[sizeof...(Args) + 1]  = {&args...};
    constexpr priv::ErasedDispatchFn dispatchers[sizeof...(Args) + 1] = {&priv::erasedDispatch<Args>...};

    priv::formatAssembleImpl(sink, fmt.str, erasedArgs, dispatchers, sizeof...(Args));

    if (!sink.overflowed()) [[likely]]
    {
        userSink.append(scratch, sink.size());
        return;
    }

    const auto appendFn = +[](void* s, const char* data, SizeT n) { static_cast<Sink*>(s)->append(data, n); };

    priv::formatToHeapFallback(&userSink, appendFn, fmt.str, erasedArgs, dispatchers, sizeof...(Args));
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stdout (no trailing newline; flushes
/// stdout). Use this for prompts and partial-line output.
////////////////////////////////////////////////////////////
template <typename... Args>
void print(typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args)
{
    priv::StdoutSink sink;
    formatTo(sink, fmt, args...);
    priv::formatFlushStdout();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stdout, followed by a newline.
/// No explicit flush -- on a TTY, '\n' triggers line-buffer flush;
/// on a piped stream, block buffering is preserved for throughput.
////////////////////////////////////////////////////////////
template <typename... Args>
void printLn(typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args)
{
    priv::StdoutSink sink;
    formatTo(sink, fmt, args...);
    priv::formatWriteStdoutNewline();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stderr (no trailing newline; flushes
/// stderr). Use this for partial-line error output.
////////////////////////////////////////////////////////////
template <typename... Args>
void printErr(typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args)
{
    priv::StderrSink sink;
    formatTo(sink, fmt, args...);
    priv::formatFlushStderr();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stderr, followed by a newline + flush.
////////////////////////////////////////////////////////////
template <typename... Args>
void printErrLn(typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args)
{
    priv::StderrSink sink;
    formatTo(sink, fmt, args...);
    priv::formatWriteStderrNewline();
    priv::formatFlushStderr();
}

} // namespace sf::base
