#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/ImGui/Export.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/Window/Joystick.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Event;
class RenderTarget;
class RenderTexture;
class RenderWindow;
class Texture;
class Window;
struct Sprite;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
class ImGuiContext
{
public:
    ////////////////////////////////////////////////////////////
    explicit ImGuiContext(bool loadDefaultFont = true);

    ////////////////////////////////////////////////////////////
    ~ImGuiContext();

    ////////////////////////////////////////////////////////////
    ImGuiContext(const ImGuiContext&)            = delete;
    ImGuiContext& operator=(const ImGuiContext&) = delete;

    ////////////////////////////////////////////////////////////
    ImGuiContext(ImGuiContext&&) noexcept;
    ImGuiContext& operator=(ImGuiContext&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void setActiveJoystickId(unsigned int joystickId);
    void setJoystickDPadThreshold(float threshold);
    void setJoystickLStickThreshold(float threshold);
    void setJoystickRStickThreshold(float threshold);
    void setJoystickLTriggerThreshold(float threshold);
    void setJoystickRTriggerThreshold(float threshold);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void setJoystickMapping(int key, unsigned int joystickButton);
    void setDPadXAxis(Joystick::Axis dPadXAxis, bool inverted = false);
    void setDPadYAxis(Joystick::Axis dPadYAxis, bool inverted = false);
    void setLStickXAxis(Joystick::Axis lStickXAxis, bool inverted = false);
    void setLStickYAxis(Joystick::Axis lStickYAxis, bool inverted = false);
    void setRStickXAxis(Joystick::Axis rStickXAxis, bool inverted = false);
    void setRStickYAxis(Joystick::Axis rStickYAxis, bool inverted = false);
    void setLTriggerAxis(Joystick::Axis lTriggerAxis);
    void setRTriggerAxis(Joystick::Axis rTriggerAxis);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void processEvent(const Window& window, const Event& event);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void updateMouseCursor(Window& window);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void update(RenderWindow& renderWindow, Time dt);
    void update(Window& window, RenderTarget& renderTarget, Time dt);
    void update(Vec2i mousePos, Vec2f displaySize, Time dt);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void render(RenderWindow& window);
    void render(RenderTarget& target);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void image(const Texture& texture, Color tintColor = Color::White, Color borderColor = Color::Transparent);
    void image(const Texture& texture, Vec2f size, Color tintColor = Color::White, Color borderColor = Color::Transparent);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void image(const RenderTexture& texture, Color tintColor = Color::White, Color borderColor = Color::Transparent);
    void image(const RenderTexture& texture, Vec2f size, Color tintColor = Color::White, Color borderColor = Color::Transparent);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void image(const Sprite&  sprite,
               const Texture& texture,
               Color          tintColor   = Color::White,
               Color          borderColor = Color::Transparent);

    void image(const Sprite&  sprite,
               const Texture& texture,
               Vec2f          size,
               Color          tintColor   = Color::White,
               Color          borderColor = Color::Transparent);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool imageButton(const char*    id,
                                   const Texture& texture,
                                   Vec2f          size,
                                   Color          bgColor   = Color::Transparent,
                                   Color          tintColor = Color::White);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool imageButton(const char*          id,
                                   const RenderTexture& texture,
                                   Vec2f                size,
                                   Color                bgColor   = Color::Transparent,
                                   Color                tintColor = Color::White);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool imageButton(const char*    id,
                                   const Sprite&  sprite,
                                   const Texture& texture,
                                   Vec2f          size,
                                   Color          bgColor   = Color::Transparent,
                                   Color          tintColor = Color::White);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawLine(Vec2f a, Vec2f b, Color col, float thickness = 1.f);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawRect(const FloatRect& rect, Color color, float rounding = 0.f, int roundingCorners = 0x0F, float thickness = 1.f);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawRectFilled(const FloatRect& rect, Color color, float rounding = 0.f, int roundingCorners = 0x0F);

private:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void initDefaultJoystickMapping();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl; //!< Implementation details
};

} // namespace sf
