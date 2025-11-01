#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "BubbleType.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "ImGuiNotify.hpp"
#include "InputHelper.hpp"
#include "Milestones.hpp"
#include "ParticleType.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "RNGFast.hpp"
#include "Sampler.hpp"
#include "Serialization.hpp"
#include "Sounds.hpp"
#include "Version.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm/AnyOf.hpp"
#include "SFML/Base/Algorithm/Count.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FloatMax.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <string>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>


////////////////////////////////////////////////////////////
float Main::uiGetMaxWindowHeight() const
{
    return sf::base::max(getResolution().y - 30.f, (getResolution().y - 30.f) / profile.uiScale);
}

////////////////////////////////////////////////////////////
void Main::uiSetFontScale(const float scale)
{
    lastFontScale = scale;
    ImGui::SetWindowFontScale(scale);
}

////////////////////////////////////////////////////////////
void Main::uiMakeButtonLabels(const char* label, const char* xLabelBuffer)
{
    const char* readableLabelEnd = ImGui::FindRenderedTextEnd(label);

    const bool subBullet = label[0] == ' ' && label[1] == ' ';

    const char* labelStart = subBullet ? label + 2 : label;
    const auto  labelSize  = readableLabelEnd - labelStart;

    // button label
    uiSetFontScale((subBullet ? uiSubBulletFontScale : uiNormalFontScale) * 1.15f);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%.*s", static_cast<int>(labelSize), labelStart);
    ImGui::SameLine();

    // button top label
    uiSetFontScale(0.5f);
    ImGui::Text("%s", xLabelBuffer);
    uiSetFontScale(subBullet ? uiSubBulletFontScale : uiNormalFontScale);
    ImGui::SameLine();

    ImGui::NextColumn();
}

////////////////////////////////////////////////////////////
void Main::uiPushButtonColors()
{
    const auto convertColorWithHueMod = [&](const auto colorId)
    {
        return sf::Color::fromVec4(ImGui::GetStyleColorVec4(colorId)).withRotatedHue(uiButtonHueMod).template toVec4<ImVec4>();
    };

    ImGui::PushStyleColor(ImGuiCol_Button, convertColorWithHueMod(ImGuiCol_Button));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, convertColorWithHueMod(ImGuiCol_ButtonHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, convertColorWithHueMod(ImGuiCol_ButtonActive));
    ImGui::PushStyleColor(ImGuiCol_Border, colorBlueOutline.withRotatedHue(uiButtonHueMod).toVec4<ImVec4>());
}

////////////////////////////////////////////////////////////
void Main::uiPopButtonColors()
{
    ImGui::PopStyleColor(4);
}

////////////////////////////////////////////////////////////
void Main::uiBeginTooltip(const float width)
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0), ImVec2(width, SFML_BASE_FLOAT_MAX));

    ImGui::BeginTooltip();
    ImGui::PushFont(fontImGuiMouldyCheese);
    uiSetFontScale(uiToolTipFontScale);
}

////////////////////////////////////////////////////////////
void Main::uiEndTooltip()
{
    uiSetFontScale(uiNormalFontScale);
    ImGui::PopFont();
    ImGui::EndTooltip();
}

////////////////////////////////////////////////////////////
void Main::uiMakeTooltip(const bool small)
{
    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) || std::strlen(uiTooltipBuffer) == 0u)
        return;

    const float width = small ? 176.f : uiTooltipWidth;

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetMousePos().x - width, ImGui::GetMousePos().y + (small ? -40.f : 20.f)));

    uiBeginTooltip(width);
    ImGui::TextWrapped("%s", uiTooltipBuffer);
    uiEndTooltip();
}

////////////////////////////////////////////////////////////
void Main::uiMakeShrineOrCatTooltip(const sf::Vec2f mousePos)
{
    const auto* hoveredShrine = [&]() -> const Shrine*
    {
        for (Shrine& shrine : pt->shrines)
            if ((mousePos - shrine.position).lengthSquared() <= shrine.getRadiusSquared())
                return &shrine;

        return nullptr;
    }();

    const auto* hoveredCat = [&]() -> const Cat*
    {
        if (hoveredShrine == nullptr)
            for (const Cat& cat : pt->cats)
                if ((mousePos - cat.position).lengthSquared() <= cat.getRadiusSquared())
                    return &cat;

        return nullptr;
    }();

    if ((hoveredShrine == nullptr && hoveredCat == nullptr) || std::strlen(uiTooltipBuffer) == 0u)
        return;

    ImGui::SetNextWindowPos(ImVec2(getResolution().x - 15.f, getResolution().y - 15.f), 0, ImVec2(1, 1));
    uiBeginTooltip(uiTooltipWidth);

    if (hoveredShrine != nullptr)
    {
        std::sprintf(uiTooltipBuffer, "%s", shrineTooltipsByType[static_cast<SizeT>(hoveredShrine->type)] + 1);
    }
    else
    {
        const char* catNormalTooltip = R"(
~~ Cat ~~

Pops bubbles or bombs, whatever comes first. Not the brightest, despite not being orange.

Prestige points can be spent for their college tuition, making them more cleverer.)";

        if (pt->perm.geniusCatsPurchased)
        {
            catNormalTooltip =
                R"(
~~ Genius Cat ~~

A truly intelligent being: prioritizes popping bombs first, then star bubbles, then normal bubbles. Can be instructed to ignore specific bubble types.

Through the sheer power of their intellect, they also get a x2 multiplier on all bubble values.

We do not speak of the origin of the large brain attached to their body.)";
        }
        else if (pt->perm.smartCatsPurchased)
        {
            catNormalTooltip =
                R"(
~~ Smart Cat ~~

Pops bubbles or bombs. Smart enough to prioritizes bombs and star bubbles over normal bubbles, but can't really tell those two apart.

We do not speak of the tuition fees.)";
        }

        const char* catUniTooltip = R"(
~~ Unicat ~~

Imbued with the power of stars and rainbows, transforms bubbles into star bubbles, worth x15 more.

Must have eaten something they weren't supposed to, because they keep changing color.
)";

        if (isUnicatTranscendenceActive())
            catUniTooltip = R"(
~~ Transcended Unicat ~~

Imbued with the power of the void, transforms bubbles into nova bubbles, worth x50 more.

Radiates a somewhat creepy energy.
)";

        const char* catDevilTooltip = R"(
~~ Devilcat ~~

Hired diplomat of the NB (NOBUBBLES) political party. Convinces bubbles to turn into bombs and explode for the rightful cause.

Bubbles caught in explosions are worth x10 more.)";

        if (isDevilcatHellsingedActive())
            catDevilTooltip = R"(
~~ Hellsinged Devilcat ~~

Sold their soul to the devil. Opens up portals to hells that absorb bubbles with a x50 multiplier.

From politician to demon... the NB (NOBUBBLES) party is truly a mystery.)";

        const char* catAstroTooltip = R"(
~~ Astrocat ~~

Pride of the NCSA, a highly trained feline astronaut that continuously flies across the map, popping bubbles with a x20 multiplier.

Desperately trying to get funding from the government for a mission on the cheese moon. Perhaps some prestige points could help?)";

        if (pt->perm.astroCatInspirePurchased)
        {
            catAstroTooltip =
                R"(
~~ Propagandist Astrocat ~~

Pride of the NCSA, a highly trained feline astronaut that continuously flies across the map, popping bubbles with a x20 multiplier.

Finally financed by the NB (NOBUBBLES) political party to inspire other cats to work faster when flying by.)";
        }

        const char* catTooltipsByType[]{
            catNormalTooltip,
            catUniTooltip,
            catDevilTooltip,
            catAstroTooltip,
            R"(
~~ Witchcat ~~
(unique cat)

Loves to perform rituals on other cats, hexing one of them at random and capturing their soul in voodoo dolls that appear around the map.

Collecting all the dolls will release the hex and trigger a powerful timed effect depending on the type of the cursed cat.

Their dark magic is puzzling... but not as puzzling as the sheer number of dolls they carry around.)",
            R"(
~~ Wizardcat ~~
(unique cat)

Ancient arcane feline capable of unleashing powerful spells, if only they could remember them.
Can absorb the magic of star bubbles to recall their past lives and remember spells.

The scriptures say that they "unlock a Magic menu", but nobody knows what that means.

Witchcat interaction: after being hexed, will grant a x3.5 faster mana regen buff.)",
            R"(
~~ Mousecat ~~
(unique cat)

They stole a Logicat gaming mouse and they're now on the run. Surprisingly, the mouse still works even though it's not plugged in to anything.

Able to keep up a combo like for manual popping, and empowers nearby cats to pop bubbles with Mousecat's current combo multiplier.

Is affected by both cat reward value multipliers and click reward value multipliers, including their own buff.

Provides a global click reward value multiplier (upgradable via PPs) by merely existing... Logicat does know how to make a good mouse.

Witchcat interaction: after being hexed, will grant a x10 click reward buff.)",
            R"(
~~ Engicat ~~
(unique cat)

Periodically performs maintenance on all nearby cats, temporarily increasing their engine efficiency and making them faster. (Note: this buff stacks with inspirational NB propaganda.)

Provides a global cat reward value multiplier (upgradable via PPs) by merely existing... guess they're a "10x engineer"?

Witchcat interaction: after being hexed, will grant a x2 global faster cat cooldown buff.)",
            R"(
~~ Repulsocat ~~
(unique cat)

Continuously pushes bubbles away with their powerful USB fan, powered by only Dog knows what kind of batteries. (Note: this effect is applied even while Repulsocat is being dragged.)

Bubbles being pushed away by Repulsocat are worth x2 more.

Using prestige points, the fan can be upgraded to filter specific bubble types and/or convert a percentage of bubbles to star bubbles.

Witchcat interaction: after being hexed, will grant a x2 bubble count buff and increase wind speed.)",
            R"(
~~ Attractocat ~~
(unique cat)

Continuously attracts bubbles with their huge magnet, because soap is definitely magnetic. (Note: this effect is applied even while Attractocat is being dragged.)

Bubbles being attracted by Attractocat are worth x2 more.

Using prestige points, the magnet can be upgraded to filter specific bubble types.

Witchcat interaction: after being hexed, all bombs or hell portals will attract bubbles.)",
            R"(
~~ Copycat ~~
(unique cat)

Mimics an existing unique cat, gaining their abilities and effects. (Note: the mimicked cat can be changed via the toolbar near the bottom of the screen.)

Mimicking Witchcat:
- Two separate rituals will be performed.

Mimicking Wizardcat:
- Spells will also be casted by the Copycat.
- Mewliplier Aura's bonus does not stack twice.

Mimicking Mousecat:
- The combo multiplier is shared.
- The global clicking buff multiplier is doubled.

Mimicking Engicat:
- The global clicking buff multiplier is doubled.

Witchcat interaction: after being hexed, will grant the same buff as the mimicked cat.)",
            R"(
~~ Duck ~~

It's a duck.)",
        };

        static_assert(sf::base::getArraySize(catTooltipsByType) == nCatTypes);

        SFML_BASE_ASSERT(hoveredCat != nullptr);
        std::sprintf(uiTooltipBuffer, "%s", catTooltipsByType[static_cast<SizeT>(hoveredCat->type)] + 1);
    }

    ImGui::TextWrapped("%s", uiTooltipBuffer);
    uiTooltipBuffer[0] = '\0';

    uiEndTooltip();
}

////////////////////////////////////////////////////////////
Main::AnimatedButtonOutcome Main::uiAnimatedButton(const char* label, const ImVec2& btnSize, const float fontScale, const float fontScaleMult)
{
    ImGuiWindow* imGuiWindow = ImGui::GetCurrentWindow();

    if (imGuiWindow->SkipItems)
        return AnimatedButtonOutcome::None;

    const char* labelEnd = ImGui::FindRenderedTextEnd(label);

    const auto id = static_cast<ImGuiID>(std::strtoul(labelEnd + 2, nullptr, 10));

    const ImVec2 labelSize = ImGui::CalcTextSize(label, labelEnd, true);

    const ImVec2 size = ImGui::CalcItemSize(btnSize,
                                            labelSize.x + ImGui::GetStyle().FramePadding.x * 2.f,
                                            labelSize.y + ImGui::GetStyle().FramePadding.y * 2.f);

    const ImRect bb(imGuiWindow->DC.CursorPos, imGuiWindow->DC.CursorPos + size);
    ImGui::ItemSize(bb);

    if (!ImGui::ItemAdd(bb, id))
        return AnimatedButtonOutcome::None;

    // Store animation state in window data
    struct AnimState
    {
        float hoverAnim;     // 0.f to 1.f for tilt
        float clickAnim;     // 0.f to 1.f for scale
        float lastClickTime; // in seconds
    };

    // Get or create animation state
    ImGuiStorage* storage     = imGuiWindow->DC.StateStorage;
    const ImGuiID animStateId = id + 1; // Use a different ID for the state

    auto* animState = static_cast<AnimState*>(storage->GetVoidPtr(animStateId));
    if (animState == nullptr)
    {
        static sf::base::Vector<sf::base::UniquePtr<AnimState>> animStateStorage;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
        animState = animStateStorage.emplaceBack(sf::base::makeUnique<AnimState>(0.f, 0.f, -1.f)).get();
#pragma GCC diagnostic pop

        storage->SetVoidPtr(animStateId, animState);
    }

    const bool isCurrentlyDisabled  = (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) != 0;
    const bool hovered              = ImGui::ItemHoverable(bb, id, ImGuiItemFlags_None);
    const bool pressed              = !isCurrentlyDisabled && hovered && ImGui::IsMouseDown(0);
    const bool clicked              = !isCurrentlyDisabled && hovered && ImGui::IsMouseReleased(0);
    const bool clickedWhileDisabled = isCurrentlyDisabled && hovered && ImGui::IsMouseReleased(0);

    // Update animations
    const float deltaTime = ImGui::GetIO().DeltaTime;

    // Hover animation (tilt)
    animState->hoverAnim = hovered ? ImMin(animState->hoverAnim + deltaTime * 5.f, 1.f) : 0.f;

    // Click animation (scale)
    if (pressed)
        animState->lastClickTime = static_cast<float>(ImGui::GetTime());

    const float timeSinceClick = static_cast<float>(ImGui::GetTime()) - animState->lastClickTime;

    const float clickAnimDir = timeSinceClick > 0.f && timeSinceClick < 0.2f ? 1.f : -1.f;
    animState->clickAnim     = sf::base::clamp(animState->clickAnim + deltaTime * 4.5f * clickAnimDir, 0.f, 1.f);

    // Save current cursor pos
    const ImVec2 originalPos = imGuiWindow->DC.CursorPos;
    ImDrawList*  drawList    = imGuiWindow->DrawList;

    // Calculate center point for transformations
    const ImVec2 center = bb.Min + size * 0.5f;

    // Apply transformations (with a small extra clip so nothing gets cut off)
    const ImVec2 clipMin{bb.Min.x - 20.f, bb.Min.y - 20.f};
    const ImVec2 clipMax{bb.Max.x + 20.f, bb.Max.y + 20.f};
    drawList->PushClipRect(clipMin, clipMax, true);

    // Scale transform: shrink by up to 10% when clicked.
    const float scale = 1.f - easeInOutElastic(animState->clickAnim) * 0.75f;

    // Tilt transform: rotate by up to 0.05 radians (≈2.9°). (Use ~0.0873f for 5°.)
    const float tiltAngle = sf::base::sin(easeInOutSine(animState->hoverAnim) * sf::base::tau) * 0.1f;
    const float tiltCos   = sf::base::cos(tiltAngle);
    const float tiltSin   = sf::base::sin(tiltAngle);

    // Helper lambda: apply scale & rotation about the button center.
    const auto transformPoint = [&](const ImVec2& p) -> ImVec2
    {
        // Translate so that the center is at (0,0)
        const ImVec2 centered = p - center;

        // Apply scale
        const ImVec2 scaled = centered * scale;

        // Apply rotation
        const ImVec2 rotated{scaled.x * tiltCos - scaled.y * tiltSin, scaled.x * tiltSin + scaled.y * tiltCos};

        // Translate back
        return center + rotated;
    };

    // ── Draw Button Background as a Rounded Rectangle ──
    const ImU32 btnBgColor = ImGui::GetColorU32(
        pressed   ? ImGuiCol_ButtonActive
        : hovered ? ImGuiCol_ButtonHovered
                  : ImGuiCol_Button);

    const float rounding = ImGui::GetStyle().FrameRounding;

    drawList->PathClear();
    drawList->PathRect(bb.Min, bb.Max, rounding);

    for (int i = 0; i < drawList->_Path.Size; ++i)
        drawList->_Path[i] = transformPoint(drawList->_Path[i]);

    drawList->PathFillConvex(btnBgColor);

    if (ImGui::GetStyle().FrameBorderSize > 0.f)
    {
        // Recreate the path for the border.
        drawList->PathClear();
        drawList->PathRect(bb.Min, bb.Max, rounding);

        for (int i = 0; i < drawList->_Path.Size; ++i)
            drawList->_Path[i] = transformPoint(drawList->_Path[i]);

        drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Border), ImDrawFlags_Closed, ImGui::GetStyle().FrameBorderSize);
    }

    // ── Draw Text with the Same Transformation ──

    // First, compute the untransformed text position.
    const ImVec2 textPos = bb.Min + (size - labelSize * fontScaleMult) * 0.5f;

    // Record the current vertex buffer size...
    const int vtxBufferSizeBeforeTransformation = drawList->VtxBuffer.Size;

    // ...and add the text at its normal (unrotated/unscaled) position.
    uiSetFontScale(fontScale * fontScaleMult);
    drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), label, labelEnd);

    // Then, apply our transform to only the new text vertices.
    for (int i = vtxBufferSizeBeforeTransformation; i < drawList->VtxBuffer.Size; ++i)
        drawList->VtxBuffer[i].pos = transformPoint(drawList->VtxBuffer[i].pos);

    // Restore the previous clip rect and cursor position.
    drawList->PopClipRect();
    imGuiWindow->DC.CursorPos = originalPos;

    return clicked                ? AnimatedButtonOutcome::Clicked
           : clickedWhileDisabled ? AnimatedButtonOutcome::ClickedWhileDisabled
                                  : AnimatedButtonOutcome::None;
}

////////////////////////////////////////////////////////////
bool Main::uiMakeButtonImpl(const char* label, const char* xBuffer)
{
    const bool  subBullet = label[0] == ' ' && label[1] == ' ';
    const float fontScale = subBullet ? uiSubBulletFontScale : uiNormalFontScale;

    float fontScaleMult = 1.f;

    const auto xBufferLen = SFML_BASE_STRLEN(xBuffer);

    if (xBufferLen > 20)
        fontScaleMult = 0.65f;
    else if (xBufferLen > 19)
        fontScaleMult = 0.7f;
    else if (xBufferLen > 18)
        fontScaleMult = 0.8f;
    else if (xBufferLen > 16)
        fontScaleMult = 0.9f;
    else
        fontScaleMult = 1.f;

    if (fontScale == uiSubBulletFontScale)
        fontScaleMult = std::pow(fontScaleMult, 0.4f);

    const float scaledButtonWidth = uiButtonWidth * profile.uiScale;

    ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - scaledButtonWidth - 2.5f, 0.f)); // Push to right
    ImGui::SameLine();

    uiPushButtonColors();

    bool clicked = false;
    if (const auto outcome = uiAnimatedButton(xBuffer, ImVec2(scaledButtonWidth, 0.f), fontScale, fontScaleMult);
        outcome == AnimatedButtonOutcome::Clicked)
    {
        playSound(sounds.buy);
        clicked = true;

        for (SizeT i = 0u; i < 24u; ++i)
            spawnHUDTopParticle({.position      = getHUDMousePos(),
                                 .velocity      = rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.75f}),
                                 .scale         = rngFast.getF(0.08f, 0.27f) * 0.7f,
                                 .scaleDecay    = 0.f,
                                 .accelerationY = 0.002f,
                                 .opacity       = 1.f,
                                 .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                                 .rotation      = rngFast.getF(0.f, sf::base::tau),
                                 .torque        = rngFast.getF(-0.002f, 0.002f)},
                                /* hue */ wrapHue(165.f + uiButtonHueMod + currentBackgroundHue.asDegrees()),
                                ParticleType::Star);
    }
    else if (outcome == AnimatedButtonOutcome::ClickedWhileDisabled)
    {
        playSound(sounds.failpopui);

        for (SizeT i = 0u; i < 6u; ++i)
            spawnHUDTopParticle({.position      = getHUDMousePos(),
                                 .velocity      = rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.75f}) * 0.5f,
                                 .scale         = rngFast.getF(0.08f, 0.27f),
                                 .scaleDecay    = 0.f,
                                 .accelerationY = 0.002f * 0.75f,
                                 .opacity       = 1.f,
                                 .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                                 .rotation      = rngFast.getF(0.f, sf::base::tau),
                                 .torque        = rngFast.getF(-0.002f, 0.002f)},
                                /* hue */ 0.f,
                                ParticleType::Bubble);
    }

    uiPopButtonColors();

    uiMakeTooltip();

    if (label[0] == ' ')
        uiSetFontScale(uiNormalFontScale);

    ImGui::NextColumn();
    return clicked;
}

////////////////////////////////////////////////////////////
template <typename TCost>
bool Main::makePSVButtonExByCurrency(
    const char*              label,
    PurchasableScalingValue& psv,
    const SizeT              times,
    const TCost              cost,
    TCost&                   availability,
    const char*              currencyFmt)
{
    const bool maxedOut = psv.nPurchases == psv.data->nMaxPurchases;

    if (profile.hideMaxedOutPurchasables && maxedOut)
        return false;

    bool result = false;

    if (maxedOut)
        std::sprintf(uiBuffer, "MAX##%u", uiWidgetId++);
    else if (cost == 0u || times == 0u)
        std::sprintf(uiBuffer, "N/A##%u", uiWidgetId++);
    else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        std::sprintf(uiBuffer, currencyFmt, toStringWithSeparators(cost), uiWidgetId++);
#pragma GCC diagnostic pop

    ImGui::BeginDisabled(uiCheckPurchasability(label, maxedOut || availability < cost || cost == 0u));

    uiMakeButtonLabels(label, uiLabelBuffer);
    if (uiMakeButtonImpl(label, uiBuffer))
    {
        result = true;
        availability -= cost;

        if (&availability == &pt->money)
            spentMoney += cost;

        psv.nPurchases += times;

        if (&availability == &pt->prestigePoints && times == 1u)
        {
            undoPPPurchase.emplaceBack([&psv, &availability, times, cost]
            {
                psv.nPurchases -= times;
                availability += cost;
            });

            undoPPPurchaseTimer.value = 10000.f;
        }
    }

    ImGui::EndDisabled();
    return result;
}


////////////////////////////////////////////////////////////
bool Main::makePSVButtonEx(const char* label, PurchasableScalingValue& psv, const SizeT times, const MoneyType cost)
{
    return makePSVButtonExByCurrency(label, psv, times, cost, pt->money, "$%s##%u");
}

////////////////////////////////////////////////////////////
bool Main::makePSVButton(const char* label, PurchasableScalingValue& psv)
{
    return makePSVButtonEx(label, psv, 1u, static_cast<MoneyType>(psv.nextCost()));
}

////////////////////////////////////////////////////////////
bool Main::uiCheckPurchasability(const char* label, const bool disabled)
{
    uiLabelToY[label] = ImGui::GetCursorScreenPos().y;

    if (disabled)
    {
        btnWasDisabled[label] = true;
    }
    else if (btnWasDisabled[label] && !disabled)
    {
        btnWasDisabled[label] = false;

        const bool anyPurchaseUnlockedEffectWithSameLabel = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                            purchaseUnlockedEffects.end(),
                                                                            [&](const PurchaseUnlockedEffect& effect)
        { return effect.widgetLabel == label; });

        const bool anyPurchaseUnlockedEffectWithSameY = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                        purchaseUnlockedEffects.end(),
                                                                        [&](const PurchaseUnlockedEffect& effect)
        {
            const auto* it = uiLabelToY.find(effect.widgetLabel);
            return it != uiLabelToY.end() && it->second == uiLabelToY[label];
        });

        if (!anyPurchaseUnlockedEffectWithSameLabel && !anyPurchaseUnlockedEffectWithSameY)
        {
            purchaseUnlockedEffects.pushBack({
                .widgetLabel    = label,
                .countdown      = Countdown{.value = 1000.f},
                .arrowCountdown = Countdown{.value = 2000.f},
                .hue            = uiButtonHueMod,
                .type           = 1, // now purchasable
            });

            playSound(sounds.purchasable, /* maxOverlap */ 1u);
            playSound(sounds.shimmer, /* maxOverlap */ 1u);
        }
    }

    return disabled;
}

////////////////////////////////////////////////////////////
template <typename TCost>
bool Main::makePurchasableButtonOneTimeByCurrency(
    const char* label,
    bool&       done,
    const TCost cost,
    TCost&      availability,
    const char* currencyFmt)
{
    bool result = false;

    if (done)
        std::sprintf(uiBuffer, "DONE##%u", uiWidgetId++);
    else if (cost == 0u)
        std::sprintf(uiBuffer, "FREE##%u", uiWidgetId++);
    else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        std::sprintf(uiBuffer, currencyFmt, toStringWithSeparators(cost), uiWidgetId++);
#pragma GCC diagnostic pop

    ImGui::BeginDisabled(uiCheckPurchasability(label, done || availability < cost));

    uiMakeButtonLabels(label, uiLabelBuffer);
    if (uiMakeButtonImpl(label, uiBuffer))
    {
        result = true;
        availability -= cost;

        if (&availability == &pt->money)
            spentMoney += cost;

        done = true;

        if (&availability == &pt->prestigePoints && cost > 0u)
        {
            undoPPPurchase.emplaceBack([&availability, &done, cost]
            {
                done = false;
                availability += cost;
            });

            undoPPPurchaseTimer.value = 10000.f;
        }
    }

    ImGui::EndDisabled();
    return result;
}

////////////////////////////////////////////////////////////
bool Main::makePurchasableButtonOneTime(const char* label, const MoneyType cost, bool& done)
{
    return makePurchasableButtonOneTimeByCurrency(label, done, cost, pt->money, "$%s##%u");
}

////////////////////////////////////////////////////////////
bool Main::makePurchasablePPButtonOneTime(const char* label, const PrestigePointsType prestigePointsCost, bool& done)
{
    return makePurchasableButtonOneTimeByCurrency(label, done, prestigePointsCost, pt->prestigePoints, "%s PPs##%u");
}

////////////////////////////////////////////////////////////
bool Main::makePrestigePurchasablePPButtonPSV(const char* label, PurchasableScalingValue& psv)
{
    return makePSVButtonExByCurrency(label,
                                     psv,
                                     /* times */ 1u,
                                     /* cost */ static_cast<PrestigePointsType>(psv.nextCost()),
                                     /* availability */ pt->prestigePoints,
                                     "%s PPs##%u");
}

////////////////////////////////////////////////////////////
void Main::uiBeginColumns() const
{
    ImGui::Columns(2, "twoColumns", false);
    ImGui::SetColumnWidth(0, (uiWindowWidth - uiButtonWidth - 40.f) * profile.uiScale);
    ImGui::SetColumnWidth(1, (uiButtonWidth + 10.f) * profile.uiScale);
}

////////////////////////////////////////////////////////////
void Main::uiCenteredText(const char* str, const float offsetX, const float offsetY)
{
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(str).x) * 0.5f + offsetX);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

    ImGui::Text("%s", str);
}

////////////////////////////////////////////////////////////
void Main::uiCenteredTextColored(const sf::Color color, const char* str, const float offsetX, const float offsetY)
{
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(str).x) * 0.5f + offsetX);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

    ImGui::TextColored(color.toVec4<ImVec4>(), "%s", str);
}

////////////////////////////////////////////////////////////
sf::Vec2f Main::uiGetWindowPos() const
{
    const float ratio = getAspectRatioScalingFactor(gameScreenSize, getResolution());

    const float scaledWindowWidth = uiWindowWidth * profile.uiScale;

    const float rightAnchorX = sf::base::min(getResolution().x - scaledWindowWidth - 15.f * profile.uiScale,
                                             gameScreenSize.x * ratio + 30.f * profile.uiScale);

    return {rightAnchorX, 15.f};
}

////////////////////////////////////////////////////////////
void Main::uiDrawExitPopup(const float newScalingFactor)
{
    if (!escWasPressed)
        return;

    constexpr float scaleMult = 1.25f;

    ImGui::SetNextWindowPos({getResolution().x / 2.f, getResolution().y / 2.f}, 0, {0.5f, 0.5f});
    ImGui::SetNextWindowSizeConstraints(ImVec2(400.f * scaleMult * newScalingFactor, 0.f),
                                        ImVec2(400.f * scaleMult * newScalingFactor, 300.f * scaleMult * newScalingFactor));

    ImGui::Begin("##exit",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoTitleBar);

    uiSetFontScale(scaleMult);

    uiCenteredText("Are you sure you want to exit?");

    ImGui::Dummy(ImVec2(0.f, 10.f * scaleMult));
    const float exitButtonWidth = 60.f * scaleMult * profile.uiScale;

    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - exitButtonWidth * 2.f - 5.f) * 0.5f);

    ImGui::BeginGroup();

    if (ImGui::Button("Yes", ImVec2(exitButtonWidth, 0.f)))
    {
        playSound(sounds.uitab);
        mustExit = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("No", ImVec2(exitButtonWidth, 0.f)))
    {
        playSound(sounds.uitab);
        escWasPressed = false;
    }

    ImGui::EndGroup();

    uiSetFontScale(uiNormalFontScale);

    ImGui::End();
}


////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarCopyCat(const sf::Vec2f quickBarPos, Cat& copyCat)
{
    const bool asWitchAndBusy = pt->copycatCopiedCatType == CatType::Witch && (anyCatCopyHexed() || !pt->copyDolls.empty());

    const bool asWizardAndBusy = pt->copycatCopiedCatType == CatType::Wizard && isWizardBusy();

    const bool mustDisable = asWitchAndBusy || asWizardAndBusy;

    ImGui::BeginDisabled(mustDisable);

    constexpr const char* popupLabel = "CopyCatSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconCopyCat,
                     {.scale = {0.65f, 0.65f},
                      .color = (mustDisable ? sf::Color::Gray : sf::Color::White).withAlpha(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;


    std::sprintf(uiTooltipBuffer, "Select Copycat mask");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        ImGui::SetNextItemWidth(210.f * profile.uiScale);

        if (ImGui::BeginCombo("##copycatsel", CatConstants::typeNamesLong[asIdx(pt->copycatCopiedCatType)]))
        {
            for (SizeT i = asIdx(CatType::Normal); i < nCatTypes; ++i)
            {
                if (static_cast<CatType>(i) == CatType::Duck)
                    continue;

                if (!isUniqueCatType(static_cast<CatType>(i)))
                    continue;

                if (i == asIdx(CatType::Copy))
                    continue;

                if (findFirstCatByType(static_cast<CatType>(i)) == nullptr)
                    continue;

                const bool isSelected = pt->copycatCopiedCatType == static_cast<CatType>(i);
                if (ImGui::Selectable(CatConstants::typeNamesLong[i], isSelected))
                {
                    statDisguise();
                    pt->copycatCopiedCatType = static_cast<CatType>(i);

                    copyCat.cooldown.value = pt->getComputedCooldownByCatType(pt->copycatCopiedCatType);
                    copyCat.hits           = 0u;

                    sounds.smokebomb.settings.position = {copyCat.position.x, copyCat.position.y};
                    sounds.smokebomb.settings.position = {copyCat.position.x, copyCat.position.y};
                    playSound(sounds.smokebomb);

                    for (sf::base::SizeT iP = 0u; iP < 8u; ++iP)
                        spawnParticle(ParticleData{.position   = copyCat.position,
                                                   .velocity   = {rngFast.getF(-0.15f, 0.15f), rngFast.getF(0.f, 0.1f)},
                                                   .scale      = rngFast.getF(0.75f, 1.f),
                                                   .scaleDecay = -0.0005f,
                                                   .accelerationY = -0.00017f,
                                                   .opacity       = 1.f,
                                                   .opacityDecay  = rngFast.getF(0.00065f, 0.00075f),
                                                   .rotation      = rngFast.getF(0.f, sf::base::tau),
                                                   .torque        = rngFast.getF(-0.002f, 0.002f)},
                                      0.f,
                                      ParticleType::Smoke);

                    playSound(sounds.uitab);
                    ImGui::CloseCurrentPopup();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::EndPopup();
    }

    ImGui::EndDisabled();
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarBackgroundSelector(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "BackgroundSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconBg, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiTooltipBuffer, "Select background");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        auto& [entries, selectedIndex] = getBackgroundSelectorData();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);

        if (ImGui::BeginCombo("##backgroundsel", entries[static_cast<sf::base::SizeT>(selectedIndex)].name))
        {
            for (SizeT i = 0u; i < entries.size(); ++i)
            {
                const bool isSelected = selectedIndex == static_cast<int>(i);
                if (ImGui::Selectable(entries[i].name, isSelected))
                {
                    selectedIndex = static_cast<int>(i);

                    selectBackground(entries, static_cast<int>(i));

                    playSound(sounds.uitab);
                    ImGui::CloseCurrentPopup();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarBGMSelector(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "MusicSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconBGM, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiTooltipBuffer, "Select music");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        auto& [entries, selectedIndex] = getBGMSelectorData();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);

        if (ImGui::BeginCombo("##musicsel", entries[static_cast<sf::base::SizeT>(selectedIndex)].name))
        {
            for (SizeT i = 0u; i < entries.size(); ++i)
            {
                const bool isSelected = selectedIndex == static_cast<int>(i);
                if (ImGui::Selectable(entries[i].name, isSelected))
                {
                    selectedIndex = static_cast<int>(i);

                    selectBGM(entries, static_cast<int>(i));
                    switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ false);

                    playSound(sounds.uitab);
                    ImGui::CloseCurrentPopup();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarQuickSettings(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "QuickSettingsPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconCfg, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiTooltipBuffer, "Quick settings");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        uiCheckbox("Enable tips", &profile.tipsEnabled);
        uiCheckbox("Enable notifications", &profile.enableNotifications);

        ImGui::Separator();

        uiCheckbox("Enable $/s meter", &profile.showDpsMeter);

        ImGui::Separator();

        uiCheckbox("Show cat range", &profile.showCatRange);
        uiCheckbox("Show cat text", &profile.showCatText);

        ImGui::Separator();

        uiCheckbox("Show particles", &profile.showParticles);

        ImGui::BeginDisabled(!profile.showParticles);
        uiCheckbox("Show coin particles", &profile.showCoinParticles);
        ImGui::EndDisabled();

        uiCheckbox("Show text particles", &profile.showTextParticles);

        ImGui::Separator();

        uiCheckbox("Enable screen shake", &profile.enableScreenShake);

        ImGui::Separator();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Background Opacity", &profile.backgroundOpacity, 0.f, 100.f, "%.f%%");

        uiCheckbox("Always show drawings", &profile.alwaysShowDrawings);

        ImGui::Separator();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Minimap Scale", &profile.minimapScale, 5.f, 40.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("HUD Scale", &profile.hudScale, 0.5f, 2.f, "%.2f");

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

        uiCheckbox("Bubble shader", &profile.useBubbleShader);

        ImGui::BeginDisabled(!profile.useBubbleShader);
        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Bubble Lightness", &profile.bsBubbleLightness, -1.f, 1.f, "%.2f");
        ImGui::EndDisabled();

        ImGui::Separator();

        if (uiCheckbox("VSync", &profile.vsync))
            window.setVerticalSyncEnabled(profile.vsync);

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarVolumeControls(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "VolumeSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconVolume, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiTooltipBuffer, "Volume settings");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Master##popupmastervolume", &profile.masterVolume, 0.f, 100.f, "%.f%%");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        if (ImGui::SliderFloat("SFX##popupsfxvolume", &profile.sfxVolume, 0.f, 100.f, "%.f%%"))
            sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Music##popupmusicvolume", &profile.musicVolume, 0.f, 100.f, "%.f%%");

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbar()
{
    const float xStartOverlay = getAspectRatioScalingFactor(gameScreenSize, getResolution()) * gameScreenSize.x;

    const float xStart = lastUiSelectedTabIdx == 0 ? xStartOverlay : sf::base::min(xStartOverlay, uiGetWindowPos().x);

    const sf::Vec2f quickBarPos{xStart - 15.f, getResolution().y - 15.f};

    ImGui::SetNextWindowPos({quickBarPos.x, quickBarPos.y}, 0, {1.f, 1.f});
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(SFML_BASE_FLOAT_MAX, SFML_BASE_FLOAT_MAX));

    ImGui::Begin("##quickmenu",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    if (cachedCopyCat != nullptr)
        uiDrawQuickbarCopyCat(quickBarPos, *cachedCopyCat);

    if (getBackgroundSelectorData().entries.size() > 1u)
        uiDrawQuickbarBackgroundSelector(quickBarPos);

    if (getBGMSelectorData().entries.size() > 1u)
        uiDrawQuickbarBGMSelector(quickBarPos);

    uiDrawQuickbarVolumeControls(quickBarPos);

    uiDrawQuickbarQuickSettings(quickBarPos);

    ImGui::End();
}

////////////////////////////////////////////////////////////
struct ImGuiStyleScales
{
    const ImVec2 windowPadding;
    const float  windowRounding;
    const ImVec2 windowMinSize;
    const float  childRounding;
    const float  popupRounding;
    const ImVec2 framePadding;
    const float  frameRounding;
    const ImVec2 itemSpacing;
    const ImVec2 itemInnerSpacing;
    const ImVec2 cellPadding;
    const ImVec2 touchExtraPadding;
    const float  indentSpacing;
    const float  columnsMinSpacing;
    const float  scrollbarSize;
    const float  scrollbarRounding;
    const float  grabMinSize;
    const float  grabRounding;
    const float  logSliderDeadzone;
    const float  tabRounding;
    const float  tabCloseButtonMinWidthUnselected;
    const float  tabBarOverlineSize;
    const ImVec2 separatorTextPadding;
    const ImVec2 displayWindowPadding;
    const ImVec2 displaySafeAreaPadding;
    const float  mouseCursorScale;

    void applyWithScale(ImGuiStyle& style, const float scale, const bool steamDeck) const
    {
        style.WindowPadding                    = windowPadding * scale;
        style.WindowRounding                   = windowRounding * scale;
        style.WindowMinSize                    = windowMinSize * scale;
        style.ChildRounding                    = childRounding * scale;
        style.PopupRounding                    = popupRounding * scale;
        style.FramePadding                     = framePadding * scale;
        style.FrameRounding                    = frameRounding * scale;
        style.ItemSpacing                      = itemSpacing * scale;
        style.ItemInnerSpacing                 = itemInnerSpacing * scale;
        style.CellPadding                      = cellPadding * scale;
        style.TouchExtraPadding                = touchExtraPadding * scale;
        style.IndentSpacing                    = indentSpacing * scale;
        style.ColumnsMinSpacing                = columnsMinSpacing * scale;
        style.ScrollbarSize                    = scrollbarSize * scale * (steamDeck ? 2.f : 0.65f);
        style.ScrollbarRounding                = scrollbarRounding * scale;
        style.GrabMinSize                      = grabMinSize * scale;
        style.GrabRounding                     = grabRounding * scale;
        style.LogSliderDeadzone                = logSliderDeadzone * scale;
        style.TabRounding                      = tabRounding * scale;
        style.TabCloseButtonMinWidthUnselected = tabCloseButtonMinWidthUnselected * scale;
        style.TabBarOverlineSize               = tabBarOverlineSize * scale;
        style.SeparatorTextPadding             = separatorTextPadding * scale;
        style.DisplayWindowPadding             = displayWindowPadding * scale;
        style.DisplaySafeAreaPadding           = displaySafeAreaPadding * scale;
        style.MouseCursorScale                 = mouseCursorScale * scale;
    }
};

////////////////////////////////////////////////////////////
void Main::uiDraw(const sf::Vec2f mousePos)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.FrameBorderSize = 2.f;
    style.FrameRounding   = 10.f;
    style.WindowRounding  = 5.f;

    static const auto initialStyleScales = [&]() -> ImGuiStyleScales
    {
        return {
            .windowPadding                    = style.WindowPadding,
            .windowRounding                   = style.WindowRounding,
            .windowMinSize                    = style.WindowMinSize,
            .childRounding                    = style.ChildRounding,
            .popupRounding                    = style.PopupRounding,
            .framePadding                     = style.FramePadding,
            .frameRounding                    = style.FrameRounding,
            .itemSpacing                      = style.ItemSpacing,
            .itemInnerSpacing                 = style.ItemInnerSpacing,
            .cellPadding                      = style.CellPadding,
            .touchExtraPadding                = style.TouchExtraPadding,
            .indentSpacing                    = style.IndentSpacing,
            .columnsMinSpacing                = style.ColumnsMinSpacing,
            .scrollbarSize                    = style.ScrollbarSize,
            .scrollbarRounding                = style.ScrollbarRounding,
            .grabMinSize                      = style.GrabMinSize,
            .grabRounding                     = style.GrabRounding,
            .logSliderDeadzone                = style.LogSliderDeadzone,
            .tabRounding                      = style.TabRounding,
            .tabCloseButtonMinWidthUnselected = style.TabCloseButtonMinWidthUnselected,
            .tabBarOverlineSize               = style.TabBarOverlineSize,
            .separatorTextPadding             = style.SeparatorTextPadding,
            .displayWindowPadding             = style.DisplayWindowPadding,
            .displaySafeAreaPadding           = style.DisplaySafeAreaPadding,
            .mouseCursorScale                 = style.MouseCursorScale,
        };
    }();

    uiWidgetId = 0u;

    ImGui::PushFont(fontImGuiSuperBakery);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f); // Set corner radius

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.f, 0.65f); // 65% transparent black
    style.Colors[ImGuiCol_Border]   = colorBlueOutline.toVec4<ImVec4>();

    const float newScalingFactor = profile.uiScale;
    initialStyleScales.applyWithScale(style, newScalingFactor, onSteamDeck);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

    ImGui::GetIO().FontGlobalScale = newScalingFactor;

    if (profile.showDpsMeter && !debugHideUI)
        uiDpsMeter();

    if (inSpeedrunPlaythrough())
        uiSpeedrunning();

    if (!debugHideUI)
        uiDrawQuickbar();

    ImGui::SetNextWindowPos({uiGetWindowPos().x, uiGetWindowPos().y}, 0, {0.f, 0.f});
    ImGui::SetNextWindowSizeConstraints(ImVec2(uiWindowWidth * newScalingFactor, 0.f),
                                        ImVec2(uiWindowWidth * newScalingFactor, uiGetMaxWindowHeight() * newScalingFactor));

    ImGui::Begin("##menu",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoTitleBar);


    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_DrawSelectedOverline))
    {
        uiTabBar();
        ImGui::EndTabBar();
    }

    if (!ImGui::GetIO().WantCaptureMouse && particleCullingBoundaries.isInside(mousePos))
        uiMakeShrineOrCatTooltip(mousePos);

    ImGui::End();

    uiDrawExitPopup(newScalingFactor);

    ImGui::PopStyleVar();
    ImGui::PopFont();
}

////////////////////////////////////////////////////////////
void Main::uiDpsMeter()
{
    const auto resolution = getResolution();

    const float  dpsMeterScale = profile.uiScale;
    const ImVec2 dpsMeterSize(240.f * dpsMeterScale, 65.f * dpsMeterScale);

    ImGui::SetNextWindowPos({15.f, resolution.y - 15.f}, 0, {0.f, 1.f});
    ImGui::SetNextWindowSizeConstraints(dpsMeterSize, dpsMeterSize);

    ImGui::Begin("##dpsmeter",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoTitleBar);

    uiSetFontScale(0.75f);

    static thread_local sf::base::Vector<float> sampleBuffer(60);
    samplerMoneyPerSecond.writeSamplesInOrder(sampleBuffer.data());

    const auto average = static_cast<MoneyType>(samplerMoneyPerSecond.getAverageAs<double>());

    static thread_local char avgBuffer[64];
    std::sprintf(avgBuffer, "%s $/s", toStringWithSeparators(average));

    ImGui::PlotLines("##dpsmeter",
                     sampleBuffer.data(),
                     static_cast<int>(sampleBuffer.size()),
                     0,
                     avgBuffer,
                     0.f,
                     SFML_BASE_FLOAT_MAX,
                     ImVec2(dpsMeterSize.x - 15.f * dpsMeterScale, dpsMeterSize.y - 17.f * dpsMeterScale));

    ImGui::End();
}

////////////////////////////////////////////////////////////
void Main::uiSpeedrunning()
{
    const auto resolution = getResolution();

    const float  dpsMeterScale = profile.uiScale;
    const ImVec2 dpsMeterSize(240.f * dpsMeterScale, 65.f * dpsMeterScale);
    const ImVec2 speedrunningWindowSize(240.f * 2.f * dpsMeterScale - 45.f, 1.f * 65.f * dpsMeterScale);

    ImGui::SetNextWindowPos({dpsMeterSize.x + 15.f + 15.f, resolution.y - 15.f}, 0, {0.f, 1.f});
    ImGui::SetNextWindowSizeConstraints(speedrunningWindowSize, speedrunningWindowSize);

    ImGui::Begin("##speedrunning",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoTitleBar);

    uiSetFontScale(0.55f);

    ImGui::Text("Game Version: %s", BUBBLEBYTE_VERSION_STR);

    ImGui::SetCursorPosY(21.f * profile.uiScale);
    uiSetFontScale(1.40f);

    if (pt->speedrunStartTime.hasValue())
    {
        const auto [hours, mins, secs, millis] = formatSpeedrunTime(sf::Clock::now() - *(pt->speedrunStartTime));
        ImGui::Text("%02llu:%02llu:%02llu:%03llu", hours, mins, secs, millis);
    }
    else
    {
        ImGui::Text("POP TO START");
    }

    uiSetFontScale(0.75f);


    const ImVec4 textColorLocked{1.f, 1.f, 1.f, 0.5f};
    const ImVec4 textColorUnlocked{1.f, 1.f, 1.f, 1.f};

    const auto textSplit = [&](const char* title, const MilestoneTimestamp split)
    {
        if (split == maxMilestone)
        {
            ImGui::TextColored(textColorLocked, "%s: XX:XX:XX:XXX", title);
        }
        else
        {
            const auto [hours, mins, secs, millis] = formatSpeedrunTime(sf::Time{static_cast<sf::base::I64>(split)});
            ImGui::TextColored(textColorUnlocked, "%s: %02llu:%02llu:%02llu:%03llu", title, hours, mins, secs, millis);
        }
    };

    ImGui::SetCursorPosY(10.f * profile.uiScale);
    ImGui::SetCursorPosX(205.f * profile.uiScale);

    ImGui::BeginGroup();
    textSplit("Prestige Lv.2", pt->speedrunSplits.prestigeLevel2);
    textSplit("Prestige Lv.3", pt->speedrunSplits.prestigeLevel3);
    ImGui::EndGroup();

    /*
    // TODO P2: balance and enable
    ImGui::SetCursorPosY(10.f * profile.uiScale);
    ImGui::SetCursorPosX((250.f + 230.f) * profile.uiScale);

    ImGui::BeginGroup();
    textSplit("Prestige Lv.4", pt->speedrunSplits.prestigeLevel4);
    textSplit("Prestige Lv.5", pt->speedrunSplits.prestigeLevel5);
    ImGui::EndGroup();
    */

    ImGui::End();
}

////////////////////////////////////////////////////////////
void Main::uiTabBar()
{
    const float childHeight = uiGetMaxWindowHeight() - (60.f * profile.uiScale);

    const auto keyboardSelectedTab = [&](const sf::Keyboard::Key key)
    {
        return !ImGui::GetIO().WantCaptureKeyboard && inputHelper.wasKeyJustPressed(key)
                   ? ImGuiTabItemFlags_SetSelected
                   : ImGuiTabItemFlags_{};
    };

    const auto selectedTab = [&](int idx)
    {
        if (shopSelectOnce == ImGuiTabItemFlags_{} && lastUiSelectedTabIdx != idx)
            playSound(sounds.uitab);

        lastUiSelectedTabIdx = idx;
    };

    if (ImGui::BeginTabItem("X",
                            nullptr,
                            keyboardSelectedTab(sf::Keyboard::Key::Slash) | keyboardSelectedTab(sf::Keyboard::Key::Grave) |
                                keyboardSelectedTab(sf::Keyboard::Key::Apostrophe) |
                                keyboardSelectedTab(sf::Keyboard::Key::Backslash)))
    {
        selectedTab(0);

        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Shop", nullptr, shopSelectOnce | keyboardSelectedTab(sf::Keyboard::Key::Num1)))
    {
        selectedTab(1);

        shopSelectOnce = {};

        ImGui::BeginChild("ShopScroll",
                          ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                          0,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar);
        uiTabBarShop();

        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    sf::base::SizeT nextTabKeyIndex = 0u;

    constexpr sf::Keyboard::Key tabKeys[] = {
        sf::Keyboard::Key::Num2,
        sf::Keyboard::Key::Num3,
        sf::Keyboard::Key::Num4,
        sf::Keyboard::Key::Num5,
        sf::Keyboard::Key::Num6,
    };

    if (cachedWizardCat != nullptr &&
        ImGui::BeginTabItem("Magic", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
    {
        selectedTab(2);

        ImGui::BeginChild("MagicScroll",
                          ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                          0,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar);

        uiTabBarMagic();

        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    if (pt->isBubbleValueUnlocked())
    {
        if (!pt->prestigeTipShown)
        {
            pt->prestigeTipShown = true;
            doTip("Prestige to increase bubble value\nand unlock permanent upgrades!");
        }

        const bool canPrestige = pt->canBuyNextPrestige();

        if (canPrestige)
        {
            ImGui::PushStyleColor(ImGuiCol_Tab, IM_COL32(135, 50, 84, 255));
            ImGui::PushStyleColor(ImGuiCol_TabHovered, IM_COL32(136, 65, 105, 255));
            ImGui::PushStyleColor(ImGuiCol_TabSelected, IM_COL32(136, 65, 105, 255));
        }

        if (ImGui::BeginTabItem("Prestige", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
        {
            selectedTab(3);

            ImGui::BeginChild("PrestigeScroll",
                              ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                              0,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar);

            uiTabBarPrestige();

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (canPrestige)
            ImGui::PopStyleColor(3);
    }

    if (ImGui::BeginTabItem("Info", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
    {
        selectedTab(4);

        ImGui::BeginChild("StatsScroll",
                          ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                          0,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar);

        uiTabBarStats();

        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Options", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
    {
        selectedTab(5);

        ImGui::BeginChild("OptionsScroll",
                          ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                          0,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar);

        uiTabBarSettings();

        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

////////////////////////////////////////////////////////////
void Main::uiSetUnlockLabelY(const sf::base::SizeT unlockId)
{
    const std::string label = std::to_string(unlockId);
    uiLabelToY[label]       = ImGui::GetCursorScreenPos().y;
}

////////////////////////////////////////////////////////////
bool Main::checkUiUnlock(const sf::base::SizeT unlockId, const bool unlockCondition)
{
    const std::string label = std::to_string(unlockId);

    if (!unlockCondition)
    {
        profile.uiUnlocks[unlockId] = false;
        return false;
    }

    if (!profile.uiUnlocks[unlockId])
    {
        profile.uiUnlocks[unlockId] = true;

        const bool anyPurchaseUnlockedEffectWithSameLabel = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                            purchaseUnlockedEffects.end(),
                                                                            [&](const PurchaseUnlockedEffect& effect)
        { return effect.widgetLabel == label; });

        const bool anyPurchaseUnlockedEffectWithSameY = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                        purchaseUnlockedEffects.end(),
                                                                        [&](const PurchaseUnlockedEffect& effect)
        {
            const auto* it = uiLabelToY.find(effect.widgetLabel);
            return it != uiLabelToY.end() && it->second == uiLabelToY[label];
        });

        if (!anyPurchaseUnlockedEffectWithSameLabel && !anyPurchaseUnlockedEffectWithSameY)
        {
            purchaseUnlockedEffects.pushBack({
                .widgetLabel    = label,
                .countdown      = Countdown{.value = 1000.f},
                .arrowCountdown = Countdown{.value = 2000.f},
                .hue            = uiButtonHueMod,
                .type           = 0, // now unlocked
            });

            playSound(sounds.unlock, /* maxOverlap */ 1u);
            playSound(sounds.shimmer, /* maxOverlap */ 1u);
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
void Main::uiImageFromAtlas(const sf::Rect2f& txr, const sf::RenderTarget::TextureDrawParams& drawParams)
{
    imGuiContext.image(
        sf::Sprite{
            .position    = drawParams.position,
            .scale       = drawParams.scale * profile.uiScale,
            .origin      = txr.size,
            .textureRect = txr,
        },
        uiTextureAtlas.getTexture(),
        drawParams.color);
}

////////////////////////////////////////////////////////////
void Main::uiImgsep(const sf::Rect2f& txr, const char* sepLabel, const bool first)
{
    if (profile.hideCategorySeparators)
        return;

    if (!first)
    {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
    }

    ImGui::Columns(1);
    uiImageFromAtlas(txr, {.scale = {0.46f, 0.5f}});

    const auto oldFontScale = lastFontScale;
    uiSetFontScale(0.75f);
    uiCenteredText(sepLabel, -5.f * profile.uiScale, -8.f * profile.uiScale);
    uiSetFontScale(oldFontScale);

    ImGui::Separator();
    ImGui::Spacing();

    uiBeginColumns();
}

////////////////////////////////////////////////////////////
void Main::uiImgsep2(const sf::Rect2f& txr, const char* sepLabel)
{
    if (profile.hideCategorySeparators)
        return;

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Columns(1);

    uiImageFromAtlas(txr, {.scale = {0.46f, 0.5f}});

    const auto oldFontScale = lastFontScale;
    uiSetFontScale(0.75f);
    uiCenteredText(sepLabel, -5.f * profile.uiScale, -6.f * profile.uiScale);
    uiSetFontScale(oldFontScale);

    uiBeginColumns();
}

////////////////////////////////////////////////////////////
void Main::uiTabBarShop()
{
    const auto nCatNormal = pt->getCatCountByType(CatType::Normal);
    const auto nCatUni    = pt->getCatCountByType(CatType::Uni);
    const auto nCatDevil  = pt->getCatCountByType(CatType::Devil);
    const auto nCatAstro  = pt->getCatCountByType(CatType::Astro);

    Cat* catWitch    = cachedWitchCat;
    Cat* catWizard   = cachedWizardCat;
    Cat* catMouse    = cachedMouseCat;
    Cat* catEngi     = cachedEngiCat;
    Cat* catRepulso  = cachedRepulsoCat;
    Cat* catAttracto = cachedAttractoCat;

    const bool anyUniqueCat = catWitch != nullptr || catWizard != nullptr || catMouse != nullptr ||
                              catEngi != nullptr || catRepulso != nullptr || catAttracto != nullptr;

    ImGui::Spacing();
    ImGui::Spacing();

    uiImgsep(txrMenuSeparator0, "click upgrades", /* first */ true);

    std::sprintf(uiTooltipBuffer,
                 "Build your combo by popping bubbles quickly, increasing the value of each subsequent "
                 "one.\n\nCombos expire on misclicks and over time, but can be upgraded to last "
                 "longer.\n\nStar bubbles are affected -- pop them while your multiplier is high!");
    uiLabelBuffer[0] = '\0';
    if (makePurchasableButtonOneTime("Combo", 20u, pt->comboPurchased))
    {
        combo = 0;
        doTip("Pop bubbles quickly to keep\nyour combo up and make more money!");
    }

    if (checkUiUnlock(0u, pt->comboPurchased))
    {
        const char* mouseNote = catMouse == nullptr ? "" : "\n\n(Note: this also applies to the Mousecat!)";

        const float currentComboStartTime = pt->psvComboStartTime.currentValue();
        const float nextComboStartTime    = pt->psvComboStartTime.nextValue();

        uiSetUnlockLabelY(0u);
        if (!pt->psvComboStartTime.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase combo duration from %.2fs to %.2fs. We are in it for the long haul!%s",
                         static_cast<double>(currentComboStartTime),
                         static_cast<double>(nextComboStartTime),
                         mouseNote);
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase combo duration (MAX). We are in it for the long haul!%s", mouseNote);
        }

        std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentComboStartTime));
        makePSVButton("  Longer combo", pt->psvComboStartTime);
    }

    if (checkUiUnlock(1u, nCatNormal > 0 && pt->psvComboStartTime.nPurchases > 0))
    {
        uiImgsep(txrMenuSeparator1, "exploration");

        if constexpr (isDemoVersion)
        {
            ImGui::Columns(1);
            uiSetFontScale(uiNormalFontScale);
            uiCenteredTextColored({229u, 63u, 63u, 255u}, "(!) Limited to 2 shrines in demo (!)");
            uiBeginColumns();
        }

        uiSetUnlockLabelY(1u);
        std::sprintf(uiTooltipBuffer,
                     "Extend the map and enable scrolling.\n\nExtending the map will increase the total number "
                     "of "
                     "bubbles you can work with, and will also reveal magical shrines that grant unique cats "
                     "upon "
                     "completion.\n\nYou can scroll the map with the scroll wheel, holding right click, by "
                     "dragging with two fingers, by using the A/D/Left/Right keys.\n\nYou can jump around the "
                     "map "
                     "by clicking on the minimap or using the PgUp/PgDn/Home/End keys.");
        uiLabelBuffer[0] = '\0';
        if (makePurchasableButtonOneTime("Map scrolling", 1000u, pt->mapPurchased))
        {
            scroll = 0.f;
            doTip(
                "Explore the map by using the mouse wheel,\ndragging via right click, or with your "
                "keyboard.\nYou can also click on the minimap!");

            if (pt->psvBubbleValue.nPurchases == 0u)
                scrollArrowCountdown.value = 2000.f;
        }

        if (checkUiUnlock(2u, pt->mapPurchased))
        {
            uiSetUnlockLabelY(2u);
            std::sprintf(uiTooltipBuffer, "Extend the map further by one screen.");
            std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(pt->getMapLimit() / boundaries.x * 100.f));
            makePSVButton("  Extend map", pt->psvMapExtension);

            ImGui::BeginDisabled(pt->psvShrineActivation.nPurchases > pt->psvMapExtension.nPurchases);
            std::sprintf(uiTooltipBuffer,
                         "Activates the next shrine, enabling it to absorb nearby popped bubbles. Once enough "
                         "bubbles are absorbed by a shrine, it will grant a unique cat.");
            std::sprintf(uiLabelBuffer, "%zu/9", pt->psvShrineActivation.nPurchases);
            if (makePSVButton("  Activate next shrine", pt->psvShrineActivation))
            {
                if (!pt->shrineActivateTipShown)
                {
                    pt->shrineActivateTipShown = true;

                    if (pt->psvBubbleValue.nPurchases == 0u)
                    {
                        doTip(
                            "Move your cats near the shrine to help\n it absorb bubbles and unlock a unique "
                            "cat!\nWill there be consequences?");
                    }
                }
            }
            ImGui::EndDisabled();
        }
    }

    if (checkUiUnlock(3u, nCatNormal > 0 && pt->psvComboStartTime.nPurchases > 0))
    {
        uiImgsep(txrMenuSeparator2, "bubble upgrades");

        uiSetUnlockLabelY(3u);
        std::sprintf(uiTooltipBuffer,
                     "Increase the total number of bubbles. Scales with map size.\n\nMore bubbles, "
                     "more money, fewer FPS!");
        std::sprintf(uiLabelBuffer, "%zu bubbles", static_cast<SizeT>(pt->psvBubbleCount.currentValue()));
        makePSVButton("More bubbles", pt->psvBubbleCount);
    }

    if (checkUiUnlock(4u, pt->comboPurchased && pt->psvComboStartTime.nPurchases > 0))
    {
        uiImgsep(txrMenuSeparator3, "cats");

        uiSetUnlockLabelY(4u);
        std::sprintf(uiTooltipBuffer,
                     "Cats pop nearby bubbles or bombs. Their cooldown and range can be upgraded. Their "
                     "behavior can be permanently upgraded with prestige points.\n\nCats can be dragged around "
                     "to "
                     "position them strategically.\n\nNo, you can't get rid of a cat once purchased, you "
                     "monster.");
        std::sprintf(uiLabelBuffer, "%zu cats", nCatNormal);
        if (makePSVButton("Cat", pt->psvPerCatType[asIdx(CatType::Normal)]))
        {
            spawnCatCentered(CatType::Normal, getHueByCatType(CatType::Normal), /* placeInHand */ !onSteamDeck);

            if (nCatNormal == 0)
                doTip("Cats periodically pop bubbles for you!\nYou can drag them around to position them.");

            if (nCatNormal == 2)
                doTip(
                    "Multiple cats can be dragged at once by\nholding shift while clicking the mouse.\nRelease "
                    "either button to drop them!");
        }
    }

    const auto makeCooldownButton = [this](const char* label, const CatType catType, const char* additionalInfo = "")
    {
        auto& psv = pt->psvCooldownMultsPerCatType[asIdx(catType)];

        const float currentCooldown = CatConstants::baseCooldowns[asIdx(catType)] * psv.currentValue();
        const float nextCooldown    = CatConstants::baseCooldowns[asIdx(catType)] * psv.nextValue();

        if (!psv.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Decrease cooldown from %.2fs to %.2fs.%s\n\n(Note: can be reverted by right-clicking, "
                         "but no refunds!)",
                         static_cast<double>(currentCooldown / 1000.f),
                         static_cast<double>(nextCooldown / 1000.f),
                         additionalInfo);
        }
        else
        {
            std::sprintf(uiTooltipBuffer,
                         "Decrease cooldown (MAX).%s\n\n(Note: can be reverted by right-clicking, but "
                         "no refunds!)",
                         additionalInfo);
        }

        std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(pt->getComputedCooldownByCatType(catType) / 1000.f));

        makePSVButton(label, psv);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) && psv.nPurchases > 0u)
        {
            --psv.nPurchases;
            playSound(sounds.buy);
        }
    };

    const auto makeRangeButton = [this](const char* label, const CatType catType, const char* additionalInfo = "")
    {
        auto& psv = pt->psvRangeDivsPerCatType[asIdx(catType)];

        const float currentRange = CatConstants::baseRanges[asIdx(catType)] / psv.currentValue();
        const float nextRange    = CatConstants::baseRanges[asIdx(catType)] / psv.nextValue();

        if (!psv.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase range from %.2fpx to %.2fpx.%s\n\n(Note: can be reverted by right-clicking, but "
                         "no refunds!)",
                         static_cast<double>(currentRange),
                         static_cast<double>(nextRange),
                         additionalInfo);
        }
        else
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase range (MAX).%s\n\n(Note: can be reverted by right-clicking, but "
                         "no refunds!)",
                         additionalInfo);
        }

        std::sprintf(uiLabelBuffer, "%.2fpx", static_cast<double>(pt->getComputedRangeByCatType(catType)));
        makePSVButton(label, psv);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) && psv.nPurchases > 0u)
        {
            --psv.nPurchases;
            playSound(sounds.buy);
        }
    };

    const bool catUpgradesUnlocked = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 2 && nCatUni >= 1;
    if (checkUiUnlock(5u, catUpgradesUnlocked))
    {
        uiSetUnlockLabelY(5u);
        makeCooldownButton("  cooldown##Normal", CatType::Normal);
        makeRangeButton("  range##Normal", CatType::Normal);
    }

    // UNICAT
    const bool catUnicornUnlocked         = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 3;
    const bool catUnicornUpgradesUnlocked = catUnicornUnlocked && nCatUni >= 2 && nCatDevil >= 1;
    if (checkUiUnlock(6u, catUnicornUnlocked))
    {
        uiImgsep(txrMenuSeparator4, "unicats");

        uiSetUnlockLabelY(6u);
        std::sprintf(uiTooltipBuffer,
                     "Unicats transform bubbles into star bubbles, which are worth x15 more!\n\nHave "
                     "your cats pop them for you, or pop them towards the end of a combo for huge rewards!");
        std::sprintf(uiLabelBuffer, "%zu unicats", nCatUni);
        if (makePSVButton("Unicat", pt->psvPerCatType[asIdx(CatType::Uni)]))
        {
            spawnCatCentered(CatType::Uni, getHueByCatType(CatType::Uni), /* placeInHand */ !onSteamDeck);

            if (nCatUni == 0)
                doTip("Unicats transform bubbles in star bubbles,\nworth x15! Pop them at the end of a combo!");
        }

        if (checkUiUnlock(7u, catUnicornUpgradesUnlocked))
        {
            uiSetUnlockLabelY(7u);
            makeCooldownButton("  cooldown##Uni", CatType::Uni);

            if (pt->perm.unicatTranscendencePurchased && pt->perm.unicatTranscendenceAOEPurchased)
                makeRangeButton("  range##Uni", CatType::Uni);
        }
    }

    // DEVILCAT
    const bool catDevilUnlocked = pt->psvBubbleValue.nPurchases > 0 && nCatNormal >= 6 && nCatUni >= 4 &&
                                  pt->nShrinesCompleted >= 1;
    const bool catDevilUpgradesUnlocked = catDevilUnlocked && nCatDevil >= 2 && nCatAstro >= 1;
    if (checkUiUnlock(8u, catDevilUnlocked))
    {
        uiImgsep(txrMenuSeparator5, "devilcats");

        uiSetUnlockLabelY(8u);
        std::sprintf(uiTooltipBuffer,
                     "Devilcats transform bubbles into bombs that explode when popped. Bubbles affected by the "
                     "explosion are worth x10 more! Bomb explosion range can be upgraded.");
        std::sprintf(uiLabelBuffer, "%zu devilcats", nCatDevil);
        if (makePSVButton("Devilcat", pt->psvPerCatType[asIdx(CatType::Devil)]))
        {
            spawnCatCentered(CatType::Devil, getHueByCatType(CatType::Devil), /* placeInHand */ !onSteamDeck);

            if (nCatDevil == 0)
                doTip(
                    "Devilcats transform bubbles in bombs!\nExplode them to pop nearby "
                    "bubbles\nwith a x10 money multiplier!",
                    /* maxPrestigeLevel */ 1);
        }

        if (checkUiUnlock(9u, nCatDevil >= 1) && !isDevilcatHellsingedActive())
        {
            const float currentExplosionRadius = pt->psvExplosionRadiusMult.currentValue();
            const float nextExplosionRadius    = pt->psvExplosionRadiusMult.nextValue();

            uiSetUnlockLabelY(9u);
            std::sprintf(uiTooltipBuffer,
                         "Increase bomb explosion radius from x%.2f to x%.2f.",
                         static_cast<double>(currentExplosionRadius),
                         static_cast<double>(nextExplosionRadius));
            std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentExplosionRadius));
            makePSVButton("  Explosion radius", pt->psvExplosionRadiusMult);
        }

        if (checkUiUnlock(10u, catDevilUpgradesUnlocked))
        {
            uiSetUnlockLabelY(10u);
            makeCooldownButton("  cooldown##Devil", CatType::Devil);

            if (pt->perm.devilcatHellsingedPurchased)
                makeRangeButton("  range##Devil", CatType::Devil);
        }
    }

    // ASTROCAT
    const bool astroCatUnlocked = nCatNormal >= 10 && nCatUni >= 5 && nCatDevil >= 2 && pt->nShrinesCompleted >= 2;
    const bool astroCatUpgradesUnlocked = astroCatUnlocked && nCatDevil >= 9 && nCatAstro >= 5;
    if (checkUiUnlock(11u, astroCatUnlocked))
    {
        uiImgsep(txrMenuSeparator6, "astrocats");

        uiSetUnlockLabelY(11u);
        std::sprintf(uiTooltipBuffer,
                     "Astrocats periodically fly across the map, popping bubbles they hit with a huge x20 "
                     "money "
                     "multiplier!\n\nThey can be permanently upgraded with prestige points to inspire cats "
                     "watching them fly past to pop bubbles faster.");
        std::sprintf(uiLabelBuffer, "%zu astrocats", nCatAstro);
        if (makePSVButton("Astrocat", pt->psvPerCatType[asIdx(CatType::Astro)]))
        {
            spawnCatCentered(CatType::Astro, getHueByCatType(CatType::Astro), /* placeInHand */ !onSteamDeck);

            if (nCatAstro == 0)
                doTip(
                    "Astrocats periodically fly across\nthe entire map, with a huge\nx20 "
                    "money multiplier!",
                    /* maxPrestigeLevel */ 1);
        }

        if (checkUiUnlock(12u, astroCatUpgradesUnlocked))
        {
            uiSetUnlockLabelY(12u);
            makeCooldownButton("  cooldown##Astro", CatType::Astro);
            makeRangeButton("  range##Astro", CatType::Astro);
        }
    }

    // UNIQUE CAT BONUSES
    if (anyUniqueCat)
    {
        uiImgsep(txrMenuSeparator7, "unique cats");

        if (checkUiUnlock(13u, catWitch != nullptr))
        {
            uiSetUnlockLabelY(13u);
            makeCooldownButton("  witchcat cooldown",
                               CatType::Witch,
                               "\n\nEffectively increases the frequency of rituals.");

            if (checkUiUnlock(14u, pt->perm.witchCatBuffPowerScalesWithNCats))
            {
                uiSetUnlockLabelY(14u);
                makeRangeButton("  witchcat range",
                                CatType::Witch,
                                "\n\nAllows more cats to participate in group rituals, increasing the duration "
                                "of "
                                "buffs.");
            }
        }

        if (checkUiUnlock(15u, catWizard != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(15u);
            makeCooldownButton("  wizardcat cooldown",
                               CatType::Wizard,
                               "\n\nDoes *not* increase mana generation rate, but increases star bubble "
                               "absorption "
                               "rate and decreases cooldown between spell casts.");

            makeRangeButton("  wizardcat range",
                            CatType::Wizard,
                            "\n\nEffectively increases the area of effect of most spells, and star bubble "
                            "absorption range.");
        }

        if (checkUiUnlock(16u, catMouse != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(16u);
            makeCooldownButton("  mousecat cooldown##Mouse", CatType::Mouse);
            makeRangeButton("  mousecat range##Mouse", CatType::Mouse);
        }

        if (checkUiUnlock(17u, catEngi != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(17u);
            makeCooldownButton("  engicat cooldown",
                               CatType::Engi,
                               "\n\nEffectively increases the frequency of maintenances.");

            makeRangeButton("  engicat range",
                            CatType::Engi,
                            "\n\nAllows more cats to be boosted by maintenance at once.");
        }

        if (checkUiUnlock(18u, catRepulso != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(18u);
            // makeCooldownButton("  repulsocat cooldown##Repulso", CatType::Repulso);
            makeRangeButton("  repulsocat range##Repulso", CatType::Repulso);
        }

        if (checkUiUnlock(19u, catAttracto != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(19u);
            // makeCooldownButton("  attractocat cooldown##Attracto", CatType::Attracto);
            makeRangeButton("  attractocat range##Attracto", CatType::Attracto);
        }
    }

    const auto nextGoalsText = [&]() -> std::string
    {
        std::string result; // Use a single local variable for NRVO

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

        const auto needNCats = [&](const SizeT& count, const SizeT needed)
        {
            const char* name = "";

            // clang-format off
                if      (&count == &nCatNormal) name = "cat";
                else if (&count == &nCatUni)    name = "unicat";
                else if (&count == &nCatDevil)  name = "devilcat";
                else if (&count == &nCatAstro)  name = "astrocat";
            // clang-format on

            if (count < needed)
                result += "\n    - buy " + std::to_string(needed - count) + " more " + name + "(s)";
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
        }

        if (!catUpgradesUnlocked && catUnicornUnlocked)
        {
            startList("- to unlock cat upgrades:");

            if (pt->psvBubbleCount.nPurchases == 0)
                result += "\n    - buy more bubbles";

            needNCats(nCatNormal, 2);
            needNCats(nCatUni, 1);
        }

        // TODO P2: change dynamically
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
    }();

    ImGui::Columns(1);

    if (nextGoalsText != "")
    {
        uiImgsep(txrMenuSeparator8, "next goals");
        ImGui::Columns(1);

        uiSetFontScale(uiSubBulletFontScale);
        ImGui::Text("%s", nextGoalsText.c_str());
        uiSetFontScale(uiNormalFontScale);
    }

    ImGui::Columns(1);
}

////////////////////////////////////////////////////////////
bool Main::uiCheckbox(const char* label, bool* b)
{
    if (!ImGui::Checkbox(label, b))
        return false;

    playSound(sounds.btnswitch);
    return true;
}

////////////////////////////////////////////////////////////
bool Main::uiRadio(const char* label, int* i, const int value)
{
    if (!ImGui::RadioButton(label, i, value))
        return false;

    playSound(sounds.btnswitch);
    return true;
}

////////////////////////////////////////////////////////////
void Main::uiTabBarPrestige()
{
    ImGui::Spacing();
    ImGui::Spacing();

    uiImgsep(txrPrestigeSeparator0, "prestige", /* first */ true);
    ImGui::Columns(1);
    uiSetFontScale(uiNormalFontScale);

    if constexpr (isDemoVersion)
    {
        uiCenteredTextColored({229u, 63u, 63u, 255u}, "(!) Limited to 1 prestige in demo (!)");
    }

    std::sprintf(uiTooltipBuffer,
                 "WARNING: this will reset your progress!\n\nPrestige to increase bubble value permanently and "
                 "obtain prestige points. Prestige points can be used to unlock powerful permanent "
                 "upgrades.\n\nYou will sacrifice all your cats, bubbles, and money, but you will keep your "
                 "prestige points and permanent upgrades, and the value of bubbles will be permanently "
                 "increased.\n\nDo not be afraid to prestige -- it is what enables you to progress further!");
    std::sprintf(uiLabelBuffer, "current bubble value x%llu", pt->getComputedRewardByBubbleType(BubbleType::Normal));

    // Figure out how many times we can prestige in a row

    const auto currentPrestigeLevel    = pt->psvBubbleValue.nPurchases;
    const auto currentCompletedShrines = pt->nShrinesCompleted;
    const auto maxPrestigeLevel        = sf::base::SizeT{20u};

    sf::base::SizeT maxPurchaseablePrestigeLevel = currentPrestigeLevel;
    MoneyType       maxCost                      = 0u;

    for (sf::base::SizeT iPrestige = currentPrestigeLevel + 1u; iPrestige < maxPrestigeLevel; ++iPrestige)
    {
        const auto requiredMoney = static_cast<MoneyType>(
            pt->psvBubbleValue.cumulativeCostBetween(currentPrestigeLevel, iPrestige));

        const auto requiredCompletedShrines = Playthrough::getShrinesCompletedNeededForPrestigeLevel(iPrestige);

        if (pt->money >= requiredMoney && currentCompletedShrines >= requiredCompletedShrines)
        {
            maxPurchaseablePrestigeLevel = iPrestige;
            maxCost                      = requiredMoney;

            continue;
        }

        break;
    }

    const auto prestigeTimes = maxPurchaseablePrestigeLevel - currentPrestigeLevel;
    const auto ppReward      = pt->calculatePrestigePointReward(prestigeTimes);

    const auto printNextPrestigeRequirements = [&](const sf::base::SizeT level)
    {
        const auto nextCost = static_cast<MoneyType>(pt->psvBubbleValue.cumulativeCostBetween(currentPrestigeLevel, level));

        const auto nextRequiredShrines = Playthrough::getShrinesCompletedNeededForPrestigeLevel(level);

        ImGui::Text("    (level %zu -> %zu): $%s, %zu completed shrines",
                    currentPrestigeLevel + 1u,
                    level + 1u,
                    toStringWithSeparators(nextCost),
                    nextRequiredShrines);
    };

    uiSetFontScale(0.75f);

    const bool canPrestigePlus1 = maxPurchaseablePrestigeLevel + 1u < maxPrestigeLevel;
    const bool canPrestigePlus2 = maxPurchaseablePrestigeLevel + 2u < maxPrestigeLevel;

    if (canPrestigePlus1 || canPrestigePlus2)
    {
        ImGui::Text("  next prestige requirements:");

        if (canPrestigePlus1)
            printNextPrestigeRequirements(maxPurchaseablePrestigeLevel + 1u);

        if (canPrestigePlus2)
            printNextPrestigeRequirements(maxPurchaseablePrestigeLevel + 2u);

        ImGui::Spacing();
        ImGui::Spacing();
    }

    uiSetFontScale(uiNormalFontScale);

    uiBeginColumns();

    uiButtonHueMod = 120.f;

    ImGui::BeginDisabled(prestigeTimes == 0u);
    if (makePSVButtonEx("Prestige", pt->psvBubbleValue, prestigeTimes, maxCost))
    {
        beginPrestigeTransition(ppReward);
    }
    ImGui::EndDisabled();

    ImGui::Columns(1);

    uiButtonHueMod = 0.f;
    uiSetFontScale(0.75f);

    const auto currentMult = static_cast<SizeT>(pt->psvBubbleValue.currentValue()) + 1u;

    if (prestigeTimes > 0u)
    {
        ImGui::Text(
            "  prestige %zu time(s) at once\n  - increase bubble value from x%zu to x%zu\n  - obtain %llu "
            "prestige "
            "point(s)",
            prestigeTimes,
            currentMult,
            currentMult + prestigeTimes,
            ppReward);
    }
    else if (pt->psvBubbleValue.nPurchases == 19u)
    {
        ImGui::Text("  max prestige level reached!");

        uiSetFontScale(uiSubBulletFontScale);
        uiBeginColumns();

        std::sprintf(uiTooltipBuffer, "Spend money to immediately get prestige points.");
        uiLabelBuffer[0] = '\0';

        static int buyPPTimes = 1;

        char buf[256];
        std::snprintf(buf, sizeof(buf), "  Buy %d PPs", buyPPTimes * 100);

        bool done = false;
        if (makePurchasableButtonOneTime(buf, 1'000'000'000'000u * static_cast<unsigned int>(buyPPTimes), done))
            pt->prestigePoints += 100u * static_cast<unsigned int>(buyPPTimes);

        ImGui::NextColumn();

        uiSetFontScale(uiSubBulletFontScale);
        ImGui::InputInt("times##buypptimes", &buyPPTimes);
        buyPPTimes = sf::base::clamp(buyPPTimes, 1, 100);
        uiSetFontScale(uiNormalFontScale);

        ImGui::Columns(1);
    }
    else
    {
        if (maxCost == 0u)
            ImGui::Text("  not enough money to prestige");

        const SizeT shrinesNeeded = pt->getShrinesCompletedNeededForNextPrestige();

        if (pt->nShrinesCompleted < shrinesNeeded)
            ImGui::Text("  must complete %zu more shrine(s)", shrinesNeeded - pt->nShrinesCompleted);
    }

    uiSetFontScale(uiNormalFontScale);

    uiImgsep(txrPrestigeSeparator1, "permanent upgrades");
    ImGui::Columns(1);

    uiSetFontScale(uiNormalFontScale * 1.1f);
    char ppBuf[256];
    std::snprintf(ppBuf, sizeof(ppBuf), "%llu PPs available", pt->prestigePoints);
    uiCenteredText(ppBuf);
    uiSetFontScale(uiNormalFontScale);

    ImGui::BeginDisabled(undoPPPurchase.empty());
    {
        ImGui::Spacing();

        uiBeginColumns();

        bool done = false;

        uiLabelBuffer[0] = '\0';

        std::sprintf(uiTooltipBuffer, "Undo your last PP purchase, refunding you the prestige points.");

        if (makePurchasablePPButtonOneTime("  Undo last purchase", 0u, done))
        {
            SFML_BASE_ASSERT(!undoPPPurchase.empty());

            undoPPPurchase.back()();
            undoPPPurchase.popBack();

            done = false;
        }

        uiSetFontScale(uiToolTipFontScale);

        if (undoPPPurchase.empty())
            ImGui::Text("%s", "");
        else
            ImGui::Text("Undo time left: %.2fs", static_cast<double>(undoPPPurchaseTimer.value / 1000.f));

        uiSetFontScale(uiNormalFontScale);

        ImGui::Columns(1);
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    uiBeginColumns();

    uiButtonHueMod = 190.f;

    if (checkUiUnlock(47u, pt->psvBubbleValue.nPurchases >= 3u))
    {
        uiImgsep2(txrPrestigeSeparator4, "faster beginning");

        uiSetUnlockLabelY(47u);
        std::sprintf(uiTooltipBuffer, "Begin your next prestige with $1000.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Starter pack", 1u, pt->perm.starterPackPurchased);
    }

    if (checkUiUnlock(48u, pt->psvBubbleValue.nPurchases >= 1u))
    {
        uiImgsep2(txrPrestigeSeparator2, "clicking tools");

        uiSetUnlockLabelY(48u);
        std::sprintf(uiTooltipBuffer,
                     "Manually popping a bubble now also pops nearby bubbles automatically!\n\n(Note: combo "
                     "multiplier still only increases once per successful click.)\n\n(Note: this effect can be "
                     "toggled "
                     "at will.)");
        uiLabelBuffer[0] = '\0';
        if (makePurchasablePPButtonOneTime("Multipop click", 0u, pt->perm.multiPopPurchased))
            doTip("Popping a bubble now also pops\nnearby bubbles automatically!",
                  /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(49u, pt->perm.multiPopPurchased))
        {
            uiSetUnlockLabelY(49u);

            if (pt->psvBubbleValue.nPurchases >= 2u)
            {
                const float currentRange = pt->psvPPMultiPopRange.currentValue();
                const float nextRange    = pt->psvPPMultiPopRange.nextValue();

                if (!pt->psvPPMultiPopRange.isMaxedOut())
                {
                    std::sprintf(uiTooltipBuffer,
                                 "Increase the range of the multipop effect from %.2fpx to %.2fpx.",
                                 static_cast<double>(currentRange),
                                 static_cast<double>(nextRange));
                }
                else
                {
                    std::sprintf(uiTooltipBuffer, "Increase the range of the multipop effect (MAX).");
                }

                std::sprintf(uiLabelBuffer, "%.2fpx", static_cast<double>(currentRange));
                makePrestigePurchasablePPButtonPSV("  range", pt->psvPPMultiPopRange);
            }

            uiSetFontScale(uiSubBulletFontScale);
            uiCheckbox("enable ##multipop", &pt->multiPopEnabled);
            if (cachedMouseCat != nullptr)
            {
                ImGui::SameLine();
                uiCheckbox("mousecat##multipopmousecat", &pt->multiPopMouseCatEnabled);
            }
            uiSetFontScale(uiNormalFontScale);
            ImGui::NextColumn();
            ImGui::NextColumn();
        }
    }

    uiImgsep2(txrPrestigeSeparator3, "wind effects");

    std::sprintf(uiTooltipBuffer,
                 "A giant fan (off-screen) will produce an intense wind, making bubbles move and "
                 "flow much faster.\n\n(Note: this effect can be toggled at will.)");
    uiLabelBuffer[0] = '\0';
    if (makePurchasablePPButtonOneTime("Giant fan", 6u, pt->perm.windPurchased))
        doTip("Hold onto something!", /* maxPrestigeLevel */ UINT_MAX);

    if (checkUiUnlock(50u, pt->perm.windPurchased))
    {
        uiSetFontScale(uiSubBulletFontScale);
        ImGui::Columns(1);

        uiSetUnlockLabelY(50u);
        uiRadio("off##windOff", &pt->windStrength, 0);
        ImGui::SameLine();
        uiRadio("slow##windSlow", &pt->windStrength, 1);
        ImGui::SameLine();
        uiRadio("medium##windMed", &pt->windStrength, 2);
        ImGui::SameLine();
        uiRadio("fast##windFast", &pt->windStrength, 3);

        uiBeginColumns();
        uiSetFontScale(uiNormalFontScale);
        ImGui::NextColumn();
        ImGui::NextColumn();
    }

    uiImgsep2(txrPrestigeSeparator5, "cats");

    uiBeginColumns();

    std::sprintf(uiTooltipBuffer,
                 "Cats have graduated!\n\nThey still cannot resist their popping insticts, but they will go "
                 "for star bubbles and bombs first, ensuring they are not wasted!");
    uiLabelBuffer[0] = '\0';
    if (makePurchasablePPButtonOneTime("Smart cats", 1u, pt->perm.smartCatsPurchased))
        doTip("Cats will now prioritize popping\nspecial bubbles over basic ones!",
              /* maxPrestigeLevel */ UINT_MAX);

    if (checkUiUnlock(51u, pt->perm.smartCatsPurchased))
    {
        uiSetUnlockLabelY(51u);
        std::sprintf(uiTooltipBuffer,
                     "Embrace the glorious evolution!\n\nCats have ascended beyond their primal "
                     "insticts and will now prioritize bombs, then star bubbles, then normal "
                     "bubbles!\n\nThey will also ignore any bubble type of your choosing.\n\nThrough the sheer "
                     "power of their intellect, they also get a x2 multiplier on all bubble values.\n\n(Note: "
                     "this effect can be toggled at will.)");
        uiLabelBuffer[0] = '\0';
        if (makePurchasablePPButtonOneTime("genius cats", 8u, pt->perm.geniusCatsPurchased))
            doTip("Genius cats prioritize bombs and\ncan be instructed to ignore certain bubbles!",
                  /* maxPrestigeLevel */ UINT_MAX);
    }

    if (checkUiUnlock(52u, pt->perm.geniusCatsPurchased))
    {
        ImGui::Columns(1);
        uiSetFontScale(uiSubBulletFontScale);

        uiSetUnlockLabelY(52u);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("  ignore: ");
        ImGui::SameLine();

        auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt->geniusCatIgnoreBubbles;

        uiCheckbox("normal##genius", &ignoreNormal);
        ImGui::SameLine();

        uiCheckbox("star##genius", &ignoreStar);
        ImGui::SameLine();

        uiCheckbox("bombs##genius", &ignoreBomb);

        uiSetFontScale(uiNormalFontScale);
        uiBeginColumns();
    }

    if (checkUiUnlock(53u, pt->psvBubbleValue.nPurchases >= 3))
    {
        uiImgsep2(txrPrestigeSeparator6, "unicats");

        uiBeginColumns();

        uiSetUnlockLabelY(53u);
        std::sprintf(uiTooltipBuffer,
                     "Unicats transcend their physical form, becoming a higher entity that transforms bubbles "
                     "into "
                     "nova bubbles, worth x50.");
        uiLabelBuffer[0] = '\0';

        if (makePurchasablePPButtonOneTime("transcendence", 96u, pt->perm.unicatTranscendencePurchased))
            doTip("Are you ready for that sweet x50?", /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(54u, pt->perm.unicatTranscendencePurchased))
        {
            uiSetUnlockLabelY(54u);
            std::sprintf(uiTooltipBuffer,
                         "Unicats can now transform all bubbles in range at once. Also unlocks Unicat range "
                         "upgrades.");
            uiLabelBuffer[0] = '\0';

            if (makePurchasablePPButtonOneTime("nova expanse", 128u, pt->perm.unicatTranscendenceAOEPurchased))
                doTip("It's about to get crazy...", /* maxPrestigeLevel */ UINT_MAX);

            if (pt->perm.unicatTranscendencePurchased)
            {
                ImGui::Columns(1);
                uiCheckbox("enable transcendent unicats", &pt->perm.unicatTranscendenceEnabled);
                uiBeginColumns();
            }
        }
    }

    if (checkUiUnlock(55u, pt->psvBubbleValue.nPurchases >= 3))
    {
        uiImgsep2(txrPrestigeSeparator7, "devilcats");

        uiBeginColumns();

        uiSetUnlockLabelY(55u);
        std::sprintf(uiTooltipBuffer,
                     "Devilcats become touched by the flames of hell, opening stationary portals that teleport "
                     "bubbles into the abyss, with a x50 multiplier. Also unlocks Devilcat range upgrades.");
        uiLabelBuffer[0] = '\0';

        if (makePurchasablePPButtonOneTime("hellsinged", 192u, pt->perm.devilcatHellsingedPurchased))
            doTip("I'm starting to get a bit scared...", /* maxPrestigeLevel */ UINT_MAX);

        if (pt->perm.devilcatHellsingedPurchased)
        {
            ImGui::Columns(1);
            uiCheckbox("enable hellsinged devilcats", &pt->perm.devilcatHellsingedEnabled);
            uiBeginColumns();
        }
    }

    if (checkUiUnlock(56u, pt->getCatCountByType(CatType::Astro) >= 1u || pt->psvBubbleValue.nPurchases >= 3))
    {
        uiImgsep2(txrPrestigeSeparator8, "astrocats");

        uiBeginColumns();

        uiCheckbox("disable flight", &pt->disableAstrocatFlight);

        uiSetUnlockLabelY(56u);
        std::sprintf(uiTooltipBuffer,
                     "Astrocats are now equipped with fancy patriotic flags, inspiring cats watching "
                     "them fly by to work faster!");
        uiLabelBuffer[0] = '\0';

        if (makePurchasablePPButtonOneTime("Space propaganda", 16u, pt->perm.astroCatInspirePurchased))
            doTip("Astrocats will inspire other cats\nto work faster when flying by!",
                  /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(57u, pt->perm.astroCatInspirePurchased))
        {
            const float currentDuration = pt->getComputedInspirationDuration();
            const float nextDuration    = pt->getComputedNextInspirationDuration();

            uiSetUnlockLabelY(57u);
            if (!pt->psvPPInspireDurationMult.isMaxedOut())
            {
                std::sprintf(uiTooltipBuffer,
                             "Increase the duration of the inspiration effect from %.2fs to %.2fs.",
                             static_cast<double>(currentDuration / 1000.f),
                             static_cast<double>(nextDuration / 1000.f));
            }
            else
            {
                std::sprintf(uiTooltipBuffer, "Increase the duration of the inspiration effect (MAX).");
            }
            std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentDuration / 1000.f));

            makePrestigePurchasablePPButtonPSV("inspire duration", pt->psvPPInspireDurationMult);
        }
    }

    const auto makeUnsealButton = [&](const PrestigePointsType ppCost, const char* catName, const CatType type)
    {
        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(type)])
            return;

        std::sprintf(uiTooltipBuffer,
                     "Permanently release the %s from their shrine. They will be waiting for you right "
                     "outside when the shrine is activated.\n\n(Note: completing the shrine will now grant "
                     "1.5x the money it absorbed.)",
                     catName);
        uiLabelBuffer[0] = '\0';

        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s##%s", "Break the seal", catName);

        (void)makePurchasablePPButtonOneTime(buf, ppCost, pt->perm.unsealedByType[asIdx(type)]);
    };

    if (checkUiUnlock(58u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]))
    {
        uiImgsep2(txrPrestigeSeparator9, "witchcat");

        uiBeginColumns();

        uiSetUnlockLabelY(58u);
        makeUnsealButton(4u, "Witchcat", CatType::Witch);
        ImGui::Separator();

        const float currentDuration = pt->psvPPWitchCatBuffDuration.currentValue();
        const float nextDuration    = pt->psvPPWitchCatBuffDuration.nextValue();

        if (!pt->psvPPWitchCatBuffDuration.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the base duration of Witchcat buffs from %.2fs to %.2fs.",
                         static_cast<double>(currentDuration),
                         static_cast<double>(nextDuration));
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase the base duration of Witchcat buffs (MAX).");
        }
        std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentDuration));
        makePrestigePurchasablePPButtonPSV("Buff duration", pt->psvPPWitchCatBuffDuration);

        ImGui::Separator();

        std::sprintf(uiTooltipBuffer,
                     "The duration of Witchcat buffs scales with the number of cats in range of the ritual.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Group ritual", 4u, pt->perm.witchCatBuffPowerScalesWithNCats);

        std::sprintf(uiTooltipBuffer, "The duration of Witchcat buffs scales with the size of the explored map.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Worldwide cult", 4u, pt->perm.witchCatBuffPowerScalesWithMapSize);

        ImGui::Separator();

        std::sprintf(uiTooltipBuffer, "Half as many voodoo dolls will appear per ritual.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Material shortage", 8u, pt->perm.witchCatBuffFewerDolls);

        ImGui::Separator();

        std::sprintf(uiTooltipBuffer, "Dolls are automatically collected by Devilcat bomb explosions.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Flammable dolls", 8u, pt->perm.witchCatBuffFlammableDolls);

        std::sprintf(uiTooltipBuffer, "Dolls are automatically collected by Astrocats during their flyby.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Orbital dolls", 16u, pt->perm.witchCatBuffOrbitalDolls);

        ImGui::Separator();

        const float currentUniPercentage = pt->psvPPUniRitualBuffPercentage.currentValue();
        const float nextUniPercentage    = pt->psvPPUniRitualBuffPercentage.nextValue();

        if (!pt->psvPPUniRitualBuffPercentage.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the star bubble spawn chance during the Unicat vododoo ritual buff from %.2f%% "
                         "to %.2f%%.",
                         static_cast<double>(currentUniPercentage),
                         static_cast<double>(nextUniPercentage));
        }
        else
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the star bubble spawn chance during the Unicat vododoo ritual buff "
                         "(MAX).");
        }
        std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentUniPercentage));
        makePrestigePurchasablePPButtonPSV("Star Spawn %", pt->psvPPUniRitualBuffPercentage);

        const float currentDevilPercentage = pt->psvPPDevilRitualBuffPercentage.currentValue();
        const float nextDevilPercentage    = pt->psvPPDevilRitualBuffPercentage.nextValue();

        if (!pt->psvPPDevilRitualBuffPercentage.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the bomb spawn chance during the Devil vododoo ritual buff from %.2f%% to "
                         "%.2f%%.",
                         static_cast<double>(currentDevilPercentage),
                         static_cast<double>(nextDevilPercentage));
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase the bomb spawn chance during the Devil vododoo ritual buff (MAX).");
        }
        std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentDevilPercentage));
        makePrestigePurchasablePPButtonPSV("Bomb Spawn %", pt->psvPPDevilRitualBuffPercentage);
    }

    if (checkUiUnlock(59u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]))
    {
        uiImgsep2(txrPrestigeSeparator10, "wizardcat");

        uiBeginColumns();

        uiSetUnlockLabelY(59u);
        makeUnsealButton(8u, "Wizardcat", CatType::Wizard);
        ImGui::Separator();

        const float currentManaCooldown = pt->getComputedManaCooldown();
        const float nextManaCooldown    = pt->getComputedManaCooldownNext();

        if (!pt->psvPPManaCooldownMult.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Decrease mana generation cooldown from %.2fs to %.2fs.",
                         static_cast<double>(currentManaCooldown / 1000.f),
                         static_cast<double>(nextManaCooldown / 1000.f));
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Decrease mana generation cooldown (MAX).");
        }
        std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentManaCooldown / 1000.f));
        makePrestigePurchasablePPButtonPSV("Mana cooldown", pt->psvPPManaCooldownMult);

        const ManaType currentMaxMana = pt->getComputedMaxMana();
        const ManaType nextMaxMana    = pt->getComputedMaxManaNext();

        if (!pt->psvPPManaMaxMult.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer, "Increase the maximum mana from %llu to %llu.", currentMaxMana, nextMaxMana);
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase the maximum mana (MAX).");
        }
        std::sprintf(uiLabelBuffer, "%llu mana", currentMaxMana);
        makePrestigePurchasablePPButtonPSV("Mana limit", pt->psvPPManaMaxMult);

        ImGui::Separator();

        std::sprintf(uiTooltipBuffer,
                     "Allow the Wizardcat to automatically cast spells when enough mana is available. Can be "
                     "enabled and configured from the \"Magic\" tab.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Autocast", 4u, pt->perm.autocastPurchased);

        ImGui::Separator();

        std::sprintf(uiTooltipBuffer,
                     "Starpaw conversion ignores bombs, transforming only normal bubbles around the wizard "
                     "into star bubbles.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Selective starpaw", 4u, pt->perm.starpawConversionIgnoreBombs);

        if (pt->perm.unicatTranscendencePurchased && pt->perm.starpawConversionIgnoreBombs)
        {
            std::sprintf(uiTooltipBuffer, "Starpaw conversion now turns all normal bubbles into nova bubbles.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Nova starpaw", 64u, pt->perm.starpawNova);
        }

        ImGui::Separator();

        std::sprintf(uiTooltipBuffer, "The duration of Mewltiplier Aura is extended from 6s to 12s.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Meeeeeewltiplier", 64u, pt->perm.wizardCatDoubleMewltiplierDuration);

        ImGui::Separator();

        std::sprintf(uiTooltipBuffer, "The duration of Stasis Field is extended from 6s to 12s.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Pop Stuck In Time", 256u, pt->perm.wizardCatDoubleStasisFieldDuration);
    }

    if (checkUiUnlock(60u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]))
    {
        uiImgsep2(txrPrestigeSeparator11, "mousecat");

        uiBeginColumns();

        uiSetUnlockLabelY(60u);
        makeUnsealButton(8u, "Mousecat", CatType::Mouse);
        ImGui::Separator();

        const float currentReward = pt->psvPPMouseCatGlobalBonusMult.currentValue();
        const float nextReward    = pt->psvPPMouseCatGlobalBonusMult.nextValue();

        if (!pt->psvPPMouseCatGlobalBonusMult.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the global click reward value multiplier from x%.2f to x%.2f.",
                         static_cast<double>(currentReward),
                         static_cast<double>(nextReward));
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase the global click reward value multiplier (MAX).");
        }

        std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentReward));
        makePrestigePurchasablePPButtonPSV("Global click mult", pt->psvPPMouseCatGlobalBonusMult);
    }

    if (checkUiUnlock(61u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]))
    {
        uiImgsep2(txrPrestigeSeparator12, "engicat");

        uiBeginColumns();

        uiSetUnlockLabelY(61u);
        makeUnsealButton(16u, "Engicat", CatType::Engi);
        ImGui::Separator();

        const float currentReward = pt->psvPPEngiCatGlobalBonusMult.currentValue();
        const float nextReward    = pt->psvPPEngiCatGlobalBonusMult.nextValue();

        if (!pt->psvPPEngiCatGlobalBonusMult.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the global cat reward value multiplierfrom x%.2f to x%.2f.",
                         static_cast<double>(currentReward),
                         static_cast<double>(nextReward));
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase the global cat reward value multiplier (MAX).");
        }

        std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentReward));
        makePrestigePurchasablePPButtonPSV("Global cat mult", pt->psvPPEngiCatGlobalBonusMult);
    }

    if (checkUiUnlock(62u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]))
    {
        uiImgsep2(txrPrestigeSeparator13, "repulsocat");

        uiBeginColumns();

        uiSetUnlockLabelY(62u);
        makeUnsealButton(128u, "Repulsocat", CatType::Repulso);
        ImGui::Separator();

        std::sprintf(uiTooltipBuffer, "The Repulsocat cordially asks their fan to filter repelled bubbles by type.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Repulsion filter", 16u, pt->perm.repulsoCatFilterPurchased);

        if (checkUiUnlock(63u, pt->perm.repulsoCatFilterPurchased))
        {
            ImGui::Columns(1);
            uiSetFontScale(uiSubBulletFontScale);

            uiSetUnlockLabelY(63u);
            ImGui::Text("  ignore: ");
            ImGui::SameLine();

            auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt->repulsoCatIgnoreBubbles;

            uiCheckbox("normal##repulso", &ignoreNormal);
            ImGui::SameLine();

            uiCheckbox("star##repulso", &ignoreStar);
            ImGui::SameLine();

            uiCheckbox("bombs##repulso", &ignoreBomb);

            uiSetFontScale(uiNormalFontScale);
            uiBeginColumns();
        }

        std::sprintf(uiTooltipBuffer,
                     "The Repulsocat coats the fan blades with star powder, giving it a chance to convert "
                     "repelled bubbles to star bubbles.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Conversion field", 32u, pt->perm.repulsoCatConverterPurchased);

        if (checkUiUnlock(64u, pt->perm.repulsoCatConverterPurchased))
        {
            uiSetFontScale(uiSubBulletFontScale);
            uiSetUnlockLabelY(64u);
            uiCheckbox("enable ##repulsoconv", &pt->repulsoCatConverterEnabled);
            uiSetFontScale(uiNormalFontScale);
            ImGui::NextColumn();
            ImGui::NextColumn();

            const float currentChance = pt->psvPPRepulsoCatConverterChance.currentValue();
            const float nextChance    = pt->psvPPRepulsoCatConverterChance.nextValue();

            if (!pt->psvPPRepulsoCatConverterChance.isMaxedOut())
            {
                std::sprintf(uiTooltipBuffer,
                             "Increase the repelled bubble conversion chance from %.2f%% to %.2f%%.",
                             static_cast<double>(currentChance),
                             static_cast<double>(nextChance));
            }
            else
            {
                std::sprintf(uiTooltipBuffer, "Increase the repelled bubble conversion chance (MAX).");
            }
            std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentChance));
            makePrestigePurchasablePPButtonPSV("Conversion chance", pt->psvPPRepulsoCatConverterChance);

            std::sprintf(uiTooltipBuffer, "Bubbles are converted into nova bubbles instead of star bubbles.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Nova conversion", 96u, pt->perm.repulsoCatNovaConverterPurchased);
        }
    }

    if (checkUiUnlock(65u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]))
    {
        uiImgsep2(txrPrestigeSeparator14, "attractocat");

        uiBeginColumns();

        uiSetUnlockLabelY(65u);
        makeUnsealButton(256u, "Attractocat", CatType::Attracto);
        ImGui::Separator();

        std::sprintf(uiTooltipBuffer,
                     "The Attractocat does some quantum science stuff to its magnet to allow filtering of "
                     "attracted bubbles by type.");
        uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Attraction filter", 96u, pt->perm.attractoCatFilterPurchased);

        if (checkUiUnlock(66u, pt->perm.attractoCatFilterPurchased))
        {
            ImGui::Columns(1);
            uiSetFontScale(uiSubBulletFontScale);

            uiSetUnlockLabelY(66u);
            ImGui::Text("  ignore: ");
            ImGui::SameLine();

            auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt->attractoCatIgnoreBubbles;

            uiCheckbox("normal##attracto", &ignoreNormal);
            ImGui::SameLine();

            uiCheckbox("star##attracto", &ignoreStar);
            ImGui::SameLine();

            uiCheckbox("bombs##attracto", &ignoreBomb);

            uiSetFontScale(uiNormalFontScale);
            uiBeginColumns();
        }
    }

    if (checkUiUnlock(67u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)]))
    {
        uiImgsep2(txrPrestigeSeparator15, "copycat");

        uiBeginColumns();

        uiSetUnlockLabelY(67u);
        makeUnsealButton(512u, "Copycat", CatType::Copy);
        ImGui::Separator();

        // TODO P1: something?
    }

    uiButtonHueMod = 0.f;

    ImGui::Columns(1);
}

////////////////////////////////////////////////////////////
void Main::uiTabBarMagic()
{
    uiSetFontScale(uiNormalFontScale);

    Cat* wizardCat = cachedWizardCat;

    if (wizardCat == nullptr)
    {
        ImGui::Text("The wizardcat is missing!");
        return;
    }

    Cat* copyCat = cachedCopyCat;

    ImGui::Spacing();
    ImGui::Spacing();

    uiImgsep(txrMagicSeparator0, "wisdom", /* first */ true);
    ImGui::Columns(1);

    ImGui::Text("Wisdom points: %s WP", toStringWithSeparators(pt->wisdom));

    uiCheckbox("Absorb wisdom from star bubbles", &pt->absorbingWisdom);
    std::sprintf(uiTooltipBuffer,
                 "The Wizardcat concentrates, absorbing wisdom points from nearby star bubbles. While the "
                 "Wizardcat is concentrating, it cannot cast spells nor be moved around.");
    uiMakeTooltip();

    uiBeginColumns();
    uiButtonHueMod = 45.f;

    std::sprintf(uiTooltipBuffer,
                 "The Wizardcat taps into memories of past lives, remembering a powerful spell.\n\nMana "
                 "costs:\n- "
                 "1st spell: 5 mana\n- 2nd spell: 20 mana\n- 3rd spell: 30 mana\n- 4th spell: 40 mana\n\nNote: "
                 "You "
                 "won't be able to cast a spell if the cost exceeds your maximum mana!");
    std::sprintf(uiLabelBuffer, "%zu/%zu", pt->psvSpellCount.nPurchases, pt->psvSpellCount.data->nMaxPurchases);
    (void)makePSVButtonExByCurrency("Remember spell",
                                    pt->psvSpellCount,
                                    1u,
                                    static_cast<MoneyType>(pt->psvSpellCount.nextCost()),
                                    pt->wisdom,
                                    "%s WP##%u");

    uiButtonHueMod = 0.f;

    uiImgsep(txrMagicSeparator1, "mana");
    ImGui::Columns(1);

    ImGui::Text("Mana: %llu / %llu", pt->mana, pt->getComputedMaxMana());

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Next mana:");

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(157, 0, 255, 128));
    ImGui::ProgressBar(pt->manaTimer / pt->getComputedManaCooldown());
    ImGui::PopStyleColor();

    ImGui::Text("Wizard cooldown: %.2fs", static_cast<double>(wizardCat->cooldown.value / 1000.f));

    uiImgsep(txrMagicSeparator2, "spells");

    if (pt->psvSpellCount.nPurchases == 0)
        ImGui::Text("No spells revealed yet...");

    ImGui::Columns(1);
    uiSetFontScale(0.8f);

    if (pt->psvSpellCount.nPurchases > 0)
    {
        if (pt->absorbingWisdom)
            uiCenteredText("Cannot cast spells while absorbing wisdom...");
        else if (wizardCat->isHexedOrCopyHexed())
            uiCenteredText("Cannot cast spells while hexed...");
        else if (wizardCat->cooldown.value > 0.f)
            uiCenteredText("Cannot cast spells while on cooldown...");
        else if (isCatBeingDragged(*wizardCat))
            uiCenteredText("Cannot cast spells while being dragged...");
        else
        {
            const bool anySpellCastable = pt->mana >= spellManaCostByIndex[0] && pt->psvSpellCount.nPurchases >= 1;

            if (anySpellCastable)
                uiCenteredText("Ready to cast a spell!");
            else
                uiCenteredText("Not enough mana to cast any spell...");
        }
    }

    ImGui::Separator();
    uiSetFontScale(uiNormalFontScale);

    uiBeginColumns();
    uiButtonHueMod = 45.f;

    //
    // SPELL 0
    if (checkUiUnlock(32u, pt->psvSpellCount.nPurchases >= 1))
    {
        uiSetUnlockLabelY(32u);
        std::sprintf(uiTooltipBuffer,
                     "Transforms a percentage of bubbles around the Wizardcat into star bubbles "
                     "immediately.\n\nCan be upgraded to ignore bombs with prestige points.");
        uiLabelBuffer[0] = '\0';
        bool done        = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Starpaw Conversion",
                                                   done,
                                                   spellManaCostByIndex[0],
                                                   pt->mana,
                                                   "%s mana##%u"))
        {
            castSpellByIndex(0u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();

        const float currentPercentage = pt->psvStarpawPercentage.currentValue();
        const float nextPercentage    = pt->psvStarpawPercentage.nextValue();

        if (!pt->psvStarpawPercentage.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the percentage of bubbles converted into star bubbles from %.2f%% to %.2f%%.",
                         static_cast<double>(currentPercentage),
                         static_cast<double>(nextPercentage));
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase the percentage of bubbles converted into star bubbles (MAX).");
        }
        std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentPercentage));
        (void)makePSVButtonExByCurrency("  higher percentage##starpawperc",
                                        pt->psvStarpawPercentage,
                                        1u,
                                        static_cast<MoneyType>(pt->psvStarpawPercentage.nextCost()),
                                        pt->wisdom,
                                        "%s WP##%u");
    }

    //
    // SPELL 1
    if (checkUiUnlock(33u, pt->psvSpellCount.nPurchases >= 2))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(33u);
        std::sprintf(uiTooltipBuffer,
                     "Creates a value multiplier aura around the Wizardcat that affects all cats and bubbles. "
                     "Lasts %d seconds.\n\nCasting this spell multiple times will accumulate the aura "
                     "duration.",
                     pt->perm.wizardCatDoubleMewltiplierDuration ? 12 : 6);
        std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(pt->mewltiplierAuraTimer / 1000.f));
        bool done = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Mewltiplier Aura",
                                                   done,
                                                   spellManaCostByIndex[1],
                                                   pt->mana,
                                                   "%s mana##%u"))
        {
            castSpellByIndex(1u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();

        const float currentMultiplier = pt->psvMewltiplierMult.currentValue();
        const float nextMultiplier    = pt->psvMewltiplierMult.nextValue();

        std::sprintf(uiTooltipBuffer,
                     "Increase the multiplier applied while the aura is active from x%.2f to x%.2f.",
                     static_cast<double>(currentMultiplier),
                     static_cast<double>(nextMultiplier));
        std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentMultiplier));
        (void)makePSVButtonExByCurrency("  higher multiplier",
                                        pt->psvMewltiplierMult,
                                        1u,
                                        static_cast<MoneyType>(pt->psvMewltiplierMult.nextCost()),
                                        pt->wisdom,
                                        "%s WP##%u");
    }

    //
    // SPELL 2
    if (checkUiUnlock(34u, pt->psvSpellCount.nPurchases >= 3))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(34u);
        std::sprintf(uiTooltipBuffer,
                     "The Wizardcat uses their magic to empower a nearby Witchcat, reducing their remaining "
                     "ritual cooldown.\n\nNote: This spell has no effect if there is no Witchcat "
                     "nearby, or if there are voodoo dolls left to collect.");
        uiLabelBuffer[0] = '\0';

        bool done = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Dark Union", done, spellManaCostByIndex[2], pt->mana, "%s mana##%u"))
        {
            castSpellByIndex(2u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();

        const float currentPercentage = pt->psvDarkUnionPercentage.currentValue();
        const float nextPercentage    = pt->psvDarkUnionPercentage.nextValue();

        if (!pt->psvDarkUnionPercentage.isMaxedOut())
        {
            std::sprintf(uiTooltipBuffer,
                         "Increase the cooldown reduction percentage from %.2f%% to %.2f%%.",
                         static_cast<double>(currentPercentage),
                         static_cast<double>(nextPercentage));
        }
        else
        {
            std::sprintf(uiTooltipBuffer, "Increase the cooldown reduction percentage (MAX).");
        }
        std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentPercentage));
        (void)makePSVButtonExByCurrency("  higher reduction##darkunionperc",
                                        pt->psvDarkUnionPercentage,
                                        1u,
                                        static_cast<MoneyType>(pt->psvDarkUnionPercentage.nextCost()),
                                        pt->wisdom,
                                        "%s WP##%u");
    }

    //
    // SPELL 3
    if (checkUiUnlock(35u, pt->psvSpellCount.nPurchases >= 4))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(35u);
        std::sprintf(uiTooltipBuffer,
                     "The Wizardcat controls time itself, creating a stasis field for %d seconds. All bubbles "
                     "caught in the field become frozen in time, unable to move or be destroyed. However, they "
                     "can "
                     "still be popped, as many times as you want!\n\nCasting this spell multiple times will "
                     "accumulate the field duration.\n\nNote: This spell has no effect if there are no bubbles "
                     "nearby. Bombs are also affected by the stasis field.",
                     pt->perm.wizardCatDoubleStasisFieldDuration ? 12 : 6);
        std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(pt->stasisFieldTimer / 1000.f));

        bool done = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Stasis Field",
                                                   done,
                                                   spellManaCostByIndex[3],
                                                   pt->mana,
                                                   "%s mana##%u"))
        {
            castSpellByIndex(3u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();
    }

    uiButtonHueMod = 0.f;
    ImGui::Columns(1);

    if (pt->psvSpellCount.nPurchases > 0 && pt->perm.autocastPurchased)
    {
        uiImgsep(txrMagicSeparator3, "autocast");

        ImGui::Columns(1);
        uiButtonHueMod = 45.f;

        constexpr const char* entries[]{
            "None",
            "Starpaw Conversion",
            "Mewltiplier Aura",
            "Dark Union",
            "Stasis Field",
        };

        if (ImGui::BeginCombo("Spell##autocastspell", entries[pt->perm.autocastIndex]))
        {
            for (SizeT i = 0u; i < pt->psvSpellCount.nPurchases + 1; ++i)
            {
                const bool isSelected = pt->perm.autocastIndex == i;
                if (ImGui::Selectable(entries[i], isSelected))
                {
                    pt->perm.autocastIndex = i;
                    playSound(sounds.uitab);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        uiButtonHueMod = 0.f;
    }
}

//////////////////////////////////////////////////////////////
void Main::uiTabBarStats()
{
    const auto displayStats = [&](const Stats& stats)
    {
        ImGui::Spacing();
        ImGui::Spacing();

        const auto [h, m, s] = formatTime(stats.secondsPlayed);
        ImGui::Text("Time played: %lluh %llum %llus", h, m, s);

        ImGui::Spacing();
        ImGui::Spacing();

        const auto bubblesPopped            = stats.getTotalNBubblesPopped();
        const auto bubblesHandPopped        = stats.getTotalNBubblesHandPopped();
        const auto bubblesPoppedRevenue     = stats.getTotalRevenue();
        const auto bubblesHandPoppedRevenue = stats.getTotalRevenueHand();

        ImGui::Text("Bubbles popped: %s", toStringWithSeparators(bubblesPopped));
        ImGui::Indent();
        ImGui::Text("Clicked: %s", toStringWithSeparators(bubblesHandPopped));
        ImGui::Text("By cats: %s", toStringWithSeparators(bubblesPopped - bubblesHandPopped));
        ImGui::Unindent();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Highest $/s: %s", toStringWithSeparators(stats.highestDPS));

        ImGui::Text("Revenue: $%s", toStringWithSeparators(bubblesPoppedRevenue));
        ImGui::Indent();
        ImGui::Text("Clicked: $%s", toStringWithSeparators(bubblesHandPoppedRevenue));
        ImGui::Text("By cats: $%s", toStringWithSeparators(bubblesPoppedRevenue - bubblesHandPoppedRevenue));
        ImGui::Indent();
        ImGui::Text("Bombs:  $%s", toStringWithSeparators(stats.explosionRevenue));
        ImGui::Text("Flights: $%s", toStringWithSeparators(stats.flightRevenue));
        ImGui::Text("Portals: $%s", toStringWithSeparators(stats.hellPortalRevenue));
        ImGui::Unindent();
        ImGui::Unindent();
    };

    if (ImGui::BeginTabBar("TabBarStats", ImGuiTabBarFlags_DrawSelectedOverline))
    {
        static int lastSelectedTabIdx = 0;

        const auto selectedTab = [&](int idx)
        {
            if (lastSelectedTabIdx != idx)
                playSound(sounds.uitab);

            lastSelectedTabIdx = idx;
        };

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Tips "))
        {
            selectedTab(0);

            ImGui::BeginChild("TipsScroll", ImVec2(ImGui::GetContentRegionAvail().x, uiGetMaxWindowHeight() - 125.f));

            const auto addTip = [&](const char* title, const char* description)
            {
                uiSetFontScale(uiNormalFontScale * 2.0f);
                uiCenteredText(title);

                ImGui::PushFont(fontImGuiMouldyCheese);
                uiSetFontScale(uiNormalFontScale);
                ImGui::TextWrapped("%s", description);
                ImGui::PopFont();

                ImGui::Separator();
            };

            addTip("Getting Started",
                   "Click on bubbles to pop them and earn money.\n\nPurchase upgrades and cats to increase "
                   "your "
                   "income and automate your bubble popping journey.");

            if (pt->comboPurchased)
                addTip("Combos",
                       "Popping bubbles in quick succession will increase your combo multiplier, boosting your "
                       "revenue. Keep the combo going for maximum profit!\n\nPopping high-value bubbles such "
                       "as "
                       "star bubbles while your combo multiplier is high will yield even more revenue.");

            if (pt->getCatCountByType(CatType::Normal) > 0)
            {
                addTip("Regular Cats",
                       "Regular cats will automatically pop bubbles for you, even while you are away or the "
                       "game "
                       "is in the background.\n\nThey are the bread and butter of any cat formation!");

                addTip("Cat Dragging",
                       "You can drag cats around the screen to reposition them.\n\nMoving individual cats can "
                       "be "
                       "done by clicking and dragging them.\n\nMultiple cats can be moved at once by holding "
                       "down "
                       "left shift and dragging a selection box around them. After that, either release left "
                       "shift "
                       "or the mouse button and drag them to their intended position. This is a great way to "
                       "move "
                       "an entire formation of cats at once.");
            }

            if (pt->getCatCountByType(CatType::Uni) > 0)
                addTip("Unicats",
                       "Unicats will convert normal bubbles into star bubbles, which are worth x15 the value "
                       "of "
                       "normal bubbles.\n\nPopping star bubbles manually while your combo multiplier is high "
                       "(towards the end of a combo) is a great way of making money early.\n\nAlternatively, "
                       "you "
                       "can place regular cats under unicats to have them pop the star bubbles for you.");

            if (pt->mapPurchased)
            {
                addTip("Map Exploration",
                       "Expand the map to discover shrines containing powerful unique cats and to have more "
                       "real estate for your cat army.\n\nYou can scroll the map with the scroll wheel, "
                       "holding "
                       "right click, by dragging with two fingers, by using the A/D/Left/Right keys.\n\nYou "
                       "can "
                       "jump around the map by clicking on the minimap or using the PgUp/PgDn/Home/End keys.");

                addTip("Shrines",
                       "Shrines contain powerful cats with unique powers and synergies.\n\nIn order to unseal "
                       "the "
                       "cats, the shrine must first be activated by purchasing \"Activate next shrine\" in the "
                       "shop.\n\nAfterwards, the shrine must be completed by popping bubbles in its range "
                       "until "
                       "the required amount of revenue is collected.");
            }

            if (pt->psvBubbleValue.nPurchases > 0 ||
                (pt->getCatCountByType(CatType::Uni) >= 3 && pt->nShrinesCompleted > 0))
                addTip("Prestige",
                       "Prestige to reset your current progress, permanently increasing the value of bubbles "
                       "and "
                       "unlocking powerful permanent upgrades that persist between prestiges.\n\nDo not be "
                       "afraid "
                       "of prestiging, as its benefits will allow you to return to your current state very "
                       "quickly "
                       "and progress much further than it was possible before!");

            if (pt->getCatCountByType(CatType::Devil) > 0)
                addTip("Devilcats",
                       "Devilcats will convert normal bubbles into bombs, which explode when popped. Bubbles "
                       "caught in the explosion are worth x10 their original value. This means that every star "
                       "bubble caught in the explosion will be worth x150 the value of a normal "
                       "bubble.\n\nPosition regular cats beneath Devilcats to automatically pop bombs, and "
                       "Unicats "
                       "nearby to maximize the chance of having star bubbles caught in the explosion.");

            if (pt->getCatCountByType(CatType::Astro) > 0)
                addTip("Astrocats",
                       "Astrocats will periodically fly across the map, looping around when they reach the "
                       "edge of "
                       "the screen.\n\nAny bubble they touch while flying will be popped with a x20 "
                       "multiplier.\n\nUpgrading the total bubble count and expanding the map will indirectly "
                       "increase the effectiveness of Astrocats.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
                addTip("Witchcat",
                       "The Witchcat periodically perform voodoo rituals.\n\nDuring a ritual, a random cat in "
                       "range of the Witchcat will be hexed and will become inactive until the ritual "
                       "ends.\n\nAt "
                       "the same time, voodoo dolls will appear throughout the map -- collect all of them to "
                       "end "
                       "the ritual and gain a powerful timed buff depending on the type of cat that was "
                       "hexed.");


            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
                addTip("Wizardcat",
                       "The Wizardcat casts powerful spells using mana that regenerates over time.\n\nIn order "
                       "to "
                       "learn new spells, the Wizardcat must concentrate and absorb wisdom from star bubbles, "
                       "earning \"wisdom points\".\n\nCasting spells or changing the Wizardcat's state can be "
                       "done "
                       "in the \"Magic\" menu. ");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
                addTip("Mousecat",
                       "The Mousecat pops nearby bubbles keeping up its own personal combo.\n\nCombo/click "
                       "upgrades you purchased also apply to the Mousecat.\n\nRegular cats in range of the "
                       "Mousecat will gain the same combo multiplier as the Mousecat.\n\nFurthermore, the "
                       "Mousecat "
                       "provides a global click revenue value buff.");


            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
                addTip("Engicat",
                       "The Engicat periodically increases the speed of nearby cats, effectively decreasing "
                       "their "
                       "cooldown.\n\nFurthermore, the Mousecat provides a global cat revenue value buff.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)])
                addTip("Repulsocat",
                       "The Repulsocat blows nearby bubbles away.\n\nRecently blown bubbles are worth x2 their "
                       "value.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)])
                addTip("Attractocat",
                       "The Attractocat attracts nearby bubbles.\n\nBubbles in range of the Attractocat are "
                       "worth "
                       "x2 their value.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)])
                addTip("Copycat",
                       "The Copycat can mimic the abilities, effects, and properties of any other unique "
                       "cat.\n\nThe mimicked cat can be chosen through the disguise menu near the bottom of "
                       "the "
                       "screen.");

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Statistics "))
        {
            selectedTab(1);

            uiSetFontScale(1.f);
            uiCenteredText(" ~~ Lifetime ~~ ");

            uiSetFontScale(0.75f);
            displayStats(profile.statsLifetime);

            ImGui::Separator();

            uiSetFontScale(1.f);
            uiCenteredText(" ~~ This playthrough ~~ ");

            uiSetFontScale(0.75f);
            displayStats(pt->statsTotal);

            ImGui::Separator();

            uiSetFontScale(1.f);
            uiCenteredText(" ~~ This prestige ~~ ");

            uiSetFontScale(0.75f);
            displayStats(pt->statsSession);

            ImGui::Spacing();
            ImGui::Spacing();

            // per cat type
            MoneyType revenueByCatType[nCatTypes] = {};

            for (const Cat& cat : pt->cats)
                revenueByCatType[asIdx(cat.type)] += cat.moneyEarned;

            ImGui::Text("Cat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Normal)]));
            ImGui::Text("Devilcat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Devil)]));

            ImGui::Text("Astrocat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Astro)]));
            ImGui::Text("Mousecat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Mouse)]));

            uiSetFontScale(uiNormalFontScale);

            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Milestones "))
        {
            selectedTab(2);

            uiSetFontScale(0.75f);

            const auto doMilestone = [&](const char* name, const MilestoneTimestamp value)
            {
                if (value == maxMilestone)
                {
                    ImGui::Text("%s: N/A", name);
                    return;
                }

                const auto [h, m, s] = formatTime(value);
                ImGui::Text("%s: %lluh %llum %llus", name, h, m, s);
            };

            doMilestone("1st Cat", pt->milestones.firstCat);
            doMilestone("5th Cat", pt->milestones.fiveCats);
            doMilestone("10th Cat", pt->milestones.tenCats);

            ImGui::Separator();

            doMilestone("1st Unicat", pt->milestones.firstUnicat);
            doMilestone("5th Unicat", pt->milestones.fiveUnicats);
            doMilestone("10th Unicat", pt->milestones.tenUnicats);

            ImGui::Separator();

            doMilestone("1st Devilcat", pt->milestones.firstDevilcat);
            doMilestone("5th Devilcat", pt->milestones.fiveDevilcats);
            doMilestone("10th Devilcat", pt->milestones.tenDevilcats);

            ImGui::Separator();

            doMilestone("1st Astrocat", pt->milestones.firstAstrocat);
            doMilestone("5th Astrocat", pt->milestones.fiveAstrocats);
            doMilestone("10th Astrocat", pt->milestones.tenAstrocats);

            ImGui::Separator();

            doMilestone("Prestige Level 2", pt->milestones.prestigeLevel2);
            doMilestone("Prestige Level 3", pt->milestones.prestigeLevel3);
            doMilestone("Prestige Level 4", pt->milestones.prestigeLevel4);
            doMilestone("Prestige Level 5", pt->milestones.prestigeLevel5);
            doMilestone("Prestige Level 6", pt->milestones.prestigeLevel6);
            doMilestone("Prestige Level 10", pt->milestones.prestigeLevel10);
            doMilestone("Prestige Level 15", pt->milestones.prestigeLevel15);
            doMilestone("Prestige Level 20", pt->milestones.prestigeLevel20);

            ImGui::Separator();

            doMilestone("$10.000 Revenue", pt->milestones.revenue10000);
            doMilestone("$100.000 Revenue", pt->milestones.revenue100000);
            doMilestone("$1.000.000 Revenue", pt->milestones.revenue1000000);
            doMilestone("$10.000.000 Revenue", pt->milestones.revenue10000000);
            doMilestone("$100.000.000 Revenue", pt->milestones.revenue100000000);
            doMilestone("$1.000.000.000 Revenue", pt->milestones.revenue1000000000);

            ImGui::Separator();

            for (SizeT i = 0u; i < nShrineTypes; ++i)
                doMilestone(shrineNames[i], pt->milestones.shrineCompletions[i]);

            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Achievements "))
        {
            selectedTab(3);

            const sf::base::SizeT nAchievementsUnlocked = sf::base::countTruthy(profile.unlockedAchievements,
                                                                                profile.unlockedAchievements + nAchievements);

            uiSetFontScale(uiNormalFontScale);
            ImGui::Text("%zu / %zu achievements unlocked", nAchievementsUnlocked, sf::base::getArraySize(achievementData));

            static bool showCompleted = true;
            uiSetFontScale(0.75f);
            uiCheckbox("Show completed", &showCompleted);

            ImGui::Separator();
            uiSetFontScale(0.75f);

            ImGui::BeginChild("AchScroll", ImVec2(ImGui::GetContentRegionAvail().x, uiGetMaxWindowHeight() - 125.f));

            sf::base::U64 id = 0u;
            for (const auto& [name, description, secret] : achievementData)
            {
                const bool unlocked = profile.unlockedAchievements[id];

                if (!showCompleted && unlocked)
                {
                    ++id;
                    continue;
                }

                const float opacity = unlocked ? 1.f : 0.5f;

                const ImVec4 textColor{1.f, 1.f, 1.f, opacity};

                uiSetFontScale(uiNormalFontScale * 1.15f);
                ImGui::TextColored(textColor, "%llu - %s", id, (!secret || unlocked) ? name : "???");

                ImGui::PushFont(fontImGuiMouldyCheese);
                uiSetFontScale(0.75f);
                ImGui::TextColored(textColor, "%s", (!secret || unlocked) ? description : "(...secret achievement...)");

                if (!unlocked && achievementProgress[id].hasValue())
                    ImGui::TextColored(textColor,
                                       "(%s / %s)",
                                       toStringWithSeparators<0>(achievementProgress[id]->value),
                                       toStringWithSeparators<1>(achievementProgress[id]->threshold));

                ImGui::PopFont();

                ImGui::Separator();

                ++id;
            }

            if (debugMode)
            {
                uiButtonHueMod = 120.f;
                uiPushButtonColors();

                uiSetFontScale(uiNormalFontScale);
                if (ImGui::Button("Reset stats and achievements"))
                {
                    withAllStats([](Stats& stats) { stats = {}; });

                    for (bool& b : profile.unlockedAchievements)
                        b = false;
                }

                uiPopButtonColors();
                uiButtonHueMod = 0.f;
            }

            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        uiSetFontScale(uiNormalFontScale);
        ImGui::EndTabBar();
    }

    uiSetFontScale(uiNormalFontScale);
}

////////////////////////////////////////////////////////////
void Main::uiTabBarSettings()
{
    bool sgActive = false;
    SFML_BASE_SCOPE_GUARD({
        if (sgActive)
            ImGui::EndTabBar();
    });
    sgActive = ImGui::BeginTabBar("TabBarSettings", ImGuiTabBarFlags_DrawSelectedOverline);

    static int lastSelectedTabIdx = 0;

    const auto selectedTab = [&](int idx)
    {
        if (lastSelectedTabIdx != idx)
            playSound(sounds.uitab);

        lastSelectedTabIdx = idx;
    };

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(" Audio "))
    {
        selectedTab(0);

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

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(" Interface "))
    {
        selectedTab(1);

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

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(" Graphics "))
    {
        selectedTab(2);

        uiSetFontScale(uiNormalFontScale);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Background Opacity", &profile.backgroundOpacity, 0.f, 100.f, "%.f%%");

        uiCheckbox("Always show drawings", &profile.alwaysShowDrawings);

        ImGui::Separator();

        uiCheckbox("Show cat range", &profile.showCatRange);
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

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Vibrance", &profile.ppSVibrance, 0.f, 2.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Saturation", &profile.ppSSaturation, 0.f, 2.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Lightness", &profile.ppSLightness, 0.5f, 1.5f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Sharpness", &profile.ppSSharpness, 0.f, 1.f, "%.2f");

        ImGui::Separator();

        // TODO P1: check if this solves flickering
        ImGui::Text("Flickering troubleshooting");

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

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(" Display "))
    {
        selectedTab(3);

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
        if (ImGui::DragFloat("FPS Limit", &fpsLimit, 1.f, 60.f, 144.f, "%.f", ImGuiSliderFlags_AlwaysClamp))
        {
            profile.frametimeLimit = static_cast<unsigned int>(fpsLimit);
            window.setFramerateLimit(profile.frametimeLimit);
        }

        uiSetFontScale(uiNormalFontScale);

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(" Data "))
    {
        selectedTab(4);

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

        uiButtonHueMod = 120.f;
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
        uiButtonHueMod = 0.f;

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

                rng.reseed(pt->seed);
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

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (isDebugModeEnabled() && ImGui::BeginTabItem(" Debug "))
    {
        selectedTab(5);

        if (ImGui::Button("Slide"))
        {
            fixedBgSlideTarget += 1.f;

            if (fixedBgSlideTarget >= 3.f)
                fixedBgSlideTarget = 0.f;
        }

        ImGui::Separator();

        constexpr sf::base::I64 iStep            = 1;
        static sf::base::I64    speedrunTimerSet = 0;

        ImGui::SetNextItemWidth(240.f * profile.uiScale);
        if (ImGui::InputScalar("Speedrun timer", ImGuiDataType_S64, &speedrunTimerSet, &iStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
            pt->speedrunStartTime.emplace(speedrunTimerSet);

        ImGui::Separator();

        if (ImGui::Button("Save game"))
        {
            ptMain.fullVersion = !isDemoVersion;
            savePlaythroughToFile(ptMain, "userdata/playthrough.json");
        }

        ImGui::SameLine();

        if (ImGui::Button("Load game"))
            loadPlaythroughFromFileAndReseed();

        ImGui::SameLine();

        uiButtonHueMod = 120.f;
        uiPushButtonColors();

        if (ImGui::Button("Reset game"))
            forceResetGame();

        ImGui::SameLine();

        if (ImGui::Button("Reset profile"))
            forceResetProfile();

        uiPopButtonColors();
        uiButtonHueMod = 0.f;

        ImGui::Separator();

        static int catTypeN = 0;
        ImGui::SetNextItemWidth(320.f * profile.uiScale);
        ImGui::Combo("typeN", &catTypeN, CatConstants::typeNames, nCatTypes);

        if (ImGui::Button("Spawn"))
        {
            const auto catType = static_cast<CatType>(catTypeN);

            if (isUniqueCatType(catType))
            {
                const auto pos = window.mapPixelToCoords((getResolution() / 2.f).toVec2i(), gameView);
                spawnSpecialCat(pos, catType);
            }
            else
                spawnCatCentered(catType, getHueByCatType(catType));
        }

        ImGui::SameLine();

        if (ImGui::Button("Do Ritual"))
            if (auto* wc = cachedWitchCat)
                wc->cooldown.value = 10.f;

        ImGui::SameLine();

        if (ImGui::Button("Do Copy Ritual"))
            if (auto* wc = cachedCopyCat)
                wc->cooldown.value = 10.f;

        ImGui::SameLine();

        if (ImGui::Button("Do Letter"))
        {
            victoryTC.emplace(TargetedCountdown{.startingValue = 6500.f});
            victoryTC->restart();
            delayedActions.emplaceBack(Countdown{.value = 7000.f}, [this] { playSound(sounds.letterchime); });
        }

        if (ImGui::Button("Do Tip"))
            doTip("Hello, I am a tip!\nHello world... How are you doing today?\nTest test test");

        ImGui::SameLine();

        if (ImGui::Button("Do Prestige"))
        {
            ++pt->psvBubbleValue.nPurchases;
            const auto ppReward = pt->calculatePrestigePointReward(1u);
            beginPrestigeTransition(ppReward);
        }

        ImGui::Separator();

        ImGui::Checkbox("hide ui", &debugHideUI);

        ImGui::Separator();

        ImGui::PushFont(fontImGuiMouldyCheese);
        uiSetFontScale(uiToolTipFontScale);

        SizeT step    = 1u;
        SizeT counter = 0u;

        static char filenameBuf[128] = "userdata/custom.json";

        ImGui::SetNextItemWidth(320.f * profile.uiScale);
        ImGui::InputText("##Filename", filenameBuf, sizeof(filenameBuf));

        if (ImGui::Button("Custom save"))
        {
            pt->fullVersion = !isDemoVersion;
            savePlaythroughToFile(*pt, filenameBuf);
        }

        ImGui::SameLine();

        if (ImGui::Button("Custom load"))
            (void)loadPlaythroughFromFile(*pt, filenameBuf);

        ImGui::Separator();

        if (ImGui::Button("Feed next shrine"))
        {
            for (Shrine& shrine : pt->shrines)
            {
                if (!shrine.isActive() || shrine.tcDeath.hasValue())
                    continue;

                const auto requiredReward = pt->getComputedRequiredRewardByShrineType(shrine.type);
                shrine.collectedReward += requiredReward / 3u;
                break;
            }
        }

        ImGui::Separator();

        ImGui::SetNextItemWidth(240.f * profile.uiScale);
        ImGui::InputScalar("Money", ImGuiDataType_U64, &pt->money, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

        ImGui::SetNextItemWidth(240.f * profile.uiScale);
        ImGui::InputScalar("PPs", ImGuiDataType_U64, &pt->prestigePoints, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

        ImGui::SetNextItemWidth(240.f * profile.uiScale);
        ImGui::InputScalar("WPs", ImGuiDataType_U64, &pt->wisdom, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

        ImGui::SetNextItemWidth(240.f * profile.uiScale);
        ImGui::InputScalar("Mana", ImGuiDataType_U64, &pt->mana, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

        ImGui::Separator();

        const auto scalarInput = [&](const char* label, float& value)
        {
            std::string lbuf = label;
            lbuf += "##";
            lbuf += std::to_string(counter++);

            ImGui::SetNextItemWidth(140.f * profile.uiScale);
            if (ImGui::InputScalar(lbuf.c_str(), ImGuiDataType_Float, &value, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
                value = sf::base::clamp(value, 0.f, 10'000.f);
        };

        const auto psvScalarInput = [&](const char* label, PurchasableScalingValue& psv)
        {
            if (psv.data->nMaxPurchases == 0u)
                return;

            std::string lbuf = label;
            lbuf += "##";
            lbuf += std::to_string(counter++);

            ImGui::SetNextItemWidth(140.f * profile.uiScale);
            if (ImGui::InputScalar(lbuf.c_str(), ImGuiDataType_U64, &psv.nPurchases, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
                psv.nPurchases = sf::base::clamp(psv.nPurchases, SizeT{0u}, psv.data->nMaxPurchases);
        };

        psvScalarInput("ComboStartTime", pt->psvComboStartTime);
        psvScalarInput("MapExtension", pt->psvMapExtension);
        psvScalarInput("ShrineActivation", pt->psvShrineActivation);
        psvScalarInput("BubbleCount", pt->psvBubbleCount);
        psvScalarInput("SpellCount", pt->psvSpellCount);
        psvScalarInput("BubbleValue", pt->psvBubbleValue);
        psvScalarInput("ExplosionRadiusMult", pt->psvExplosionRadiusMult);

        ImGui::Separator();

        for (SizeT i = 0u; i < nCatTypes; ++i)
        {
            scalarInput((std::to_string(i) + "Buff").c_str(), pt->buffCountdownsPerType[i].value);
        }

        ImGui::Separator();

        for (SizeT i = 0u; i < nCatTypes; ++i)
        {
            ImGui::Text("%s", CatConstants::typeNames[i]);
            psvScalarInput("PerCatType", pt->psvPerCatType[i]);
            psvScalarInput("CooldownMultsPerCatType", pt->psvCooldownMultsPerCatType[i]);
            psvScalarInput("RangeDivsPerCatType", pt->psvRangeDivsPerCatType[i]);

            ImGui::Separator();
        }

        psvScalarInput("PPMultiPopRange", pt->psvPPMultiPopRange);
        psvScalarInput("PPInspireDurationMult", pt->psvPPInspireDurationMult);
        psvScalarInput("PPManaCooldownMult", pt->psvPPManaCooldownMult);
        psvScalarInput("PPManaMaxMult", pt->psvPPManaMaxMult);
        psvScalarInput("PPMouseCatGlobalBonusMult", pt->psvPPMouseCatGlobalBonusMult);
        psvScalarInput("PPEngiCatGlobalBonusMult", pt->psvPPEngiCatGlobalBonusMult);
        psvScalarInput("PPRepulsoCatConverterChance", pt->psvPPRepulsoCatConverterChance);
        psvScalarInput("PPWitchCatBuffDuration", pt->psvPPWitchCatBuffDuration);
        psvScalarInput("PPUniRitualBuffPercentage", pt->psvPPUniRitualBuffPercentage);
        psvScalarInput("PPDevilRitualBuffPercentage", pt->psvPPDevilRitualBuffPercentage);

        ImGui::Separator();

        ImGui::Checkbox("starterPackPurchased", &pt->perm.starterPackPurchased);
        ImGui::Checkbox("multiPopPurchased", &pt->perm.multiPopPurchased);
        ImGui::Checkbox("smartCatsPurchased", &pt->perm.smartCatsPurchased);
        ImGui::Checkbox("geniusCatsPurchased", &pt->perm.geniusCatsPurchased);
        ImGui::Checkbox("windPurchased", &pt->perm.windPurchased);
        ImGui::Checkbox("astroCatInspirePurchased", &pt->perm.astroCatInspirePurchased);
        ImGui::Checkbox("starpawConversionIgnoreBombs", &pt->perm.starpawConversionIgnoreBombs);
        ImGui::Checkbox("starpawNova", &pt->perm.starpawNova);
        ImGui::Checkbox("repulsoCatFilterPurchased", &pt->perm.repulsoCatFilterPurchased);
        ImGui::Checkbox("repulsoCatConverterPurchased", &pt->perm.repulsoCatConverterPurchased);
        ImGui::Checkbox("repulsoCatNovaConverterPurchased", &pt->perm.repulsoCatNovaConverterPurchased);
        ImGui::Checkbox("attractoCatFilterPurchased", &pt->perm.attractoCatFilterPurchased);
        ImGui::Checkbox("witchCatBuffPowerScalesWithNCats", &pt->perm.witchCatBuffPowerScalesWithNCats);
        ImGui::Checkbox("witchCatBuffPowerScalesWithMapSize", &pt->perm.witchCatBuffPowerScalesWithMapSize);
        ImGui::Checkbox("witchCatBuffFewerDolls", &pt->perm.witchCatBuffFewerDolls);
        ImGui::Checkbox("witchCatBuffFlammableDolls", &pt->perm.witchCatBuffFlammableDolls);
        ImGui::Checkbox("witchCatBuffOrbitalDolls", &pt->perm.witchCatBuffOrbitalDolls);
        ImGui::Checkbox("wizardCatDoubleMewltiplierDuration", &pt->perm.wizardCatDoubleMewltiplierDuration);
        ImGui::Checkbox("wizardCatDoubleStasisFieldDuration", &pt->perm.wizardCatDoubleStasisFieldDuration);
        ImGui::Checkbox("unicatTranscendencePurchased", &pt->perm.unicatTranscendencePurchased);
        ImGui::Checkbox("unicatTranscendenceAOEPurchased", &pt->perm.unicatTranscendenceAOEPurchased);
        ImGui::Checkbox("devilcatHellsingedPurchased", &pt->perm.devilcatHellsingedPurchased);
        ImGui::Checkbox("unicatTranscendenceEnabled", &pt->perm.unicatTranscendenceEnabled);
        ImGui::Checkbox("devilcatHellsingedEnabled", &pt->perm.devilcatHellsingedEnabled);
        ImGui::Checkbox("autocastPurchased", &pt->perm.autocastPurchased);

        ImGui::Separator();

        ImGui::Checkbox("shrineCompleted Witch", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]);
        ImGui::Checkbox("shrineCompleted Wizard", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]);
        ImGui::Checkbox("shrineCompleted Mouse", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]);
        ImGui::Checkbox("shrineCompleted Engi", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]);
        ImGui::Checkbox("shrineCompleted Attracto", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]);
        ImGui::Checkbox("shrineCompleted Repulso", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]);
        ImGui::Checkbox("shrineCompleted Copy", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)]);
        ImGui::Checkbox("shrineCompleted Duck", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Duck)]);

        uiSetFontScale(uiNormalFontScale);
        ImGui::PopFont();

        ImGui::EndTabItem();
    }

    ImGui::Separator();
    uiSetFontScale(uiNormalFontScale);

    ImGui::Text("FPS: %.2f", static_cast<double>(fps));
}

////////////////////////////////////////////////////////////
void Main::gameLoopDrawImGui(const sf::base::U8 shouldDrawUIAlpha)
{
    if (profile.enableNotifications)
        ImGui::RenderNotifications(/* paddingY */ (profile.showDpsMeter ? (15.f + 60.f + 15.f) : 15.f) * profile.uiScale,
                                   [&]
        {
            ImGui::PushFont(fontImGuiMouldyCheese);
            uiSetFontScale(uiToolTipFontScale);
        },
                                   [&]
        {
            uiSetFontScale(uiNormalFontScale);
            ImGui::PopFont();
        });

    rtImGui.setView(scaledHUDView);
    rtImGui.clear(sf::Color::Transparent);
    imGuiContext.render(rtImGui);
    rtImGui.display();

    rtGame.draw(rtImGui.getTexture(),
                {.scale = {1.f / profile.hudScale, 1.f / profile.hudScale},
                 .color = hueColor(currentBackgroundHue.asDegrees(), shouldDrawUIAlpha)},
                {.shader = &shader});
}

////////////////////////////////////////////////////////////
void Main::gameLoopUpdateNotificationQueue(const float deltaTimeMs)
{
    if (tipTCByte.hasValue())
        return;

    if (notificationQueue.empty())
        return;

    if (notificationCountdown.updateAndIsActive(deltaTimeMs))
        return;

    notificationCountdown.restart();

    const auto& notification = notificationQueue.front();

    ImGuiToast toast{ImGuiToastType::None, 4500};
    toast.setTitle(notification.title);
    toast.setContent("%s", notification.content.c_str());

    ImGui::InsertNotification(toast);
    playSound(sounds.notification);

    // pop front
    notificationQueue.erase(notificationQueue.begin());
}

#pragma GCC diagnostic pop
