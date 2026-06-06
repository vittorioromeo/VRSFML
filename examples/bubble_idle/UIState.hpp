#pragma once

#include "ExampleUtils/Progress.hpp"

#include "Zancle/String/String.hpp"

#include "Zancle/Container/AnkerlUnorderedDense.hpp"
#include "Zancle/Container/Vector.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"


////////////////////////////////////////////////////////////
struct PurchaseUnlockedEffect
{
    za::String widgetLabel;
    Countdown  countdown;
    Countdown  arrowCountdown;
    float      hue;
    int        type;
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

    ankerl::unordered_dense::map<za::String, float> uiLabelToY;

    za::Rect2f minimapRect;
    za::Rect2f minimapZoomButtonsRect;
    Countdown  scrollArrowCountdown;

    za::Vector<PurchaseUnlockedEffect>             purchaseUnlockedEffects;
    ankerl::unordered_dense::map<za::String, bool> btnWasDisabled;
    bool                                           debugHideUI        = false;
    bool                                           debugWindowVisible = true;

    int shopSelectOnce       = 1 << 1; // ImGuiTabItemFlags_SetSelected;
    int lastUiSelectedTabIdx = 1;
};
