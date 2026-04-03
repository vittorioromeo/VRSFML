#include "BubbleIdleMain.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

void Main::uiTabBarPrestige()
{
    uiPrestigeDrawOverview();
    uiPrestigeDrawCoreUpgrades();
    uiPrestigeDrawShrineCatUpgrades();

    uiState.uiButtonHueMod = 0.f;

    ImGui::Columns(1);
}
