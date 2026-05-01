#include "BubbleIdleMain.hpp"
#include "IconsFontAwesome6.h"

#include "SFML/ImGui/IncludeImGui.hpp"

void Main::uiTabBarSettings()
{
    constexpr TabButtonPalette palette{
        .idle    = ImVec4(0.15f, 0.35f, 0.60f, 1.0f),
        .hovered = ImVec4(0.25f, 0.45f, 0.80f, 1.0f),
        .active  = ImVec4(1.f, 1.f, 1.f, 1.0f),
    };

    static int lastSelectedTabIdx = 0;

    if (lastSelectedTabIdx > 4)
        lastSelectedTabIdx = 0;

    const auto selectedTab = [&](const int idx)
    {
        if (lastSelectedTabIdx != idx)
            playSound(sounds.uitab);

        lastSelectedTabIdx = idx;
    };

    uiSetFontScale(0.75f);

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SameLine(0.f, 5.f);
    if (drawTabButton(0.75f, ICON_FA_VOLUME_HIGH " Audio ##199910", lastSelectedTabIdx == 0, palette))
        selectedTab(0);

    ImGui::SameLine(0.f, 5.f);
    if (drawTabButton(0.75f, ICON_FA_WINDOW_MAXIMIZE " UI ##19991", lastSelectedTabIdx == 1, palette))
        selectedTab(1);

    ImGui::SameLine(0.f, 5.f);
    if (drawTabButton(0.75f, ICON_FA_IMAGE " Graphics ##19992", lastSelectedTabIdx == 2, palette))
        selectedTab(2);

    ImGui::SameLine(0.f, 5.f);
    if (drawTabButton(0.75f, ICON_FA_DESKTOP " Display ##19993", lastSelectedTabIdx == 3, palette))
        selectedTab(3);

    ImGui::SameLine(0.f, 5.f);
    if (drawTabButton(0.75f, ICON_FA_FILE " Data ##19994", lastSelectedTabIdx == 4, palette))
        selectedTab(4);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    uiSetFontScale(0.75f);

    if (lastSelectedTabIdx == 0)
        uiSettingsDrawAudioTab();

    if (lastSelectedTabIdx == 1)
        uiSettingsDrawUiTab();

    if (lastSelectedTabIdx == 2)
        uiSettingsDrawGraphicsTab();

    if (lastSelectedTabIdx == 3)
        uiSettingsDrawDisplayTab();

    if (lastSelectedTabIdx == 4)
        uiSettingsDrawDataTab();

    ImGui::Separator();
    uiSetFontScale(uiNormalFontScale);
    ImGui::Text("FPS: %.2f", static_cast<double>(fps));
}
