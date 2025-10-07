#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Span.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf::profiler
{
////////////////////////////////////////////////////////////
struct ScopeInfo
{
    base::StringView label;
    base::StringView file;
    base::StringView func;
    base::StringView line;

    base::I64 timeUs;
};

} // namespace sf::profiler


namespace sf::profiler::priv
{
////////////////////////////////////////////////////////////
inline constexpr base::SizeT maxNodes = 128u;


////////////////////////////////////////////////////////////
struct [[nodiscard]] Database
{
    ////////////////////////////////////////////////////////////
    ScopeInfo   nodes[priv::maxNodes]{};
    base::SizeT nextNodeId = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] ScopeInfo& initNode(const base::StringView label,
                                      const base::StringView file,
                                      const base::StringView func,
                                      const base::StringView line)
    {
        const base::SizeT id = nextNodeId++;
        SFML_BASE_ASSERT(id < priv::maxNodes);

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

} // namespace sf::profiler::priv


namespace sf::profiler
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

} // namespace sf::profiler


namespace sf::profiler
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline base::Span<const ScopeInfo> getScopeInfos()
{
    return base::Span<const ScopeInfo>{priv::tlDatabase.nodes, priv::tlDatabase.nextNodeId};
}

} // namespace sf::profiler


////////////////////////////////////////////////////////////
#define SFML_SYSTEM_PRIV_STRINGIZE(x)  SFML_SYSTEM_PRIV_STRINGIZE2(x)
#define SFML_SYSTEM_PRIV_STRINGIZE2(x) #x
#define SFML_SYSTEM_PRIV_LINE_STRING   SFML_SYSTEM_PRIV_STRINGIZE(__LINE__)

////////////////////////////////////////////////////////////
#define SFML_SYSTEM_PRIV_CONCAT_TOKENS_IMPL(a, b) a##b

////////////////////////////////////////////////////////////
#define SFML_SYSTEM_PRIV_CONCAT_TOKENS(a, b) SFML_SYSTEM_PRIV_CONCAT_TOKENS_IMPL(a, b)

////////////////////////////////////////////////////////////
#define SFML_SYSTEM_PRIV_UNIQUE_NAME(name) SFML_SYSTEM_PRIV_CONCAT_TOKENS(name, __LINE__)

////////////////////////////////////////////////////////////
#define SFML_SYSTEM_PROFILER_SCOPE(label)                                                                                            \
                                                                                                                                     \
    static thread_local auto& SFML_SYSTEM_PRIV_UNIQUE_NAME(                                                                          \
        sfProfilerScopeInfo) = ::sf::profiler::priv::tlDatabase.initNode((label), __FILE__, __func__, SFML_SYSTEM_PRIV_LINE_STRING); \
                                                                                                                                     \
    const ::sf::profiler::ScopeGuard SFML_SYSTEM_PRIV_UNIQUE_NAME(sfProfilerScopeGuard)(                                             \
        SFML_SYSTEM_PRIV_UNIQUE_NAME(sfProfilerScopeInfo))


////////////////////////////////////////////////////////////
/// \class sf::Profiler
/// \ingroup system
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
