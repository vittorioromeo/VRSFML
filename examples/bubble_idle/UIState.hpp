#pragma once

#include "Countdown.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
struct PurchaseUnlockedEffect
{
    sf::base::String widgetLabel;
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
    sf::Vec2f    uiMenuLastDrawPos{};
    sf::Vec2f    uiMenuLastDrawSize{425.f, 0.f};

    ankerl::unordered_dense::map<sf::base::String, float> uiLabelToY;

    sf::Rect2f minimapRect;
    sf::Rect2f minimapZoomButtonsRect;
    Countdown  scrollArrowCountdown;

    sf::base::Vector<PurchaseUnlockedEffect>             purchaseUnlockedEffects;
    ankerl::unordered_dense::map<sf::base::String, bool> btnWasDisabled;
    bool                                                 debugHideUI        = false;
    bool                                                 debugWindowVisible = true;

    ImGuiTabItemFlags_ shopSelectOnce       = ImGuiTabItemFlags_SetSelected;
    int                lastUiSelectedTabIdx = 1;
};
