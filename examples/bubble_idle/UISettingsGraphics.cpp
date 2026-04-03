#include "BubbleIdleMain.hpp"

#include "Profile.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

namespace
{
template <typename TVibrance, typename TSaturation, typename TLightness, typename TSharpness, typename TBlur>
void drawPostProcessControls(Profile& profile,
                             const float uiScale,
                             TVibrance vibrance,
                             TSaturation saturation,
                             TLightness lightness,
                             TSharpness sharpness,
                             TBlur blur)
{
    ImGui::SetNextItemWidth(210.f * uiScale);
    ImGui::SliderFloat("Vibrance", &(profile.*vibrance), 0.f, 2.f, "%.2f");

    ImGui::SetNextItemWidth(210.f * uiScale);
    ImGui::SliderFloat("Saturation", &(profile.*saturation), 0.f, 2.f, "%.2f");

    ImGui::SetNextItemWidth(210.f * uiScale);
    ImGui::SliderFloat("Lightness", &(profile.*lightness), 0.5f, 1.5f, "%.2f");

    ImGui::SetNextItemWidth(210.f * uiScale);
    ImGui::SliderFloat("Sharpness", &(profile.*sharpness), 0.f, 1.f, "%.2f");

    ImGui::SetNextItemWidth(210.f * uiScale);
    ImGui::SliderFloat("Blur", &(profile.*blur), 0.f, 1.f, "%.2f");

    if (ImGui::Button("Reset to default"))
    {
        Profile defaultProfile{};

        profile.*vibrance   = defaultProfile.*vibrance;
        profile.*saturation = defaultProfile.*saturation;
        profile.*lightness  = defaultProfile.*lightness;
        profile.*sharpness  = defaultProfile.*sharpness;
        profile.*blur       = defaultProfile.*blur;
    }
}
} // namespace

void Main::uiSettingsDrawGraphicsTab()
{
    uiSetFontScale(uiNormalFontScale);

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("Background Opacity", &profile.backgroundOpacity, 0.f, 100.f, "%.f%%");

    uiCheckbox("Always show drawings", &profile.alwaysShowDrawings);

    ImGui::Separator();

    uiCheckbox("Show cat range", &profile.showCatRange);
    uiCheckbox("Show ranges only on hover", &profile.showRangesOnlyOnHover);
    uiCheckbox("Show cat text", &profile.showCatText);
    uiCheckbox("Enable cat bobbing", &profile.enableCatBobbing);

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    ImGui::SliderFloat("Cat range thickness", &profile.catRangeOutlineThickness, 1.f, 4.f, "%.2fpx");

    ImGui::Separator();

    uiCheckbox("Show particles", &profile.showParticles);

    ImGui::BeginDisabled(!profile.showParticles);
    uiCheckbox("Show coin particles", &profile.showCoinParticles);
    ImGui::EndDisabled();

    uiCheckbox("Show text particles", &profile.showTextParticles);

    ImGui::Separator();

    uiCheckbox("Enable screen shake", &profile.enableScreenShake);

    ImGui::Separator();

    uiCheckbox("Show bubbles", &profile.showBubbles);

    ImGui::Separator();

    uiCheckbox("Show doll particle border", &profile.showDollParticleBorder);

    ImGui::Separator();

    uiCheckbox("Bubble shader", &profile.useBubbleShader);

    ImGui::BeginDisabled(!profile.useBubbleShader);
    {
        uiSetFontScale(0.75f);

        if (isDebugModeEnabled())
        {
            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Iridescence", &profile.bsIridescenceStrength, 0.f, 1.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Edge Factor Min", &profile.bsEdgeFactorMin, 0.f, 1.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Edge Factor Max", &profile.bsEdgeFactorMax, 0.f, 1.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Edge Factor Strength", &profile.bsEdgeFactorStrength, 0.f, 10.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Distortion Strength", &profile.bsDistortionStrength, 0.f, 1.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Lens Distortion", &profile.bsLensDistortion, 0.f, 10.f, "%.2f");
        }

        ImGui::BeginDisabled(!profile.useBubbleShader);
        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Bubble Lightness", &profile.bsBubbleLightness, -1.f, 1.f, "%.2f");
        ImGui::EndDisabled();

        uiSetFontScale(uiNormalFontScale);
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    ImGui::Text("Foreground postprocess");
    ImGui::PushID("foregroundPostProcess");

    drawPostProcessControls(profile,
                            profile.uiScale,
                            &Profile::ppSVibrance,
                            &Profile::ppSSaturation,
                            &Profile::ppSLightness,
                            &Profile::ppSSharpness,
                            &Profile::ppSBlur);

    ImGui::PopID();

    ImGui::Separator();

    ImGui::Text("Background postprocess");
    ImGui::PushID("backgroundPostProcess");

    drawPostProcessControls(profile,
                            profile.uiScale,
                            &Profile::ppBGVibrance,
                            &Profile::ppBGSaturation,
                            &Profile::ppBGLightness,
                            &Profile::ppBGSharpness,
                            &Profile::ppBGBlur);

    ImGui::PopID();

    ImGui::Separator();

    ImGui::Text("Advanced options");

    constexpr const char* autobatchModes[3]{"Off", "CPU", "GPU"};

    ImGui::SetNextItemWidth(210.f * profile.uiScale);
    if (ImGui::Combo("Batching mode", &profile.autobatchMode, autobatchModes, 3))
        refreshWindowAutoBatchModeFromProfile();

    uiCheckbox("Flush after every batch", &flushAfterEveryBatch);
    uiCheckbox("Finish after every batch", &finishAfterEveryBatch);
    uiCheckbox("Flush before display", &flushBeforeDisplay);
    uiCheckbox("Finish before display", &finishBeforeDisplay);
    uiCheckbox("Flush after display", &flushAfterDisplay);
    uiCheckbox("Finish after display", &finishAfterDisplay);
}
