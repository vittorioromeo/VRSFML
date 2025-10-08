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
#include "SFML/Base/Vector.hpp"


namespace sfex
{
////////////////////////////////////////////////////////////
using NodeId = sf::base::SizeT;

////////////////////////////////////////////////////////////
inline constexpr auto   nullNode = static_cast<NodeId>(-1u);
inline constexpr NodeId maxNodes = 128u;

////////////////////////////////////////////////////////////
struct [[nodiscard]] ScopeInfo
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


namespace sfex::priv
{
////////////////////////////////////////////////////////////

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
        const NodeId id = nextNodeId++;
        SFML_BASE_ASSERT(id < maxNodes);

        nodes[id] = ScopeInfo{
            .label        = label,
            .file         = file,
            .func         = func,
            .line         = line,
            .timeUs       = 0,
            .nodeId       = id,
            .parentNodeId = nullNode,
            .depth        = currentDepth,
        };

        return nodes[id];
    }
};

////////////////////////////////////////////////////////////
inline thread_local Database tlDatabase;

} // namespace sfex::priv


namespace sfex
{
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

} // namespace sfex


namespace sfex
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline sf::base::Span<const ScopeInfo> getScopeInfos()
{
    return sf::base::Span<const ScopeInfo>{priv::tlDatabase.nodes, priv::tlDatabase.nextNodeId};
}

////////////////////////////////////////////////////////////
inline void populateNodes(sf::base::Span<const ScopeInfo>             scopeInfos,
                          sf::base::Vector<sf::base::Vector<NodeId>>& childrenMap,
                          sf::base::Vector<NodeId>&                   rootNodes)
{
    childrenMap.resize(maxNodes);
    for (auto& vec : childrenMap)
        vec.clear();

    rootNodes.clear();

    for (const auto& info : scopeInfos)
        if (info.parentNodeId == nullNode) // top-level node
            rootNodes.pushBack(info.nodeId);
        else // child node
            childrenMap[info.parentNodeId].pushBack(info.nodeId);
}

} // namespace sfex


////////////////////////////////////////////////////////////
#define SFEX_PRIV_CONCAT_TOKENS_IMPL(a, b) a##b

////////////////////////////////////////////////////////////
#define SFEX_PRIV_CONCAT_TOKENS(a, b) SFEX_PRIV_CONCAT_TOKENS_IMPL(a, b)

////////////////////////////////////////////////////////////
#define SFEX_PRIV_UNIQUE_NAME(name) SFEX_PRIV_CONCAT_TOKENS(name, __LINE__)

////////////////////////////////////////////////////////////
#define SFEX_PROFILE_SCOPE(label)                                                                       \
                                                                                                         \
    static thread_local auto& SFEX_PRIV_UNIQUE_NAME(                                                     \
        sfProfilerScopeInfo) = ::sfex::priv::tlDatabase.initNode((label), __FILE__, __func__, __LINE__); \
                                                                                                         \
    const ::sfex::ScopeGuard SFEX_PRIV_UNIQUE_NAME(sfProfilerScopeGuard)(SFEX_PRIV_UNIQUE_NAME(sfProfilerScopeInfo))


////////////////////////////////////////////////////////////
/// \class sf::Profiler
/// \ingroup system
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
