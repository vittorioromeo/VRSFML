#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Span.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#ifdef SFEX_PROFILER_ENABLED
    #include "SFML/System/Clock.hpp"
    #include "SFML/System/IO.hpp"
    #include "SFML/System/Time.hpp"

    #include "SFML/Base/Abort.hpp"
    #include "SFML/Base/Fmt/Fmt.hpp"
    #include "SFML/Base/Fmt/FmtNumeric.hpp"
#endif


namespace sfex
{
////////////////////////////////////////////////////////////
using NodeId = sf::base::SizeT;


////////////////////////////////////////////////////////////
inline constexpr auto          nullNode = static_cast<NodeId>(-1u);
inline constexpr NodeId        maxNodes = 512u;
inline constexpr sf::base::I64 nullTime = -1;


////////////////////////////////////////////////////////////
struct [[nodiscard]] ScopeInfo // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    sf::base::StringView label;
    sf::base::StringView file;
    sf::base::StringView func;

    int line;

    sf::base::I64 timeUs;

    NodeId nodeId;
    NodeId parentNodeId;

    sf::base::SizeT depth;
};

} // namespace sfex


#ifdef SFEX_PROFILER_ENABLED

namespace sfex::priv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] Database
{
    ////////////////////////////////////////////////////////////
    ScopeInfo nodes[maxNodes]{};

    NodeId nextNodeId    = 0u;
    NodeId currentNodeId = nullNode;

    sf::base::SizeT currentDepth = 0u;


    ////////////////////////////////////////////////////////////
    [[nodiscard]] ScopeInfo& initNode(const sf::base::StringView label,
                                      const sf::base::StringView file,
                                      const sf::base::StringView func,
                                      const int                  line)
    {
        if (nextNodeId >= maxNodes) [[unlikely]]
        {
            sf::base::printErrLn("SFEX Profiler: exceeded maximum number of nodes ({})", maxNodes);
            sf::base::abort();
        }

        const NodeId id = nextNodeId++;

        nodes[id] = ScopeInfo{
            .label        = label,
            .file         = file,
            .func         = func,
            .line         = line,
            .timeUs       = nullTime,
            .nodeId       = id,
            .parentNodeId = nullNode,
            .depth        = currentDepth,
        };

        return nodes[id];
    }
};


////////////////////////////////////////////////////////////
inline thread_local Database tlDatabase;


////////////////////////////////////////////////////////////
struct [[nodiscard]] ScopeGuard
{
    ////////////////////////////////////////////////////////////
    ScopeInfo& scopeInfo;
    sf::Time   startTime;
    NodeId     previousNodeId;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] explicit ScopeGuard(ScopeInfo& theScopeInfo) :
        scopeInfo{theScopeInfo},
        startTime{sf::Clock::now()},
        previousNodeId{priv::tlDatabase.currentNodeId}
    {
        auto& db = priv::tlDatabase;

        scopeInfo.parentNodeId = db.currentNodeId;
        db.currentNodeId       = scopeInfo.nodeId;
        db.currentDepth        = scopeInfo.depth + 1u;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~ScopeGuard()
    {
        auto& db = priv::tlDatabase;

        scopeInfo.timeUs = (sf::Clock::now() - startTime).asMicroseconds();
        db.currentNodeId = previousNodeId;
        db.currentDepth  = scopeInfo.depth;
    }
};

} // namespace sfex::priv

#endif

namespace sfex
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline sf::base::Span<const ScopeInfo> getScopeInfos()
{
#ifdef SFEX_PROFILER_ENABLED
    return sf::base::Span<const ScopeInfo>{priv::tlDatabase.nodes, priv::tlDatabase.nextNodeId};
#else
    return {};
#endif
}

} // namespace sfex


////////////////////////////////////////////////////////////
// Linker-enforced configuration check.
//
// `SFEX_PROFILER_ENABLED` must be consistenly defined.
////////////////////////////////////////////////////////////
namespace sfex::priv
{
#ifdef SFEX_PROFILER_ENABLED
void sfexProfilerLinkCheckEnabledOnly();
#else
void sfexProfilerLinkCheckDisabledOnly();
#endif
} // namespace sfex::priv


////////////////////////////////////////////////////////////
// Force-reference the macro-state-dependent tag from every TU that includes
// this header. The address-of inside the initializer creates a relocation
// against the symbol, so an unresolved reference is reported by the linker.
// `[[gnu::used]]` keeps the variable from being optimized away even though
// nothing in user code reads it. The anonymous namespace gives each TU its
// own internal-linkage copy, sidestepping ODR concerns.
////////////////////////////////////////////////////////////
namespace
{
[[gnu::used, maybe_unused]] const auto sfexProfilerLinkCheckRef =
#ifdef SFEX_PROFILER_ENABLED
    &::sfex::priv::sfexProfilerLinkCheckEnabledOnly;
#else
    &::sfex::priv::sfexProfilerLinkCheckDisabledOnly;
#endif
} // namespace


namespace sfex
{


////////////////////////////////////////////////////////////
// TODO P1: nicer interface?
inline void populateNodes([[maybe_unused]] sf::base::Span<const ScopeInfo>             scopeInfos,
                          [[maybe_unused]] sf::base::Vector<sf::base::Vector<NodeId>>& childrenMap,
                          [[maybe_unused]] sf::base::Vector<NodeId>&                   rootNodes)
{
#ifdef SFEX_PROFILER_ENABLED
    childrenMap.resize(maxNodes);

    for (auto& vec : childrenMap)
        vec.clear();

    rootNodes.clear();

    for (const auto& info : scopeInfos)
    {
        if (info.timeUs == nullTime)
            continue;

        if (info.parentNodeId == nullNode) // top-level node
            rootNodes.pushBack(info.nodeId);
        else // child node
            childrenMap[info.parentNodeId].pushBack(info.nodeId);
    }
#endif
}


////////////////////////////////////////////////////////////
inline void resetNodes()
{
#ifdef SFEX_PROFILER_ENABLED
    for (auto& node : priv::tlDatabase.nodes)
        node.timeUs = nullTime;
#endif
}

} // namespace sfex


////////////////////////////////////////////////////////////
#define SFEX_PRIV_STRINGIFY(a) #a


////////////////////////////////////////////////////////////
#define SFEX_PRIV_CONCAT_TOKENS_IMPL(a, b) a##b


////////////////////////////////////////////////////////////
#define SFEX_PRIV_CONCAT_TOKENS(a, b) SFEX_PRIV_CONCAT_TOKENS_IMPL(a, b)


////////////////////////////////////////////////////////////
#define SFEX_PRIV_UNIQUE_NAME(name) SFEX_PRIV_CONCAT_TOKENS(name, __LINE__)


#ifdef SFEX_PROFILER_ENABLED

    ////////////////////////////////////////////////////////////
    #define SFEX_PROFILE_SCOPE(label)                                                                        \
                                                                                                             \
        static thread_local auto& SFEX_PRIV_UNIQUE_NAME(                                                     \
            sfProfilerScopeInfo) = ::sfex::priv::tlDatabase.initNode((label), __FILE__, __func__, __LINE__); \
                                                                                                             \
        const ::sfex::priv::ScopeGuard SFEX_PRIV_UNIQUE_NAME(sfProfilerScopeGuard)(                          \
            SFEX_PRIV_UNIQUE_NAME(sfProfilerScopeInfo))

    ////////////////////////////////////////////////////////////
    #define SFEX_PROFILE_SCOPE_AUTOLABEL() SFEX_PROFILE_SCOPE(__func__)

#else

    ////////////////////////////////////////////////////////////
    #define SFEX_PROFILE_SCOPE(label)      (void)0

    ////////////////////////////////////////////////////////////
    #define SFEX_PROFILE_SCOPE_AUTOLABEL() (void)0

#endif


////////////////////////////////////////////////////////////
/// \class sf::Profiler
/// \ingroup system
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
