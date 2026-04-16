#include "BubbleIdleMain.hpp"
#include "CatType.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"

sf::base::String Main::uiShopBuildNextGoalsText()
{
    const auto nCatNormal = pt->getCatCountByType(CatType::Normal);
    const auto nCatUni    = pt->getCatCountByType(CatType::Uni);
    const auto nCatDevil  = pt->getCatCountByType(CatType::Devil);
    const auto nCatAstro  = pt->getCatCountByType(CatType::Astro);

    const bool prestigedBefore            = pt->psvBubbleValue.nPurchases > 0u;
    const bool catUpgradesUnlocked        = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 2 && nCatUni >= 1;
    const bool catUnicornUnlocked         = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 3 &&
                                    (prestigedBefore || pt->anyCatEverWokenFromNap);
    const bool catUnicornUpgradesUnlocked = catUnicornUnlocked && nCatUni >= 2 && nCatDevil >= 1;
    const bool catDevilUnlocked         = catUnicornUnlocked && pt->psvBubbleValue.nPurchases > 0 && nCatNormal >= 6 &&
                                          nCatUni >= 4 && pt->nShrinesCompleted >= 1;
    const bool catDevilUpgradesUnlocked = catDevilUnlocked && nCatDevil >= 2 && nCatAstro >= 1;
    const bool astroCatUnlocked = nCatNormal >= 10 && nCatUni >= 5 && nCatDevil >= 2 && pt->nShrinesCompleted >= 2;
    const bool astroCatUpgradesUnlocked = astroCatUnlocked && nCatDevil >= 9 && nCatAstro >= 5;

    sf::base::String result;

    if (!pt->comboPurchased)
    {
        result = "- buy combo to earn money faster";
        return result;
    }

    if (pt->psvComboStartTime.nPurchases == 0)
    {
        result = "- buy longer combo to unlock cats";
        return result;
    }

    if (nCatNormal == 0)
    {
        result = "- buy a cat";
        return result;
    }

    const auto startList = [&](const char* s)
    {
        result += result.empty() ? "" : "\n\n";
        result += s;
    };

    const auto needNCats = [&](const sf::base::SizeT& count, const sf::base::SizeT needed)
    {
        const char* name = "";

        // clang-format off
            if      (&count == &nCatNormal) name = "cat";
            else if (&count == &nCatUni)    name = "unicat";
            else if (&count == &nCatDevil)  name = "devilcat";
            else if (&count == &nCatAstro)  name = "astrocat";
        // clang-format on

        if (count < needed)
            result += "\n    - buy " + sf::base::toString(needed - count) + " more " + name + "(s)";
    };

    if (!pt->mapPurchased)
    {
        startList("- to extend playing area:");
        result += "\n    - buy map scrolling";
    }

    if (!catUnicornUnlocked)
    {
        startList("- to unlock unicats:");

        if (pt->psvBubbleCount.nPurchases == 0)
            result += "\n    - buy more bubbles";

        needNCats(nCatNormal, 3);

        if (!prestigedBefore && !pt->anyCatEverWokenFromNap)
            result += "\n    - wake up a sleepy cat";
    }

    if (!catUpgradesUnlocked && catUnicornUnlocked)
    {
        startList("- to unlock cat upgrades:");

        if (pt->psvBubbleCount.nPurchases == 0)
            result += "\n    - buy more bubbles";

        needNCats(nCatNormal, 2);
        needNCats(nCatUni, 1);
    }

    if (catUnicornUnlocked && !pt->isBubbleValueUnlocked())
    {
        startList("- to unlock prestige:");

        if (pt->psvBubbleCount.nPurchases == 0)
            result += "\n    - buy more bubbles";

        if (pt->nShrinesCompleted < 1)
            result += "\n    - complete at least one shrine";

        needNCats(nCatUni, 3);
    }

    if (catUnicornUnlocked && pt->isBubbleValueUnlocked() && !catDevilUnlocked)
    {
        startList("- to unlock devilcats:");

        if (pt->psvBubbleValue.nPurchases == 0)
            result += "\n    - prestige at least once";

        if (pt->nShrinesCompleted < 1)
            result += "\n    - complete at least one shrine";

        if (pt->psvBubbleValue.nPurchases > 0u)
        {
            needNCats(nCatNormal, 6);
            needNCats(nCatUni, 4);
        }
    }

    if (catUnicornUnlocked && catDevilUnlocked && !catUnicornUpgradesUnlocked)
    {
        startList("- to unlock unicat upgrades:");
        needNCats(nCatUni, 2);
        needNCats(nCatDevil, 1);
    }

    if (catUnicornUnlocked && catDevilUnlocked && !astroCatUnlocked)
    {
        startList("- to unlock astrocats:");

        if (pt->nShrinesCompleted < 2)
            result += "\n    - complete at least two shrines";

        needNCats(nCatNormal, 10);
        needNCats(nCatUni, 5);
        needNCats(nCatDevil, 2);
    }

    if (catUnicornUnlocked && catDevilUnlocked && astroCatUnlocked && !catDevilUpgradesUnlocked)
    {
        startList("- to unlock devilcat upgrades:");
        needNCats(nCatDevil, 2);
        needNCats(nCatAstro, 1);
    }

    if (catUnicornUnlocked && catDevilUnlocked && astroCatUnlocked && !astroCatUpgradesUnlocked)
    {
        startList("- to unlock astrocat upgrades:");
        needNCats(nCatDevil, 9);
        needNCats(nCatAstro, 5);
    }

    return result;
}
