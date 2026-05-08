/**
 * @file ImGuiNotify.hpp
 * @brief Toast notifications with ImGui.
 *
 * Based on imgui-notify by patrickcjk
 * https://github.com/patrickcjk/imgui-notify
 *
 * @version 0.0.3 by TyomaVader
 * @date 07.07.2024
 */
#pragma once


#pragma GCC system_header

#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"

#include <cstdarg>


/**
 * CONFIGURATION SECTION Start
 */

#define NOTIFY_MAX_MSG_LENGTH    4096 // Max message content length
#define NOTIFY_PADDING_X         15.f // Bottom-left X padding
#define NOTIFY_PADDING_MESSAGE_Y 10.f // Padding Y between each message
#define NOTIFY_FADE_IN_OUT_TIME  150  // Fade in and out duration
#define NOTIFY_DEFAULT_DISMISS \
    3000                          // Auto dismiss after X ms (default, applied only of no data provided in constructors)
#define NOTIFY_OPACITY       0.8f // 0-1 Toast opacity
#define NOTIFY_USE_SEPARATOR false // If true, a separator will be rendered between the title and the content
#define NOTIFY_USE_DISMISS_BUTTON \
    false                     // If true, a dismiss button will be rendered in the top right corner of the toast
#define NOTIFY_RENDER_LIMIT 5 // Max number of toasts rendered at the same time. Set to 0 for unlimited

// Warning: Requires ImGui docking with multi-viewport enabled
#define NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW \
    false // If true, the notifications will be rendered in the corner of the monitor, otherwise in the corner of the main window

/**
 * CONFIGURATION SECTION End
 */


struct ImVec4;
typedef int ImGuiWindowFlags;

extern const ImGuiWindowFlags NOTIFY_DEFAULT_TOAST_FLAGS;


enum class ImGuiToastType : sf::base::U8
{
    None,
    Success,
    Warning,
    Error,
    Info,
    COUNT
};

enum class ImGuiToastPhase : sf::base::U8
{
    FadeIn,
    Wait,
    FadeOut,
    Expired,
    COUNT
};

enum class ImGuiToastPos : sf::base::U8
{
    TopLeft,
    TopCenter,
    TopRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    Center,
    COUNT
};


/**
 * @brief A class for creating toast notifications with ImGui.
 */
class ImGuiToast
{
private:
    ImGuiWindowFlags flags;

    ImGuiToastType type = ImGuiToastType::None;
    char           title[NOTIFY_MAX_MSG_LENGTH];
    char           content[NOTIFY_MAX_MSG_LENGTH];

    int      dismissTime  = NOTIFY_DEFAULT_DISMISS;
    sf::Time creationTime = {};

    char buttonLabel[NOTIFY_MAX_MSG_LENGTH];

private:
    [[gnu::format(printf, 2, 0)]] void setTitle(const char* format, va_list args);
    [[gnu::format(printf, 2, 0)]] void setContent(const char* format, va_list args);
    [[gnu::format(printf, 2, 0)]] void setButtonLabel(const char* format, va_list args);

public:
    [[gnu::format(printf, 2, 3)]] void setTitle(const char* format, ...);
    [[gnu::format(printf, 2, 3)]] void setContent(const char* format, ...);
    void                               setType(const ImGuiToastType& newType);
    void                               setWindowFlags(const ImGuiWindowFlags& newFlags);
    [[gnu::format(printf, 2, 3)]] void setButtonLabel(const char* format, ...);

    [[nodiscard]] const char*      getTitle();
    [[nodiscard]] const char*      getDefaultTitle();
    [[nodiscard]] ImGuiToastType   getType();
    [[nodiscard]] ImVec4           getColor();
    [[nodiscard]] const char*      getIcon();
    [[nodiscard]] char*            getContent();
    [[nodiscard]] sf::Time         getElapsedTime();
    [[nodiscard]] ImGuiToastPhase  getPhase();
    [[nodiscard]] float            getFadePercent();
    [[nodiscard]] ImGuiWindowFlags getWindowFlags();
    [[nodiscard]] const char*      getButtonLabel();

    ImGuiToast(ImGuiToastType type, int dismissTime = NOTIFY_DEFAULT_DISMISS);
    [[gnu::format(printf, 3, 4)]] ImGuiToast(ImGuiToastType type, const char* format, ...);
    [[gnu::format(printf, 4, 5)]] ImGuiToast(ImGuiToastType type, int dismissTime, const char* format, ...);
};


namespace ImGui
{
using NotificationCallback = void (*)(float opacity, void* userData);

void InsertNotification(const ImGuiToast& toast);
void RemoveNotification(int index);
void RenderNotifications(float paddingY, NotificationCallback afterBegin, NotificationCallback beforeEnd, void* userData);

template <typename AfterBegin, typename BeforeEnd>
void RenderNotifications(const float paddingY, AfterBegin&& afterBegin, BeforeEnd&& beforeEnd)
{
    struct Context
    {
        AfterBegin& afterBegin;
        BeforeEnd&  beforeEnd;
    };

    Context context{afterBegin, beforeEnd};

    RenderNotifications(paddingY,
                        [](const float opacity, void* const userData)
    {
        auto& ctx = *static_cast<Context*>(userData);
        ctx.afterBegin(opacity);
    },
                        [](const float opacity, void* const userData)
    {
        auto& ctx = *static_cast<Context*>(userData);
        ctx.beforeEnd(opacity);
    },
                        &context);
}

} // namespace ImGui
