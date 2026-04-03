#include "BubbleIdleMain.hpp"
#include "IconsFontAwesome6.h"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Window/Keyboard.hpp"

#include "SFML/Base/SizeT.hpp"

////////////////////////////////////////////////////////////
void Main::uiTabBar()
{
    const float     childHeight                = uiGetMaxWindowHeight() - (60.f * profile.uiScale);
    constexpr float uiMenuAutoHideDelaySeconds = 1.25f;

    constexpr TabButtonPalette defaultPalette{
        .idle    = ImVec4(0.15f, 0.35f, 0.60f, 1.0f),
        .hovered = ImVec4(0.25f, 0.45f, 0.80f, 1.0f),
        .active  = ImVec4(0.35f, 0.55f, 0.95f, 1.0f),
    };

    constexpr TabButtonPalette prestigePalette{
        .idle    = ImVec4(0.53f, 0.20f, 0.33f, 1.0f),
        .hovered = ImVec4(0.53f, 0.25f, 0.41f, 1.0f),
        .active  = ImVec4(0.62f, 0.29f, 0.48f, 1.0f),
    };

    const auto keyboardSelectedTab = [&](const sf::Keyboard::Key key) -> bool
    { return !ImGui::GetIO().WantCaptureKeyboard && inputHelper.wasKeyJustPressed(key); };

    const auto selectTab = [&](const int idx)
    {
        if (uiState.lastUiSelectedTabIdx != idx)
            playSound(sounds.uitab);

        uiState.lastUiSelectedTabIdx = idx;
    };

    if (uiState.shopSelectOnce != ImGuiTabItemFlags_{})
    {
        uiState.lastUiSelectedTabIdx = 1;
        uiState.shopSelectOnce       = {};
    }

    if (uiState.lastUiSelectedTabIdx == 2 && getWizardCat() == nullptr)
        uiState.lastUiSelectedTabIdx = 1;

    if (uiState.lastUiSelectedTabIdx == 3 && !pt->isBubbleValueUnlocked())
        uiState.lastUiSelectedTabIdx = 1;

    if (keyboardSelectedTab(sf::Keyboard::Key::Slash) || keyboardSelectedTab(sf::Keyboard::Key::Grave) ||
        keyboardSelectedTab(sf::Keyboard::Key::Apostrophe) || keyboardSelectedTab(sf::Keyboard::Key::Backslash))
        selectTab(0);

    sf::base::SizeT nextTabKeyIndex = 0u;

    constexpr sf::Keyboard::Key tabKeys[] = {
        sf::Keyboard::Key::Num1,
        sf::Keyboard::Key::Num2,
        sf::Keyboard::Key::Num3,
        sf::Keyboard::Key::Num4,
        sf::Keyboard::Key::Num5,
        sf::Keyboard::Key::Num6,
    };

    if (keyboardSelectedTab(tabKeys[nextTabKeyIndex++]))
        selectTab(1);

    if (getWizardCat() != nullptr && keyboardSelectedTab(tabKeys[nextTabKeyIndex++]))
        selectTab(2);

    if (pt->isBubbleValueUnlocked())
    {
        if (!pt->prestigeTipShown)
        {
            pt->prestigeTipShown = true;
            doTip("Prestige to increase bubble value\nand unlock permanent upgrades!");
        }

        if (keyboardSelectedTab(tabKeys[nextTabKeyIndex++]))
            selectTab(3);
    }


    ImGui::SameLine(0.f, 12.f * profile.uiScale);
    if (drawTabButton(1.f, " " ICON_FA_STORE " Shop ##992", uiState.lastUiSelectedTabIdx == 1, defaultPalette))
        selectTab(1);

    if (getWizardCat() != nullptr)
    {
        ImGui::SameLine(0.f, 0.f);
        if (drawTabButton(1.f, " " ICON_FA_STAR " Magic ##993", uiState.lastUiSelectedTabIdx == 2, defaultPalette))
            selectTab(2);
    }

    if (pt->isBubbleValueUnlocked())
    {
        const bool              canPrestige = pt->canBuyNextPrestige();
        const TabButtonPalette& palette     = canPrestige ? prestigePalette : defaultPalette;

        ImGui::SameLine(0.f, 0.f);
        if (drawTabButton(1.f, " " ICON_FA_TROPHY " Prestige ##994", uiState.lastUiSelectedTabIdx == 3, palette))
            selectTab(3);
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 92.f * profile.uiScale, 0.f);
    if (drawTabButton(1.f, ICON_FA_CIRCLE_INFO "##991", uiState.lastUiSelectedTabIdx == 4, defaultPalette, {}, true))
        selectTab(4);

    ImGui::SameLine(0.f, 0.f);
    if (drawTabButton(1.f, ICON_FA_GEAR "##990", uiState.lastUiSelectedTabIdx == 5, defaultPalette, {}, true))
        selectTab(5);

    ImGui::SameLine(0.f, 0.f);
    if (drawTabButton(1.f,
                      uiState.uiMenuLocked ? ICON_FA_LOCK "##995" : ICON_FA_LOCK_OPEN "##995",
                      uiState.uiMenuLocked,
                      defaultPalette,
                      {},
                      true))
    {
        uiState.uiMenuLocked    = !uiState.uiMenuLocked;
        uiState.uiMenuHideTimer = uiMenuAutoHideDelaySeconds;
        playSound(sounds.uitab);
    }

    switch (uiState.lastUiSelectedTabIdx)
    {
        case 1:
        {
            ImGui::BeginChild("ShopScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight));
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            uiTabBarShop();
            ImGui::EndChild();

            break;
        }

        case 2:
        {
            if (getWizardCat() != nullptr)
            {
                ImGui::BeginChild("MagicScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight));
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                uiTabBarMagic();
                ImGui::EndChild();
            }

            break;
        }

        case 3:
        {
            if (pt->isBubbleValueUnlocked())
            {
                ImGui::BeginChild("PrestigeScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight));
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                uiTabBarPrestige();
                ImGui::EndChild();
            }

            break;
        }

        case 4:
        {
            ImGui::BeginChild("StatsScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight));
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            uiTabBarStats();
            ImGui::EndChild();

            break;
        }

        case 5:
        {
            ImGui::BeginChild("OptionsScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight));
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            uiTabBarSettings();
            ImGui::EndChild();

            break;
        }

        default:
            break;
    }
}
