#include "BubbleIdleMain.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

void Main::uiSettingsDrawUiTab()
{
    uiSetFontScale(uiNormalFontScale);

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("Minimap Scale", &profile.minimapScale, 5.f, 40.f, "%.2f");

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("HUD Scale", &profile.hudScale, 0.5f, 2.f, "%.2f");

    ImGui::Separator();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("UI Scale");

    const auto makeUIScaleButton = [&](const char* label, const float scaleFactor)
    {
        ImGui::SameLine();
        if (ImGui::Button(label, ImVec2{46.f * profile.uiScale, 0.f}))
        {
            playSound(sounds.buy);
            profile.uiScale = scaleFactor;
        }
    };

    makeUIScaleButton("XXL", 1.75f);
    makeUIScaleButton("XL", 1.5f);
    makeUIScaleButton("L", 1.25f);
    makeUIScaleButton("M", 1.f);
    makeUIScaleButton("S", 0.75f);
    makeUIScaleButton("XS", 0.5f);

    ImGui::Separator();

    uiCheckbox("Hide maxed-out upgrades", &profile.hideMaxedOutPurchasables);
    uiCheckbox("Hide category separators", &profile.hideCategorySeparators);

    ImGui::Separator();

    uiCheckbox("Enable tips", &profile.tipsEnabled);

    ImGui::Separator();

    uiCheckbox("Enable notifications", &profile.enableNotifications);

    ImGui::BeginDisabled(!profile.enableNotifications);
    uiCheckbox("Enable full mana notification", &profile.showFullManaNotification);
    ImGui::EndDisabled();

    ImGui::Separator();

    uiCheckbox("Enable $/s meter", &profile.showDpsMeter);

    ImGui::Separator();

    constexpr const char* trailModeNames[3]{"Combo only", "Always", "Never"};

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::Combo("Cursor trail mode", &profile.cursorTrailMode, trailModeNames, 3);

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("Cursor trail scale", &profile.cursorTrailScale, 0.25f, 5.f, "%.2f");

    ImGui::Separator();

    uiCheckbox("High-visibility cursor", &profile.highVisibilityCursor);

    ImGui::BeginDisabled(!profile.highVisibilityCursor);
    {
        uiSetFontScale(0.75f);

        uiCheckbox("Multicolor", &profile.multicolorCursor);

        ImGui::BeginDisabled(profile.multicolorCursor);
        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Hue", &profile.cursorHue, 0.f, 360.f, "%.2f");
        ImGui::EndDisabled();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Scale", &profile.cursorScale, 0.3f, 1.5f, "%.2f");

        uiSetFontScale(uiNormalFontScale);
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    uiCheckbox("Accumulating combo effect", &profile.accumulatingCombo);
    uiCheckbox("Show cursor combo text", &profile.showCursorComboText);
    uiCheckbox("Show cursor combo bar", &profile.showCursorComboBar);

    ImGui::Separator();

    uiCheckbox("Invert mouse buttons", &profile.invertMouseButtons);

    ImGui::Separator();

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("Cat drag timer", &profile.catDragPressDuration, 50.f, 500.f, "%.2fms");
}
