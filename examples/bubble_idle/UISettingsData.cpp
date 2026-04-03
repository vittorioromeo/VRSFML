#include "BubbleIdleMain.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

void Main::uiSettingsDrawDataTab()
{
    uiSetFontScale(uiNormalFontScale);

    ImGui::Text("!!! Danger Zone !!!");

    uiSetFontScale(0.75f);
    ImGui::Text("Be careful with these dangerous settings!\nYour progress might be lost forever!");
    uiSetFontScale(uiNormalFontScale);

    static bool dangerZone = false;
    uiCheckbox("Enable Danger Zone (!)", &dangerZone);

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::BeginDisabled(!dangerZone);

    uiState.uiButtonHueMod = 120.f;
    uiPushButtonColors();

    if (ImGui::Button("Reset *current* prestige##dangerzoneforceprestige"))
    {
        dangerZone = false;
        beginPrestigeTransition(0u);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Reset game##dangerzoneresetgame"))
    {
        dangerZone = false;
        forceResetGame();
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset profile##dangerzoneresetprofile"))
    {
        dangerZone = false;
        forceResetProfile();
    }

    uiPopButtonColors();
    uiState.uiButtonHueMod = 0.f;

    ImGui::EndDisabled();

    ImGui::Separator();

    if (inSpeedrunPlaythrough())
    {
        ImGui::Text("Speedrun mode enabled!");
        uiSetFontScale(0.75f);
        ImGui::Text("Currently in a speedrun playthrough");
        uiSetFontScale(uiNormalFontScale);

        if (ImGui::Button("End Speedrun"))
        {
            pt = &ptMain;

            reseedRNGs(pt->seed);
            shuffledCatNamesPerType = makeShuffledCatNames(rng);
        }
    }
    else
    {
        ImGui::Text("Speedrun mode disabled!");
        uiSetFontScale(0.75f);
        ImGui::Text("Currently in the normal playthrough");
        uiSetFontScale(uiNormalFontScale);

        if (ImGui::Button("Start New Speedrun"))
        {
            pt = &ptSpeedrun;
            forceResetGame(/* goToShopTab */ false);
        }
    }
}
