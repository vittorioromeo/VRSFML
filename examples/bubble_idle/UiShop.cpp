#include "BubbleIdleMain.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Base/String.hpp"

void Main::uiTabBarShop()
{
    ImGui::Spacing();
    ImGui::Spacing();

    uiShopDrawCoreUpgrades();
    uiShopDrawSpecialCats();
    uiShopDrawUniqueCatBonuses();

    const sf::base::String nextGoalsText = uiShopBuildNextGoalsText();

    ImGui::Columns(1);

    if (nextGoalsText != "")
    {
        uiImgsep(txrMenuSeparator8, "next goals");
        ImGui::Columns(1);

        uiSetFontScale(uiSubBulletFontScale);
        ImGui::Text("%s", nextGoalsText.cStr());
        uiSetFontScale(uiNormalFontScale);
    }

    ImGui::Columns(1);
}
