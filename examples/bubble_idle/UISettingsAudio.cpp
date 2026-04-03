#include "BubbleIdleMain.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

void Main::uiSettingsDrawAudioTab()
{
    uiSetFontScale(uiNormalFontScale);

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("Master volume", &profile.masterVolume, 0.f, 100.f, "%.f%%");

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    if (ImGui::SliderFloat("SFX volume", &profile.sfxVolume, 0.f, 100.f, "%.f%%"))
        sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("Music volume", &profile.musicVolume, 0.f, 100.f, "%.f%%");

    uiCheckbox("Play audio in background", &profile.playAudioInBackground);
    uiCheckbox("Enable combo scratch sound", &profile.playComboEndSound);
    uiCheckbox("Enable ritual sounds", &profile.playWitchRitualSounds);
}
