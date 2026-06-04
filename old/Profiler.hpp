#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Clock.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Span.hpp"
#include "ZancleBase/StringView.hpp"


namespace za::profiler
{
////////////////////////////////////////////////////////////
struct ScopeInfo
{
    zb::StringView label;
    zb::StringView file;
    zb::StringView func;
    zb::StringView line;

    zb::I64 timeUs;
};

} // namespace za::profiler


namespace za::profiler::priv
{
////////////////////////////////////////////////////////////
inline constexpr zb::SizeT maxNodes = 128u;


////////////////////////////////////////////////////////////
struct [[nodiscard]] Database
{
    ////////////////////////////////////////////////////////////
    ScopeInfo   nodes[priv::maxNodes]{};
    zb::SizeT nextNodeId = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] ScopeInfo& initNode(const zb::StringView label,
                                      const zb::StringView file,
                                      const zb::StringView func,
                                      const zb::StringView line)
    {
        const zb::SizeT id = nextNodeId++;
        ZB_ASSERT(id < priv::maxNodes);

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
[[nodiscard, gnu::always_inline]] inline zb::Span<const ScopeInfo> getScopeInfos()
{
    return zb::Span<const ScopeInfo>{priv::tlDatabase.nodes, priv::tlDatabase.nextNodeId};
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
