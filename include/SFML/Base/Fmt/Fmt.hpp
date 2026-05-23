#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Memmove.hpp"
#include "SFML/Base/Builtin/Memset.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Fmt/FmtArgDefaultAlign.hpp" // IWYU pragma: export
#include "SFML/Base/Fmt/FmtCString.hpp"         // IWYU pragma: export
#include "SFML/Base/Fmt/FmtResult.hpp"
#include "SFML/Base/Fmt/FmtSink.hpp"    // IWYU pragma: export
#include "SFML/Base/Fmt/FmtSinkRef.hpp" // IWYU pragma: export
#include "SFML/Base/Fmt/FmtSpan.hpp"    // IWYU pragma: export
#include "SFML/Base/Fmt/FmtSpec.hpp"    // IWYU pragma: export
#include "SFML/Base/Fmt/FmtString.hpp"  // IWYU pragma: export
#include "SFML/Base/NonDeduced.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


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
/// \brief Per-format-string properties extracted at consteval.
///
/// `placeholderCount` is matched against `sizeof...(Args)` for the count
/// invariant. `hasEscapes` is set when the parser steps over a `{{` or
/// `}}` -- the runtime needs to know about these because they require
/// `{` / `}` translation and disqualify the literal-only fast path.
////////////////////////////////////////////////////////////
struct FmtStringInfo
{
    SizeT placeholderCount;
    bool  hasEscapes;
};


////////////////////////////////////////////////////////////
/// \brief Walk the format string once, validating each placeholder and
/// reporting placeholder count + escape presence.
///
/// Throws (at consteval) on:
///   - unclosed `{`
///   - unescaped `}`
///   - malformed spec body (anything that leaves the parser short of `}`)
///   - width >= 65536 or precision > 10
///   - unknown type tag (anything outside `d` / `x` / `X` / `o` / `b` / `f`)
////////////////////////////////////////////////////////////
[[nodiscard]] consteval FmtStringInfo analyzeFmtString(const FmtSpan fmtStr)
{
    const auto*       p   = fmtStr.data;
    const auto* const end = p + fmtStr.size;

    FmtStringInfo info{0u, false};

    while (p < end)
    {
        if (*p == '{')
        {
            // Escaped '{{' -> '{'
            if (p + 1 < end && p[1] == '{')
            {
                info.hasEscapes = true;
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
            ++info.placeholderCount;
        }
        else if (*p == '}')
        {
            // Escaped '}}' -> '}'
            if (p + 1 < end && p[1] == '}')
            {
                info.hasEscapes = true;
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

    return info;
}

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Compile-time-validated format string: ensures placeholder
/// count matches `sizeof...(Args)` and exposes the "pure-literal"
/// property used by the runtime fast path.
///
/// `isPureLiteral` is true when the string carries no placeholders AND
/// no `{{` / `}}` escapes -- in that case the runtime can skip the whole
/// assemble loop and write the bytes directly to the sink.
////////////////////////////////////////////////////////////
template <typename... Args>
struct [[nodiscard]] FmtString
{
    priv::FmtSpan str;
    bool          isPureLiteral = false;

    consteval FmtString(const char* const s) : str{s, SFML_BASE_STRLEN(s)}
    {
        const priv::FmtStringInfo info = priv::analyzeFmtString(str);

        if (info.placeholderCount != sizeof...(Args))
            throw "Mismatch between number of '{}' and number of arguments.";

        isPureLiteral = (info.placeholderCount == 0u) && !info.hasEscapes;
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
[[nodiscard]] constexpr FmtResult dispatchFmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec) noexcept
{
    const auto startMark = sink.mark();

    SFML_BASE_FMT_TRY(fmtArg(sink, arg, spec)); // ADL

    if (spec.width <= 0)
        return FmtResult::ok;

    const auto contentSize = sink.mark() - startMark;
    if (static_cast<SizeT>(spec.width) <= contentSize)
        return FmtResult::ok;

    const auto padTotal = static_cast<SizeT>(spec.width) - contentSize;
    SFML_BASE_FMT_TRY(sink.ensureRoom(padTotal));

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
    return FmtResult::ok;
}


////////////////////////////////////////////////////////////
using ErasedDispatchFn = FmtResult (*)(FmtSink&, const void*, const FmtSpec&);


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr FmtResult dispatchFmtArgErased(FmtSink& sink, const void* const erasedArg, const FmtSpec& spec) noexcept
{
    return dispatchFmtArg(sink, *static_cast<const T*>(erasedArg), spec);
}


////////////////////////////////////////////////////////////
/// \brief Type-erased core. Walks `fmtStr` once, copying literals and
/// dispatching `{...}` placeholders via `dispatchers`.
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API FmtResult
    fmtAssembleImpl(FmtSink& sink, FmtSpan fmtStr, const void* const* args, const ErasedDispatchFn* dispatchers, SizeT argCount);


////////////////////////////////////////////////////////////
/// \brief Heap-fallback for `fmtTo` / `print`. Allocates a doubling
/// `base::String` until the format succeeds, then flushes once
/// through `userSink`.
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API FmtResult fmtToHeapFallback(
    FmtSinkRef              userSink,
    FmtSpan                 fmtStr,
    const void* const*      args,
    const ErasedDispatchFn* dispatchers,
    SizeT                   argCount);


////////////////////////////////////////////////////////////
/// \brief Heap-fallback for `fmtArgTo`. Same growing-buffer strategy
/// as `fmtToHeapFallback`, but calls a single `dispatcher` instead of
/// walking a format string.
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API FmtResult
    fmtArgToHeapFallback(FmtSinkRef userSink, const void* erasedArg, ErasedDispatchFn dispatcher);


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
[[nodiscard]] constexpr FmtResult fmtAssemble(FmtSink& sink, const FmtSpan fmtStr, const Args&... args)
{
    // Size both arrays at `sizeof...(Args) + 1` so the empty-pack case still
    // produces a 1-element array (zero-initialized, never indexed because
    // `argCount == 0`). The trailing slot is wasted but unconditional --
    // collapses the dispatch into a single code path with no branch.
    const void* const          erasedArgs[sizeof...(Args) + 1]  = {&args...};
    constexpr ErasedDispatchFn dispatchers[sizeof...(Args) + 1] = {&dispatchFmtArgErased<Args>...};

    return fmtAssembleImpl(sink, fmtStr, erasedArgs, dispatchers, sizeof...(Args));
}

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Args>
constexpr FmtResult FmtSink::fmt(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    return priv::fmtAssemble(*this, fmtStr.str, args...);
}


////////////////////////////////////////////////////////////
/// \brief Format into a fixed buffer.
///
/// Returns a pointer one past the last written character on success, or
/// `nullptr` on overflow / formatter failure. The buffer is **not**
/// null-terminated -- callers that need a C string must reserve an extra
/// byte and write `'\0'` at the returned pointer themselves. On failure the
/// buffer contents are indeterminate (may hold a partial write).
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
    // Fast path: literal-only string skips the whole assemble loop.
    if constexpr (sizeof...(Args) == 0)
    {
        if (fmtStr.isPureLiteral) [[likely]]
        {
            if (bufferSize < fmtStr.str.size)
                return nullptr;
            SFML_BASE_MEMCPY(buffer, fmtStr.str.data, fmtStr.str.size);
            return buffer + fmtStr.str.size;
        }
    }

    FmtSink sink{buffer, buffer + bufferSize};
    if (priv::fmtAssemble(sink, fmtStr.str, args...) != FmtResult::ok)
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

} // namespace sf::base


////////////////////////////////////////////////////////////
namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Shared scratch-buffer + heap-fallback protocol for `fmtTo`-family
/// functions.
///
/// Templated on the sink type so the success-path flush is a direct,
/// inlined `userSink.append(...)` call (static dispatch). `FmtSinkRef`
/// satisfies `AppendSink`, so opting into dynamic dispatch just means
/// passing one explicitly.
///
/// On `overflow`, hands off to `heapFn(FmtSinkRef{userSink})` -- the
/// heap fallback lives out-of-line in `.cpp` and can't be templated on
/// `Sink` without dragging `String` into this header, so it always
/// accepts a type-erased ref. That indirect call only fires when the
/// scratch buffer overflows, which is rare.
///
/// `scratchFn` and `heapFn` are passed as templates so the lambdas are
/// fully inlined into the caller.
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename ScratchFn, typename HeapFn>
[[nodiscard, gnu::always_inline]] inline FmtResult fmtViaScratchOrHeap(Sink& userSink, ScratchFn&& scratchFn, HeapFn&& heapFn)
{
    char    scratch[fmtScratchSize];
    FmtSink sink{scratch, scratch + sizeof(scratch)};

    const FmtResult result = scratchFn(sink);

    if (result == FmtResult::ok) [[likely]]
    {
        userSink.append(scratch, sink.size());
        return FmtResult::ok;
    }

    if (result == FmtResult::failed)
        return FmtResult::failed;

    return heapFn(FmtSinkRef{userSink});
}

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
// Forward-declare `String` so the generic `fmtTo` below can exclude it
// from overload resolution (the dedicated `String&` overload further
// down has a different return type / nodiscard policy). Including
// `<SFML/Base/String.hpp>` here would defeat the whole header-tier story.
////////////////////////////////////////////////////////////
class String;

} // namespace sf::base


////////////////////////////////////////////////////////////
namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Shared implementation for both `fmtTo` overloads.
///
/// Lives in `priv` so the dedicated `String&` overload can delegate here
/// without going through the constrained public `fmtTo` (which excludes
/// `Sink == String` to disambiguate overload resolution). Templated on
/// `Sink`, so `userSink.append(...)` is a *dependent* member access ->
/// only checked at instantiation time -> safe with forward-declared
/// `String` at the public overload's site.
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename... Args>
FmtResult fmtToImpl(Sink& userSink, typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    // Fast path: literal-only string bypasses the scratch + assemble loop
    // and writes the bytes straight to the sink in one call.
    if constexpr (sizeof...(Args) == 0)
    {
        if (fmtStr.isPureLiteral) [[likely]]
        {
            userSink.append(fmtStr.str.data, fmtStr.str.size);
            return FmtResult::ok;
        }
    }

    // See `priv::fmtAssemble`: trailing-sentinel sizing avoids the empty-pack branch.
    const void* const                erasedArgs[sizeof...(Args) + 1]  = {&args...};
    constexpr priv::ErasedDispatchFn dispatchers[sizeof...(Args) + 1] = {&priv::dispatchFmtArgErased<Args>...};

    return priv::fmtViaScratchOrHeap(userSink, [&](FmtSink& sink) {
        return priv::fmtAssembleImpl(sink, fmtStr.str, erasedArgs, dispatchers, sizeof...(Args));
    }, [&](FmtSinkRef us) { return priv::fmtToHeapFallback(us, fmtStr.str, erasedArgs, dispatchers, sizeof...(Args)); });
}

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Format into any sink with `append(const char*, SizeT)`.
///
/// Templated on `Sink` so the success path uses static dispatch
/// (`userSink.append(...)` is a direct, inlined call). Users who want
/// dynamic dispatch instead pass an `FmtSinkRef` (which itself satisfies
/// `AppendSink`); the same template handles both, but the indirect call
/// only happens for the type-erased case.
///
/// Tries to fit into a `fmtScratchSize`-byte stack buffer first; on
/// overflow falls back to a doubling heap buffer (no hard size limit).
/// Formatter failures are returned immediately. On success the sink
/// receives the formatted output in a single `append` call.
///
/// The `String` sink is excluded here -- see the dedicated `void`-returning
/// overload below. Without this constraint, both overloads are equally
/// viable for a `String&` argument (partial ordering ties for an empty
/// `Args...` pack).
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename... Args>
    requires(!SFML_BASE_IS_SAME(Sink, String))
[[nodiscard]] FmtResult fmtTo(Sink& userSink, typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    return priv::fmtToImpl<Sink, Args...>(userSink, fmtStr, args...);
}


////////////////////////////////////////////////////////////
/// \brief `fmtTo` overload for `String` sinks: returns `void`.
///
/// `String` grows on demand, so the only failure modes the generic overload
/// can produce -- "ran out of room" on the scratch / heap fallback -- can't
/// happen here. Returning `void` removes the `[[nodiscard]] FmtResult` tax
/// (no `(void)` cast at every "build a debug string" call site) and matches
/// the use case: you're not going to react to a failure that can't occur.
///
/// Out-of-contract user `fmtArg` overloads that return `FmtResult::failed`
/// still abort the formatting (no partial write to the sink), but the
/// signal is swallowed here. Callers that need to observe formatter-side
/// failures should drop down to the generic `fmtTo(Sink&, ...)` above
/// (e.g. wrap their `String&` as an `AppendSink`-conforming local).
////////////////////////////////////////////////////////////
template <typename... Args>
void fmtTo(String& userSink, typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    // Delegate to the shared `priv::fmtToImpl`, which is templated on `Sink`
    // so `userSink.append(...)` is a dependent member access -- only checked
    // when this overload is instantiated (where `<SFML/Base/String.hpp>` is
    // already in scope). The forward declaration of `String` in this header
    // is sufficient because we only *name* the type here, never access members.
    (void)priv::fmtToImpl<String, Args...>(userSink, fmtStr, args...);
}


////////////////////////////////////////////////////////////
/// \brief Stringify a single value directly into a sink, bypassing the
/// format-string machinery.
///
/// Calls the ADL `fmtArg(sink, value, {})` once into a `fmtScratchSize`
/// scratch buffer; on overflow falls back to the same doubling heap
/// strategy as `fmtTo`. Skips placeholder parsing, the erased-dispatch
/// table, and `fmtAssembleImpl` entirely -- intended for hot per-element
/// stringification (e.g. `concat(a, b, c)`).
////////////////////////////////////////////////////////////
template <AppendSink Sink, typename T>
[[nodiscard]] FmtResult fmtArgTo(Sink& userSink, const T& value)
{
    return priv::fmtViaScratchOrHeap(userSink,
                                     [&](FmtSink& sink) { return fmtArg(sink, value, FmtSpec{}); }, // ADL
                                     [&](FmtSinkRef us)
    { return priv::fmtArgToHeapFallback(us, &value, &priv::dispatchFmtArgErased<T>); });
}


////////////////////////////////////////////////////////////
/// \brief `fmtArgTo` overload for `String` sinks: returns `void`. Same
/// rationale as the `fmtTo(String&, ...)` overload above.
////////////////////////////////////////////////////////////
template <typename T>
void fmtArgTo(String& userSink, const T& value)
{
    (void)fmtArgTo<String, T>(userSink, value);
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stdout (no trailing newline; flushes
/// stdout). Use this for prompts and partial-line output.
////////////////////////////////////////////////////////////
template <typename... Args>
void print(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        // Fast path: literal-only string goes straight to stdio, skipping the
        // 512-byte scratch buffer + assemble loop + final memcpy.
        if (fmtStr.isPureLiteral) [[likely]]
        {
            priv::fmtWriteStdout(fmtStr.str.data, fmtStr.str.size);
            priv::fmtFlushStdout();
            return;
        }
    }

    priv::StdoutSink sink;
    (void)fmtTo(sink, fmtStr, args...);
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
    if constexpr (sizeof...(Args) == 0)
    {
        if (fmtStr.isPureLiteral) [[likely]]
        {
            priv::fmtWriteStdout(fmtStr.str.data, fmtStr.str.size);
            priv::fmtWriteStdoutNewline();
            return;
        }
    }

    priv::StdoutSink sink;
    if (fmtTo(sink, fmtStr, args...) == FmtResult::ok)
        priv::fmtWriteStdoutNewline();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stderr (no trailing newline; flushes
/// stderr). Use this for partial-line error output.
////////////////////////////////////////////////////////////
template <typename... Args>
void printErr(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        if (fmtStr.isPureLiteral) [[likely]]
        {
            priv::fmtWriteStderr(fmtStr.str.data, fmtStr.str.size);
            priv::fmtFlushStderr();
            return;
        }
    }

    priv::StderrSink sink;
    (void)fmtTo(sink, fmtStr, args...);
    priv::fmtFlushStderr();
}


////////////////////////////////////////////////////////////
/// \brief Format and write to stderr, followed by a newline + flush.
////////////////////////////////////////////////////////////
template <typename... Args>
void printErrLn(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        if (fmtStr.isPureLiteral) [[likely]]
        {
            priv::fmtWriteStderr(fmtStr.str.data, fmtStr.str.size);
            priv::fmtWriteStderrNewline();
            priv::fmtFlushStderr();
            return;
        }
    }

    priv::StderrSink sink;
    if (fmtTo(sink, fmtStr, args...) == FmtResult::ok)
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
    (void)fmtTo(sink, fmtStr, args...);
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
    if (fmtTo(sink, fmtStr, args...) == FmtResult::ok)
        sink.append("\n", 1u);
}

} // namespace sf::base
