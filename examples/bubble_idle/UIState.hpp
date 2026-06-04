#pragma once

#include "ExampleUtils/Progress.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/AnkerlUnorderedDense.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
struct PurchaseUnlockedEffect
{
    zb::String widgetLabel;
    Countdown        countdown;
    Countdown        arrowCountdown;
    float            hue;
    int              type;
};


////////////////////////////////////////////////////////////
struct UIState
{
    static inline constexpr unsigned int uiBufferSize        = 256u;
    static inline constexpr unsigned int uiLabelBufferSize   = 512u;
    static inline constexpr unsigned int uiTooltipBufferSize = 1024u;

    char         uiBuffer[uiBufferSize]{};
    char         uiLabelBuffer[uiLabelBufferSize]{};
    char         uiTooltipBuffer[uiTooltipBufferSize]{};
    float        uiButtonHueMod  = 0.f;
    unsigned int uiWidgetId      = 0u;
    float        lastFontScale   = 1.f;
    float        uiMenuRevealT   = 1.f;
    float        uiMenuHideTimer = 0.75f;
    bool         uiMenuLocked    = false;
    za::Vec2f    uiMenuLastDrawPos{};
    za::Vec2f    uiMenuLastDrawSize{425.f, 0.f};

    ankerl::unordered_dense::map<zb::String, float> uiLabelToY;

    za::Rect2f minimapRect;
    za::Rect2f minimapZoomButtonsRect;
    Countdown  scrollArrowCountdown;

    zb::Vector<PurchaseUnlockedEffect>             purchaseUnlockedEffects;
    ankerl::unordered_dense::map<zb::String, bool> btnWasDisabled;
    bool                                                 debugHideUI        = false;
    bool                                                 debugWindowVisible = true;

    int shopSelectOnce       = 1 << 1; // ImGuiTabItemFlags_SetSelected;
    int lastUiSelectedTabIdx = 1;
};
