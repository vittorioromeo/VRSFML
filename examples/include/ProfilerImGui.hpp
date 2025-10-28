#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?
#include "Profiler.hpp"

#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Span.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>


namespace sfex::priv
{
////////////////////////////////////////////////////////////
using ChildrenMap = sf::base::Vector<sf::base::Vector<sfex::NodeId>>;


////////////////////////////////////////////////////////////
template <typename T>
using SamplerVec = sf::base::Vector<Sampler<T>>;


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const, gnu::always_inline]] inline double calcPercentage(const sf::base::I64 part, const sf::base::I64 total)
{
    return total == 0 ? 0.0 : (static_cast<double>(part) * 100.0) / static_cast<double>(total);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::pure, gnu::always_inline]] inline double calcNodePercentage(const auto& scopeInfos, const sfex::ScopeInfo& info)
{
    return info.parentNodeId == sfex::nullNode ? 0.0 : calcPercentage(info.timeUs, scopeInfos[info.parentNodeId].timeUs);
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline int calcDelta(const SamplerVec<sf::base::U64>& timeSamplers,
                                   const SamplerVec<double>&        percentSamplers,
                                   const sfex::ScopeInfo&           infoA,
                                   const sfex::ScopeInfo&           infoB,
                                   const unsigned int               columnUserID)
{
    if (columnUserID == 0u) // Sort by label
        return infoA.label.compare(infoB.label);

    if (columnUserID == 1u) // Sort by time
    {
        const auto timeA = timeSamplers[infoA.nodeId].getAverageAs<double>();
        const auto timeB = timeSamplers[infoB.nodeId].getAverageAs<double>();

        return (timeA > timeB) ? 1 : (timeA < timeB) ? -1 : 0;
    }

    if (columnUserID == 2u) // Sort by percent
    {
        const auto percentA = percentSamplers[infoA.nodeId].getAverageAs<double>();
        const auto percentB = percentSamplers[infoB.nodeId].getAverageAs<double>();

        return (percentA > percentB) ? 1 : (percentA < percentB) ? -1 : 0;
    }

    if (columnUserID == 3u) // Sort by location
    {
        // Compare by file name first, then by line number
        const int delta = infoA.file.compare(infoB.file);
        return delta == 0 ? infoA.line - infoB.line : delta;
    }

    return 0;
}


////////////////////////////////////////////////////////////
inline void renderNode(const SamplerVec<sf::base::U64>&             timeSamplers,
                       const SamplerVec<double>&                    percentSamplers,
                       sfex::NodeId                                 nodeId,
                       const sf::base::Span<const sfex::ScopeInfo>& allNodes,
                       const ChildrenMap&                           childrenMap)
{
    const auto& info     = allNodes[nodeId];
    const auto& children = childrenMap[nodeId];

    ImGui::TableNextRow();

    //
    // Column 1: label
    ImGui::TableSetColumnIndex(0);

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
    if (children.empty())
    {
        // Leaf nodes don't need a collapsing arrow and don't push to the ID stack
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    constexpr const char spaces[33] = "                                ";
    const char*          spacesPtr  = spaces + sizeof(spaces) - 1u - (info.depth * 2); // points to the null terminator

    // We use the node's ID as a unique identifier for ImGui
    const bool isNodeOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(nodeId), nodeFlags, "%s", info.label.data());

    //
    // Column 2: time
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s%.3f", spacesPtr, timeSamplers[nodeId].getAverageAs<double>() / 1000.0); // Convert to ms

    //
    // Column 3: % of parent
    ImGui::TableSetColumnIndex(2);
    if (info.parentNodeId != sfex::nullNode)
    {
        if (const auto& parentInfo = allNodes[info.parentNodeId]; parentInfo.timeUs > 0)
            ImGui::Text("%s%.1f%%", spacesPtr + 1, percentSamplers[nodeId].getAverageAs<double>());
        else
            ImGui::Text("%sN/A", spacesPtr + 1);
    }
    else
        ImGui::Text(" "); // Root nodes have no parent

    //
    // Column 3: Location
    ImGui::TableSetColumnIndex(3);

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::Text("%s:%d", info.file.substrByPosLen(info.file.rfind('/') + 1u).data(), info.line);
    ImGui::PopStyleColor();

    // Recurse into children if the node is open and has children
    if (isNodeOpen && !children.empty())
    {
        for (const auto childId : children)
            renderNode(timeSamplers, percentSamplers, childId, allNodes, childrenMap);

        ImGui::TreePop(); // This is only needed if the node was not a leaf
    }
}

} // namespace sfex::priv


namespace sfex
{
////////////////////////////////////////////////////////////
inline void showImguiProfiler()
{
    const auto scopeInfos = sfex::getScopeInfos();

    if (scopeInfos.empty())
    {
        ImGui::Text("No profiling data captured for this thread.");
        return;
    }

    // Pre-process the flat list into a tree structure
    static thread_local priv::ChildrenMap               childrenMap(sfex::maxNodes);
    static thread_local sf::base::Vector<sfex::NodeId>  rootNodes;
    static thread_local priv::SamplerVec<sf::base::U64> nodeTimeSamplers(sfex::maxNodes,
                                                                         Sampler<sf::base::U64>{/* capacity */ 64u});
    static thread_local priv::SamplerVec<double> nodePercentSamplers(sfex::maxNodes, Sampler<double>{/* capacity */ 64u});

    sfex::populateNodes(scopeInfos, childrenMap, rootNodes); // Clears as the first step

    for (sfex::NodeId i = 0; i < static_cast<sfex::NodeId>(scopeInfos.size()); ++i)
    {
        if (scopeInfos[i].timeUs < 0)
            continue;

        nodeTimeSamplers[i].record(static_cast<sf::base::U64>(scopeInfos[i].timeUs));
        nodePercentSamplers[i].record(priv::calcNodePercentage(scopeInfos, scopeInfos[i]));
    }

    // Set up the table for display
    if (!ImGui::BeginTable("ProfilerTreeView",
                           4,
                           ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                               ImGuiTableFlags_Sortable))
        return;

    // Define columns
    ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, 120.0f, 0);
    ImGui::TableSetupColumn("Time (ms)", ImGuiTableColumnFlags_WidthStretch, 120.0f, 1);
    ImGui::TableSetupColumn("% of Parent", ImGuiTableColumnFlags_WidthStretch, 80.0f, 2);
    ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthStretch, 120.0f, 3);

    ImGui::TableHeadersRow();

    if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs())
    {
        const auto nodeComparer = [&](const sfex::NodeId a, const sfex::NodeId b) -> bool
        {
            const auto& infoA = scopeInfos[a];
            const auto& infoB = scopeInfos[b];

            for (int i = 0; i < specs->SpecsCount; ++i)
            {
                const ImGuiTableColumnSortSpecs* sortSpec = &specs->Specs[i];
                const int delta = priv::calcDelta(nodeTimeSamplers, nodePercentSamplers, infoA, infoB, sortSpec->ColumnUserID);

                if (delta == 0)
                    continue;

                return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? (delta < 0) : (delta > 0);
            }

            return false;
        };

        for (auto& vec : childrenMap)
            sf::base::quickSort(vec.begin(), vec.end(), nodeComparer);

        sf::base::quickSort(rootNodes.begin(), rootNodes.end(), nodeComparer);
    }

    // Kick off rendering for each root node
    for (const auto rootId : rootNodes)
        priv::renderNode(nodeTimeSamplers, nodePercentSamplers, rootId, scopeInfos, childrenMap);

    ImGui::EndTable();
}

} // namespace sfex
