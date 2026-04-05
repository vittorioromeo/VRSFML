#include "BubbleIdleMain.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

void Main::uiSettingsDrawDisplayTab()
{
    uiSetFontScale(uiNormalFontScale);

    ImGui::Text("Auto resolution");

    uiSetFontScale(0.85f);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Windowed");

    ImGui::SameLine();

    if (ImGui::Button("Large"))
    {
        playSound(sounds.buy);

        profile.resWidth = getReasonableWindowSize(1.f);
        profile.windowed = true;

        if (window.isFullscreen())
            recreateWindow();
        else
            resizeWindow();
    }

    ImGui::SameLine();

    if (ImGui::Button("Medium"))
    {
        playSound(sounds.buy);

        profile.resWidth = getReasonableWindowSize(0.9f);
        profile.windowed = true;

        if (window.isFullscreen())
            recreateWindow();
        else
            resizeWindow();
    }

    ImGui::SameLine();

    if (ImGui::Button("Small"))
    {
        playSound(sounds.buy);

        profile.resWidth = getReasonableWindowSize(0.8f);
        profile.windowed = true;

        if (window.isFullscreen())
            recreateWindow();
        else
            resizeWindow();
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Fullscreen");

    ImGui::SameLine();

    if (ImGui::Button("Borderless"))
    {
        playSound(sounds.buy);

        profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
        profile.windowed = true;

        if (window.isFullscreen())
            recreateWindow();
        else
            resizeWindow();
    }

    ImGui::SameLine();

    if (ImGui::Button("Exclusive"))
    {
        playSound(sounds.buy);

        profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
        profile.windowed = false;

        recreateWindow();
    }

    ImGui::Separator();

    if (uiCheckbox("VSync", &profile.vsync))
        window.setVerticalSyncEnabled(profile.vsync);

    static auto fpsLimit = static_cast<float>(profile.frametimeLimit);
    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    if (ImGui::DragFloat("FPS Limit", &fpsLimit, 1.f, 60.f, 360.f, "%.f", ImGuiSliderFlags_AlwaysClamp))
    {
        profile.frametimeLimit = static_cast<unsigned int>(fpsLimit);
        window.setFramerateLimit(profile.frametimeLimit);
    }

    uiSetFontScale(uiNormalFontScale);
}
