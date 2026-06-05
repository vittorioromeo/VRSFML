#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Chrono/Clock.hpp"
#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Vocabulary/Span.hpp"
#include "Zancle/String/StringView.hpp"


namespace za::profiler
{
////////////////////////////////////////////////////////////
struct ScopeInfo
{
    za::StringView label;
    za::StringView file;
    za::StringView func;
    za::StringView line;

    za::I64 timeUs;
};

} // namespace za::profiler


namespace za::profiler::priv
{
////////////////////////////////////////////////////////////
inline constexpr za::SizeT maxNodes = 128u;


////////////////////////////////////////////////////////////
struct [[nodiscard]] Database
{
    ////////////////////////////////////////////////////////////
    ScopeInfo   nodes[priv::maxNodes]{};
    za::SizeT nextNodeId = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] ScopeInfo& initNode(const za::StringView label,
                                      const za::StringView file,
                                      const za::StringView func,
                                      const za::StringView line)
    {
        const za::SizeT id = nextNodeId++;
        ZA_ASSERT(id < priv::maxNodes);

        nodes[id] = ScopeInfo{
            .label  = label,
            .file   = file,
            .func   = func,
            .line   = line,
            .timeUs = 0,
        };

        return nodes[id];
    }
};

////////////////////////////////////////////////////////////
inline thread_local Database tlDatabase;

} // namespace za::profiler::priv


namespace za::profiler
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] ScopeGuard
{
    ////////////////////////////////////////////////////////////
    ScopeInfo& scopeInfo;
    Time       startTime;

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] explicit ScopeGuard(ScopeInfo& theScopeInfo) :
        scopeInfo{theScopeInfo},
        startTime{Clock::now()}
    {
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~ScopeGuard()
    {
        scopeInfo.timeUs = (Clock::now() - startTime).asMicroseconds();
    }
};

} // namespace za::profiler


namespace za::profiler
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline za::Span<const ScopeInfo> getScopeInfos()
{
    return za::Span<const ScopeInfo>{priv::tlDatabase.nodes, priv::tlDatabase.nextNodeId};
}

} // namespace za::profiler


////////////////////////////////////////////////////////////
#define ZA_SYSTEM_PRIV_STRINGIZE(x)  ZA_SYSTEM_PRIV_STRINGIZE2(x)
#define ZA_SYSTEM_PRIV_STRINGIZE2(x) #x
#define ZA_SYSTEM_PRIV_LINE_STRING   ZA_SYSTEM_PRIV_STRINGIZE(__LINE__)

////////////////////////////////////////////////////////////
#define ZA_SYSTEM_PRIV_CONCAT_TOKENS_IMPL(a, b) a##b

////////////////////////////////////////////////////////////
#define ZA_SYSTEM_PRIV_CONCAT_TOKENS(a, b) ZA_SYSTEM_PRIV_CONCAT_TOKENS_IMPL(a, b)

////////////////////////////////////////////////////////////
#define ZA_SYSTEM_PRIV_UNIQUE_NAME(name) ZA_SYSTEM_PRIV_CONCAT_TOKENS(name, __LINE__)

////////////////////////////////////////////////////////////
#define ZA_SYSTEM_PROFILER_SCOPE(label)                                                                                            \
                                                                                                                                     \
    static thread_local auto& ZA_SYSTEM_PRIV_UNIQUE_NAME(                                                                          \
        sfProfilerScopeInfo) = ::za::profiler::priv::tlDatabase.initNode((label), __FILE__, __func__, ZA_SYSTEM_PRIV_LINE_STRING); \
                                                                                                                                     \
    const ::za::profiler::ScopeGuard ZA_SYSTEM_PRIV_UNIQUE_NAME(sfProfilerScopeGuard)(                                             \
        ZA_SYSTEM_PRIV_UNIQUE_NAME(sfProfilerScopeInfo))


////////////////////////////////////////////////////////////
/// \class za::Profiler
/// \ingroup system
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
