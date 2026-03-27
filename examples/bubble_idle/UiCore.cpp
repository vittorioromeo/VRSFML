

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "IconsFontAwesome6.h"
#include "ImGuiNotify.hpp"
#include "Milestones.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"
#include "PurchasableScalingValue.hpp"
#include "Shrine.hpp"
#include "ShrineConstants.hpp"
#include "Version.hpp"

#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/HueColor.hpp"
#include "ExampleUtils/NinePatchUtils.hpp"
#include "ExampleUtils/Profiler.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"
#include "SFML/ImGui/IncludeImGuiInternal.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawTextureSettings.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Algorithm/AnyOf.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FloatMax.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
static_assert(sizeof(unsigned int) <= sizeof(ImTextureID), "ImTextureID is not large enough to fit unsigned int.");

////////////////////////////////////////////////////////////
[[nodiscard]] ImTextureID toImTextureID(const unsigned int nativeHandle)
{
    ImTextureID textureID{};
    std::memcpy(&textureID, &nativeHandle, sizeof(nativeHandle));
    return textureID;
}

////////////////////////////////////////////////////////////
void drawNinePatchInImGui(ImDrawList&            drawList,
                          const sf::Texture&     texture,
                          const sf::Vec2f        position,
                          const sf::Vec2f        size,
                          const sf::Rect2f       textureRect,
                          const NinePatchBorders borders,
                          const ImU32            color)
{
    if (size.x <= 0.f || size.y <= 0.f)
        return;

    const sf::Rect2f sourceRect = textureRect == sf::Rect2f{} ? texture.getRect() : textureRect;

    if (sourceRect.size.x <= 0.f || sourceRect.size.y <= 0.f)
        return;

    const auto srcX    = makeNinePatchSlices(sourceRect.size.x, borders.left, borders.right);
    const auto srcY    = makeNinePatchSlices(sourceRect.size.y, borders.top, borders.bottom);
    const auto dstX    = makeNinePatchSlices(size.x, borders.left, borders.right);
    const auto dstY    = makeNinePatchSlices(size.y, borders.top, borders.bottom);
    const auto srcPosX = makeNinePatchPositions(sourceRect.position.x, srcX);
    const auto srcPosY = makeNinePatchPositions(sourceRect.position.y, srcY);
    const auto dstPosX = makeNinePatchPositions(position.x, dstX);
    const auto dstPosY = makeNinePatchPositions(position.y, dstY);

    const sf::Vec2f   textureSize = texture.getSize().toVec2f();
    const ImTextureID textureID   = toImTextureID(texture.getNativeHandle());

    drawList.PushClipRectFullScreen();

    for (sf::base::SizeT iy = 0; iy < 3u; ++iy)
    {
        for (sf::base::SizeT ix = 0; ix < 3u; ++ix)
        {
            if (srcX[ix] <= 0.f || srcY[iy] <= 0.f || dstX[ix] <= 0.f || dstY[iy] <= 0.f)
                continue;

            const ImVec2 pMin{dstPosX[ix], dstPosY[iy]};
            const ImVec2 pMax{dstPosX[ix] + dstX[ix], dstPosY[iy] + dstY[iy]};
            const ImVec2 uvMin{srcPosX[ix] / textureSize.x, srcPosY[iy] / textureSize.y};
            const ImVec2 uvMax{(srcPosX[ix] + srcX[ix]) / textureSize.x, (srcPosY[iy] + srcY[iy]) / textureSize.y};

            drawList.AddImage(textureID, pMin, pMax, uvMin, uvMax, color);
        }
    }

    drawList.PopClipRect();
}
} // namespace

////////////////////////////////////////////////////////////

float Main::uiGetMaxWindowHeight() const
{
    return sf::base::max(getResolution().y - 46.f, (getResolution().y - 46.f) / profile.uiScale);
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
        fontScaleMult = sf::base::pow(fontScaleMult, 0.4f);

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
    const float scaledWindowWidth = uiWindowWidth * profile.uiScale;
    const float rightAnchorX      = getResolution().x - scaledWindowWidth - 15.f * profile.uiScale;

    return {rightAnchorX - 8.f, 15.f + 16.f};
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
    const float xStart = lastUiSelectedTabIdx == 0
                             ? getResolution().x
                             : gameView.worldToScreen({getLeftMostUsefulX(), 0.f}, getResolution()).x;

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

    ImGui::GetIO().FontGlobalScale = newScalingFactor * 0.975f;

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
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    // TODO P0: cleanup
    // 1. Define your colors (Top and Bottom)
    ImU32 col_top = ImColor(25, 65, 125, 220); // Deep Blue
    ImU32 col_bot = ImColor(5, 20, 45, 240);   // Darker Navy Blue


    // 3. Get the DrawList and Window coordinates
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2      p_min     = ImGui::GetWindowPos();
    ImVec2      p_max     = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + ImGui::GetWindowHeight());


    // 4. Draw the Gradient (Top-Left, Top-Right, Bottom-Right, Bottom-Left)
    draw_list->AddRectFilledMultiColor(p_min, p_max, col_top, col_top, col_bot, col_bot);

    // float rounding = 8.f;
    // draw_list->AddRectFilled(p_min, p_max, col_top, rounding, ImDrawFlags_RoundCornersAll);

    {
        const auto prevTabRounding      = style.TabRounding;
        const auto prevItemInnerSpacing = style.ItemInnerSpacing;

        style.TabRounding      = 8.0f;
        style.ItemInnerSpacing = {0.f, 0.f};

        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.15f, 0.35f, 0.60f, 1.0f));        // Inactive
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.25f, 0.45f, 0.80f, 1.0f)); // Hover
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.35f, 0.55f, 0.95f, 1.0f));  // Active
        ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(0.15f, 0.35f, 0.60f, 1.0f));

        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_DrawSelectedOverline))
        {
            uiTabBar();
            ImGui::EndTabBar();
        }

        ImGui::PopStyleColor(4);

        style.TabRounding      = prevTabRounding;
        style.ItemInnerSpacing = prevItemInnerSpacing;
    }

    if (!ImGui::GetIO().WantCaptureMouse && particleCullingBoundaries.isInside(mousePos))
        uiMakeShrineOrCatTooltip(mousePos);

    const auto windowDrawPos  = ImGui::GetWindowPos();
    const auto windowDrawSize = ImGui::GetWindowSize();

    if (windowDrawSize.y > 64.f)
    {
        const float offset = 15.f;

        drawNinePatchInImGui(*draw_list,
                             txFrame,
                             windowDrawPos - sf::Vec2f{offset, offset} + sf::Vec2f{2.f, 1.f},
                             windowDrawSize + sf::Vec2f{offset * 2.f, offset * 2.f} - sf::Vec2f{3.f, 3.f},
                             txFrame.getRect(),
                             NinePatchBorders::all(64.f),
                             IM_COL32_WHITE);
    }
    else
    {
        const float offset = 4.f * profile.uiScale;

        drawNinePatchInImGui(*draw_list,
                             txFrameTiny,
                             windowDrawPos - sf::Vec2f{offset, offset} + sf::Vec2f{2.f, 1.f},
                             windowDrawSize + sf::Vec2f{offset * 2.f, offset * 2.f} - sf::Vec2f{3.f, 3.f},
                             txFrameTiny.getRect(),
                             NinePatchBorders::all(18.f),
                             IM_COL32_WHITE);
    }

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

    const auto oldRounding            = ImGui::GetStyle().FrameRounding;
    const auto oldBorderSize          = ImGui::GetStyle().FrameBorderSize;
    ImGui::GetStyle().FrameRounding   = 0.f;
    ImGui::GetStyle().FrameBorderSize = 0.f;

    ImGui::Begin("##dpsmeter",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs);

    ImDrawList* const drawList = ImGui::GetWindowDrawList();

    {
        const float offset = 8.f * profile.uiScale;

        const ImVec2 pMin = ImGui::GetWindowPos();
        const ImVec2 pMax = ImVec2(pMin.x + ImGui::GetWindowWidth(), pMin.y + ImGui::GetWindowHeight());

        drawList->AddRectFilledMultiColor(pMin + ImVec2{offset, offset},
                                          pMax - ImVec2{offset, offset},
                                          ImColor(25, 65, 125, 220),
                                          ImColor(25, 65, 125, 220),
                                          ImColor(5, 20, 45, 240),
                                          ImColor(5, 20, 45, 240));
    }

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

    const auto windowDrawPos  = sf::Vec2f(ImGui::GetWindowPos());
    const auto windowDrawSize = sf::Vec2f(ImGui::GetWindowSize());

    {
        const float offset = -2.f * profile.uiScale;

        drawNinePatchInImGui(*drawList,
                             txFrameTiny,
                             windowDrawPos - sf::Vec2f{offset, offset} + sf::Vec2f{1.f, 1.f},
                             windowDrawSize + sf::Vec2f{offset * 2.f, offset * 2.f} - sf::Vec2f{1.f, 3.f},
                             txFrameTiny.getRect(),
                             NinePatchBorders::all(18.f),
                             IM_COL32_WHITE);
    }

    ImGui::GetStyle().FrameRounding   = oldRounding;
    ImGui::GetStyle().FrameBorderSize = oldBorderSize;

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

    if (ImGui::BeginTabItem(ICON_FA_CHEVRON_UP,
                            nullptr,
                            keyboardSelectedTab(sf::Keyboard::Key::Slash) | keyboardSelectedTab(sf::Keyboard::Key::Grave) |
                                keyboardSelectedTab(sf::Keyboard::Key::Apostrophe) |
                                keyboardSelectedTab(sf::Keyboard::Key::Backslash)))
    {
        selectedTab(0);

        ImGui::EndTabItem();
    }

    sf::base::SizeT nextTabKeyIndex = 0u;

    constexpr sf::Keyboard::Key tabKeys[] = {
        sf::Keyboard::Key::Num1,
        sf::Keyboard::Key::Num2,
        sf::Keyboard::Key::Num3,
        sf::Keyboard::Key::Num4,
        sf::Keyboard::Key::Num5,
        sf::Keyboard::Key::Num6,
    };

    if (ImGui::BeginTabItem(ICON_FA_GEAR, nullptr, {}))
    {
        selectedTab(5);

        ImGui::BeginChild("OptionsScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight), 0, ImGuiWindowFlags_None);

        uiTabBarSettings();

        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem(ICON_FA_CIRCLE_INFO, nullptr, {}))
    {
        selectedTab(4);

        ImGui::BeginChild("StatsScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight), 0, ImGuiWindowFlags_None);

        uiTabBarStats();

        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f); // Make it invisible
    if (ImGui::BeginTabItem("##spacer0", nullptr, ImGuiTabItemFlags_NoTooltip))
        ImGui::EndTabItem();
    ImGui::PopStyleVar();

    if (ImGui::BeginTabItem(ICON_FA_STORE " Shop", nullptr, shopSelectOnce | keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
    {
        selectedTab(1);

        shopSelectOnce = {};

        ImGui::BeginChild("ShopScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight), 0, ImGuiWindowFlags_None);
        uiTabBarShop();

        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f); // Make it invisible
    if (ImGui::BeginTabItem("##spacer1", nullptr, ImGuiTabItemFlags_NoTooltip))
        ImGui::EndTabItem();
    ImGui::PopStyleVar();

    if (cachedWizardCat != nullptr &&
        ImGui::BeginTabItem(ICON_FA_STAR " Magic", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
    {
        selectedTab(2);

        ImGui::BeginChild("MagicScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight), 0, ImGuiWindowFlags_None);

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

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f); // Make it invisible
        if (ImGui::BeginTabItem("##spacer2", nullptr, ImGuiTabItemFlags_NoTooltip))
            ImGui::EndTabItem();
        ImGui::PopStyleVar();

        if (ImGui::BeginTabItem(ICON_FA_TROPHY " Prestige", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
        {
            selectedTab(3);

            ImGui::BeginChild("PrestigeScroll", ImVec2(ImGui::GetContentRegionAvail().x, childHeight), 0, ImGuiWindowFlags_None);

            uiTabBarPrestige();

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (canPrestige)
            ImGui::PopStyleColor(3);
    }
}

////////////////////////////////////////////////////////////
void Main::uiSetUnlockLabelY(const sf::base::SizeT unlockId)
{
    const sf::base::String label = sf::base::toString(unlockId);
    uiLabelToY[label]            = ImGui::GetCursorScreenPos().y;
}

////////////////////////////////////////////////////////////
bool Main::checkUiUnlock(const sf::base::SizeT unlockId, const bool unlockCondition)
{
    const sf::base::String label = sf::base::toString(unlockId);

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
void Main::uiImageFromAtlas(const sf::Rect2f& txr, const sf::DrawTextureSettings& drawParams)
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

void Main::gameLoopDrawImGui(const sf::base::U8 shouldDrawUIAlpha)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

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

    rtImGui.clear(sf::Color::Transparent);
    imGuiContext.render(rtImGui);
    rtImGui.display();

    rtGame.draw(rtImGui.getTexture(),
                {.scale = {1.f / profile.hudScale, 1.f / profile.hudScale},
                 .color = hueColor(currentBackgroundHue.asDegrees(), shouldDrawUIAlpha)},
                {.view = scaledHUDView, .shader = &shader});
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
    toast.setContent("%s", notification.content.cStr());

    ImGui::InsertNotification(toast);
    playSound(sounds.notification);

    // pop front
    notificationQueue.erase(notificationQueue.begin());
}
