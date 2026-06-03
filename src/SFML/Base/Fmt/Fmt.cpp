// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Fmt/Fmt.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/FmtResult.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"

#include <cstdio>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
FmtResult fmtAssembleImpl(FmtSink&                 sink,
                          const FmtSpan            fmtStr,
                          const void* const* const args,
                          const ErasedDispatchFn*  dispatchers,
                          const SizeT              argCount)
{
    const auto*       p   = fmtStr.data;
    const auto* const end = p + fmtStr.size;

    SizeT argIndex = 0u;

    while (p < end)
    {
        // Batch-copy the literal run up to the next '{' or '}'. Escapes
        // (`{{` / `}}`) interrupt the run and are emitted one char at a
        // time below -- they are rare relative to plain literal text.
        const auto* const litStart = p;
        while (p < end && *p != '{' && *p != '}')
            ++p;

        if (p > litStart)
            SFML_BASE_FMT_TRY(sink.append(litStart, static_cast<SizeT>(p - litStart)));

        if (p >= end)
            break;

        if (*p == '{')
        {
            // Escaped '{{' -> '{'
            if (p + 1 < end && p[1] == '{')
            {
                SFML_BASE_FMT_TRY(sink.appendChar('{'));
                p += 2;
                continue;
            }

            // Consteval validation in `FmtString` should catch arg/spec
            // mismatches at compile time. These branches are defensive (e.g.
            // for recursive `sink.fmt(...)` calls that already passed
            // their own consteval).
            if (argIndex >= argCount)
            {
                SFML_BASE_FMT_TRY(sink.append("<arg?>", 6u));
                ++p;
                continue;
            }

            ++p; // skip '{'

            FmtSpec spec;
            if (p < end && *p == ':')
            {
                ++p; // skip ':'
                parseFmtSpec(p, end, spec);
            }

            if (p >= end || *p != '}')
            {
                SFML_BASE_FMT_TRY(sink.append("<spec?>", 7u));
                continue;
            }

            SFML_BASE_FMT_TRY(dispatchers[argIndex](sink, args[argIndex], spec));
            ++argIndex;
            ++p; // skip '}'
        }
        else // *p == '}'
        {
            // Escaped '}}' -> '}'. Consteval forbids a bare '}', so the
            // fallback (single '}') is defensive.
            SFML_BASE_FMT_TRY(sink.appendChar('}'));
            p += (p + 1 < end && p[1] == '}') ? 2 : 1;
        }
    }

    return FmtResult::Ok;
}


////////////////////////////////////////////////////////////
namespace
{
////////////////////////////////////////////////////////////
/// \brief Shared growing-buffer loop. Doubles capacity until `formatFn`
/// reports something other than `Overflow`. On success flushes the
/// owned buffer once through `userSink`; on `Failed` propagates without
/// flushing. `formatFn` is templated so the per-iteration call is
/// inlined into the loop.
////////////////////////////////////////////////////////////
template <typename FormatFn>
[[nodiscard, gnu::always_inline]] inline FmtResult heapFallbackLoop(FmtSinkRef userSink, FormatFn&& formatFn)
{
    String    buf;
    SizeT     cap     = fmtScratchSize * 2u;
    bool      success = false;
    FmtResult result  = FmtResult::Overflow;

    // Double the capacity until the format succeeds or a non-size-related
    // formatter failure is reported. `resizeAndOverwrite` reserves exactly
    // `cap` bytes; the lambda writes into them and reports the final size.
    // RAII keeps the buffer owned by `buf`.
    while (result == FmtResult::Overflow)
    {
        success = false;
        buf.resizeAndOverwrite(cap,
                               [&](char* const data, const SizeT capacity) -> SizeT
        {
            FmtSink sink{data, data + capacity};
            result = formatFn(sink);

            if (result != FmtResult::Ok)
                return 0u;

            success = true;
            return sink.size();
        });

        if (result == FmtResult::Overflow)
        {
            if (cap > static_cast<SizeT>(-1) / 2u)
                return FmtResult::Failed;

            cap *= 2u;
        }
    }

    if (!success)
        return result;

    return userSink.append(buf.data(), buf.size());
}

} // namespace


////////////////////////////////////////////////////////////
FmtResult fmtToHeapFallback(const FmtSinkRef              userSink,
                            const FmtSpan                 fmtStr,
                            const void* const* const      args,
                            const ErasedDispatchFn* const dispatchers,
                            const SizeT                   argCount)
{
    return heapFallbackLoop(userSink, [&](FmtSink& sink) {
        return fmtAssembleImpl(sink, fmtStr, args, dispatchers, argCount);
    });
}


////////////////////////////////////////////////////////////
FmtResult fmtArgToHeapFallback(const FmtSinkRef userSink, const void* const erasedArg, const ErasedDispatchFn dispatcher)
{
    return heapFallbackLoop(userSink, [&](FmtSink& sink) { return dispatcher(sink, erasedArg, FmtSpec{}); });
}


////////////////////////////////////////////////////////////
void fmtWriteStdout(const char* const data, const SizeT size)
{
    std::fwrite(data, 1, size, stdout);
}


////////////////////////////////////////////////////////////
void fmtWriteStdoutNewline()
{
    std::fputc('\n', stdout);
}


////////////////////////////////////////////////////////////
void fmtFlushStdout()
{
    std::fflush(stdout);
}


////////////////////////////////////////////////////////////
void fmtWriteStderr(const char* const data, const SizeT size)
{
    std::fwrite(data, 1, size, stderr);
}


////////////////////////////////////////////////////////////
void fmtWriteStderrNewline()
{
    std::fputc('\n', stderr);
}


////////////////////////////////////////////////////////////
void fmtFlushStderr()
{
    std::fflush(stderr);
}

} // namespace sf::base::priv
