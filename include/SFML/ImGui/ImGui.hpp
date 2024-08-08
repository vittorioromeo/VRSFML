#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>

#include <SFML/Graphics/Color.hpp>

#include <SFML/Window/Joystick.hpp>

#include <SFML/System/Rect.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <SFML/Base/InPlacePImpl.hpp>
#include <SFML/Base/Optional.hpp>

namespace sf
{
class Event;
class GraphicsContext;
class RenderTarget;
class RenderTexture;
class RenderWindow;
class Sprite;
class Texture;
class Window;
} // namespace sf


namespace sf::ImGui
{
class [[nodiscard]] ImGuiContext
{
public:
    [[nodiscard]] explicit ImGuiContext(GraphicsContext& graphicsContext);
    ~ImGuiContext();

    ImGuiContext(const ImGuiContext&)            = delete;
    ImGuiContext& operator=(const ImGuiContext&) = delete;

    ImGuiContext(ImGuiContext&&) noexcept;
    ImGuiContext& operator=(ImGuiContext&&) noexcept;

    [[nodiscard]] bool init(RenderWindow& window, bool loadDefaultFont = true);
    [[nodiscard]] bool init(Window& window, RenderTarget& target, bool loadDefaultFont = true);
    [[nodiscard]] bool init(Window& window, Vector2f displaySize, bool loadDefaultFont = true);

    void processEvent(const Window& window, const Event& event);

    void update(RenderWindow& window, Time dt);
    void update(Window& window, RenderTarget& target, Time dt);
    void update(Vector2i mousePos, Vector2f displaySize, Time dt);

    void render(RenderWindow& window);
    void render(RenderTarget& target);
    void render();

    void shutdown(const Window& window);

    void setActiveJoystickId(unsigned int joystickId);
    void setJoystickDPadThreshold(float threshold);
    void setJoystickLStickThreshold(float threshold);
    void setJoystickRStickThreshold(float threshold);
    void setJoystickLTriggerThreshold(float threshold);
    void setJoystickRTriggerThreshold(float threshold);

    void setJoystickMapping(int key, unsigned int joystickButton);
    void setDPadXAxis(Joystick::Axis dPadXAxis, bool inverted = false);
    void setDPadYAxis(Joystick::Axis dPadYAxis, bool inverted = false);
    void setLStickXAxis(Joystick::Axis lStickXAxis, bool inverted = false);
    void setLStickYAxis(Joystick::Axis lStickYAxis, bool inverted = false);
    void setRStickXAxis(Joystick::Axis rStickXAxis, bool inverted = false);
    void setRStickYAxis(Joystick::Axis rStickYAxis, bool inverted = false);
    void setLTriggerAxis(Joystick::Axis lTriggerAxis);
    void setRTriggerAxis(Joystick::Axis rTriggerAxis);

    void setCurrentWindow(const Window& window);

private:
    // Shuts down all ImGui contexts
    void shutdown();

    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl; //!< Implementation details
};


// joystick functions

// custom SFML overloads for ImGui widgets

// Image overloads for sf::Texture
SFML_GRAPHICS_API void Image(const sf::Texture& texture,
                             sf::Color          tintColor   = sf::Color::White,
                             sf::Color          borderColor = sf::Color::Transparent);
SFML_GRAPHICS_API void Image(const sf::Texture& texture,
                             sf::Vector2f       size,
                             sf::Color          tintColor   = sf::Color::White,
                             sf::Color          borderColor = sf::Color::Transparent);

// Image overloads for sf::RenderTexture
SFML_GRAPHICS_API void Image(const sf::RenderTexture& texture,
                             sf::Color                tintColor   = sf::Color::White,
                             sf::Color                borderColor = sf::Color::Transparent);
SFML_GRAPHICS_API void Image(const sf::RenderTexture& texture,
                             sf::Vector2f             size,
                             sf::Color                tintColor   = sf::Color::White,
                             sf::Color                borderColor = sf::Color::Transparent);

// Image overloads for sf::Sprite
SFML_GRAPHICS_API void Image(const sf::Sprite&  sprite,
                             const sf::Texture& texture,
                             sf::Color          tintColor   = sf::Color::White,
                             sf::Color          borderColor = sf::Color::Transparent);
SFML_GRAPHICS_API void Image(const sf::Sprite&  sprite,
                             const sf::Texture& texture,
                             sf::Vector2f       size,
                             sf::Color          tintColor   = sf::Color::White,
                             sf::Color          borderColor = sf::Color::Transparent);

// ImageButton overloads for sf::Texture
[[nodiscard]] SFML_GRAPHICS_API bool ImageButton(
    const char*        id,
    const sf::Texture& texture,
    sf::Vector2f       size,
    sf::Color          bgColor   = sf::Color::Transparent,
    sf::Color          tintColor = sf::Color::White);

// ImageButton overloads for sf::RenderTexture
[[nodiscard]] SFML_GRAPHICS_API bool ImageButton(
    const char*              id,
    const sf::RenderTexture& texture,
    sf::Vector2f             size,
    sf::Color                bgColor   = sf::Color::Transparent,
    sf::Color                tintColor = sf::Color::White);

// ImageButton overloads for sf::Sprite
[[nodiscard]] SFML_GRAPHICS_API bool ImageButton(
    const char*        id,
    const sf::Sprite&  sprite,
    const sf::Texture& texture,
    sf::Vector2f       size,
    sf::Color          bgColor   = sf::Color::Transparent,
    sf::Color          tintColor = sf::Color::White);

// Draw_list overloads. All positions are in relative coordinates (relative to top-left of the
// current window)
SFML_GRAPHICS_API void DrawLine(sf::Vector2f a, sf::Vector2f b, sf::Color col, float thickness = 1.0f);
SFML_GRAPHICS_API void DrawRect(const sf::FloatRect& rect,
                                sf::Color            color,
                                float                rounding        = 0.0f,
                                int                  roundingCorners = 0x0F,
                                float                thickness       = 1.0f);
SFML_GRAPHICS_API void DrawRectFilled(const sf::FloatRect& rect, sf::Color color, float rounding = 0.0f, int roundingCorners = 0x0F);
} // namespace sf::ImGui
