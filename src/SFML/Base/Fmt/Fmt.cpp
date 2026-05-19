// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Fmt/Fmt.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"

#include <cstdio>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void fmtAssembleImpl(FmtSink&                 sink,
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
            sink.append(litStart, static_cast<SizeT>(p - litStart));

        if (p >= end)
            break;

        if (*p == '{')
        {
            // Escaped '{{' -> '{'
            if (p + 1 < end && p[1] == '{')
            {
                sink.appendChar('{');
                p += 2;
                continue;
            }

            // Consteval validation in `FmtString` should catch arg/spec
            // mismatches at compile time. These branches are defensive (e.g.
            // for recursive `sink.fmt(...)` calls that already passed
            // their own consteval).
            if (argIndex >= argCount)
            {
                sink.append("<arg?>", 6u);
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
                sink.append("<spec?>", 7u);
                continue;
            }

            dispatchers[argIndex](sink, args[argIndex], spec);
            ++argIndex;
            ++p; // skip '}'
        }
        else // *p == '}'
        {
            // Escaped '}}' -> '}'. Consteval forbids a bare '}', so the
            // fallback (single '}') is defensive.
            sink.appendChar('}');
            p += (p + 1 < end && p[1] == '}') ? 2 : 1;
        }
    }
}


////////////////////////////////////////////////////////////
void fmtToHeapFallback(void* const userSink,
                       void (*appendFn)(void*, const char*, SizeT),
                       const FmtSpan                 fmtStr,
                       const void* const* const      args,
                       const ErasedDispatchFn* const dispatchers,
                       const SizeT                   argCount)
{
    String buf;
    SizeT  cap     = fmtScratchSize * 2u;
    bool   success = false;

    // Double the capacity until the format succeeds. `resizeAndOverwrite`
    // reserves exactly `cap` bytes; the lambda writes into them and reports
    // the final size (0 signals "didn't fit, please grow"). RAII keeps the
    // buffer owned by `buf`.
    while (!success)
    {
        buf.resizeAndOverwrite(cap,
                               [&](char* const data, const SizeT capacity) -> SizeT
        {
            FmtSink sink{data, data + capacity};
            fmtAssembleImpl(sink, fmtStr, args, dispatchers, argCount);

            if (sink.overflowed())
                return 0u;

            success = true;
            return sink.size();
        });

        if (!success)
            cap *= 2u;
    }

    appendFn(userSink, buf.data(), buf.size());
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
