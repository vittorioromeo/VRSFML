////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Joystick.hpp"

#include "ExampleUtils/Scaling.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/View.hpp" // IWYU pragma: keep

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
// Axis labels as C strings
////////////////////////////////////////////////////////////
constexpr sf::base::Array<const char*, sf::Joystick::AxisCount> axisLabels =
    {"X", "Y", "Z", "R", "U", "V", "PovX", "PovY"};


////////////////////////////////////////////////////////////
// Logical display area for one joystick view (gamepad / generic)
////////////////////////////////////////////////////////////
constexpr sf::Vec2f displaySize{800.f, 400.f};


////////////////////////////////////////////////////////////
// Display for a single axis: vertical bar with a red indicator
////////////////////////////////////////////////////////////
class AxisDisplay : public sf::Transformable
{
public:
    AxisDisplay(const sf::Font& font, sf::Joystick::Axis axis) :
        m_label(font,
                {
                    .position      = {0.f, barSize.y / 2.f},
                    .string        = axisLabels[static_cast<unsigned int>(axis)],
                    .characterSize = static_cast<unsigned int>(barSize.x),
                }),
        m_axis(axis)
    {
        const sf::Rect2f labelBounds = m_label.getLocalBounds();
        m_label.origin               = labelBounds.position + labelBounds.size / 2.f;
    }

    void setValue(float value)
    {
        m_value = value;
    }

    [[nodiscard]] sf::Joystick::Axis getAxis() const
    {
        return m_axis;
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        target.draw(
            sf::RectangleShapeData{
                .origin           = {barSize.x / 2.f, 0.f},
                .fillColor        = sf::Color::Transparent,
                .outlineColor     = sf::Color::White,
                .outlineThickness = 1.f,
                .size             = barSize,
            },
            states);

        // Map the [-100, 100] axis range to a [0, barSize.y] vertical fill
        target.draw(
            sf::RectangleShapeData{
                .origin    = {barSize.x / 2.f, 0.f},
                .fillColor = sf::Color::Red,
                .size      = {barSize.x, (100.f + m_value) * barSize.y / 200.f},
            },
            states);

        target.draw(m_label, states);
    }

private:
    static constexpr sf::Vec2f barSize{20.f, 50.f};

    sf::Text           m_label;
    sf::Joystick::Axis m_axis;
    float              m_value{};
};


////////////////////////////////////////////////////////////
// Display for an _actual_ joystick (i.e. two combined axes,
// optionally bound to a press button like L3/R3)
////////////////////////////////////////////////////////////
class JoystickDisplay : public sf::Transformable
{
public:
    JoystickDisplay(const sf::Font&                  font,
                    sf::Joystick::Axis               xAxis,
                    sf::Joystick::Axis               yAxis,
                    sf::base::Optional<unsigned int> button = sf::base::nullOpt) :
        m_xAxis(xAxis),
        m_yAxis(yAxis),
        m_button(button),
        m_label(font,
                {
                    .string        = sf::base::String(axisLabels[static_cast<unsigned int>(xAxis)]) + "/" +
                                     axisLabels[static_cast<unsigned int>(yAxis)],
                    .characterSize = 20u,
                })
    {
        const sf::Rect2f labelBounds = m_label.getLocalBounds();
        m_label.origin               = labelBounds.position + labelBounds.size / 2.f;
    }

    void setX(float x)
    {
        m_xValue = x;
    }

    void setY(float y)
    {
        m_yValue = y;
    }

    [[nodiscard]] sf::Joystick::Axis getXAxis() const
    {
        return m_xAxis;
    }

    [[nodiscard]] sf::Joystick::Axis getYAxis() const
    {
        return m_yAxis;
    }

    void setPressed(bool pressed)
    {
        m_pressed = pressed;
    }

    [[nodiscard]] sf::base::Optional<unsigned int> getButton() const
    {
        return m_button;
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        target.draw(
            sf::CircleShapeData{
                .origin           = {radius, radius},
                .fillColor        = m_pressed ? sf::Color::Red : sf::Color::Transparent,
                .outlineColor     = sf::Color::White,
                .outlineThickness = 1.f,
                .radius           = radius,
            },
            states);

        target.draw(
            sf::CircleShapeData{
                .position  = {m_xValue * radius / 100.f, m_yValue * radius / 100.f},
                .origin    = {indicatorRadius, indicatorRadius},
                .fillColor = sf::Color::Red,
                .radius    = indicatorRadius,
            },
            states);

        target.draw(m_label, states);
    }

private:
    static constexpr float radius          = 50.f;
    static constexpr float indicatorRadius = 5.f;

    sf::Joystick::Axis               m_xAxis;
    sf::Joystick::Axis               m_yAxis;
    sf::base::Optional<unsigned int> m_button;
    sf::Text                         m_label;
    float                            m_xValue{};
    float                            m_yValue{};
    bool                             m_pressed{};
};


////////////////////////////////////////////////////////////
// Display for a single button: small circle that fills red when pressed
////////////////////////////////////////////////////////////
class ButtonDisplay : public sf::Transformable
{
public:
    ButtonDisplay(const sf::Font& font, unsigned int button) :
        m_label(font,
                {
                    .string        = sf::base::toString(button),
                    .characterSize = static_cast<unsigned int>(2.f * radius),
                }),
        m_button(button)
    {
        const sf::Rect2f labelBounds = m_label.getLocalBounds();
        m_label.origin               = labelBounds.position + labelBounds.size / 2.f;
    }

    void setPressed(bool pressed)
    {
        m_pressed = pressed;
    }

    [[nodiscard]] unsigned int getButton() const
    {
        return m_button;
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        target.draw(
            sf::CircleShapeData{
                .origin           = {radius, radius},
                .fillColor        = m_pressed ? sf::Color::Red : sf::Color::Transparent,
                .outlineColor     = sf::Color::White,
                .outlineThickness = 1.f,
                .radius           = radius,
            },
            states);

        target.draw(m_label, states);
    }

private:
    static constexpr float radius = 10.f;

    sf::Text     m_label;
    unsigned int m_button;
    bool         m_pressed{};
};


////////////////////////////////////////////////////////////
// Display approximating the layout of a typical gamepad
////////////////////////////////////////////////////////////
class GamepadDisplay : public sf::Transformable
{
public:
    GamepadDisplay(const sf::Font& font, unsigned int id, const sf::base::String& name) :
        m_label(font,
                {
                    .string        = sf::base::toString(id) + ": " + name,
                    .characterSize = 20u,
                })
    {
        // Sticks
        m_joysticks
            .emplaceBack(font, sf::Joystick::Axis::X, sf::Joystick::Axis::Y, sf::base::makeOptional<unsigned int>(10u))
            .position = {displaySize.x / 2.f - 100.f, displaySize.y - 50.f};

        m_joysticks
            .emplaceBack(font, sf::Joystick::Axis::Z, sf::Joystick::Axis::R, sf::base::makeOptional<unsigned int>(11u))
            .position = {displaySize.x / 2.f + 100.f, displaySize.y - 50.f};

        // Face buttons (north/south/east/west diamond)
        constexpr sf::Vec2f mainButtonPos{displaySize.x / 2.f + 250.f, displaySize.y / 2.f - 50.f};
        constexpr float     offset = 20.f;

        m_buttons.emplaceBack(font, 3u).position = mainButtonPos + sf::Vec2f{0.f, -offset}; // North
        m_buttons.emplaceBack(font, 1u).position = mainButtonPos + sf::Vec2f{0.f, offset};  // South
        m_buttons.emplaceBack(font, 2u).position = mainButtonPos + sf::Vec2f{offset, 0.f};  // East
        m_buttons.emplaceBack(font, 0u).position = mainButtonPos + sf::Vec2f{-offset, 0.f}; // West

        // Triggers
        m_triggers.emplaceBack(font, sf::Joystick::Axis::V); // Right trigger
        m_triggers.back().position = {displaySize.x / 2.f + 100.f, displaySize.y / 5.f};
        m_triggers.emplaceBack(font, sf::Joystick::Axis::U); // Left trigger
        m_triggers.back().position = {displaySize.x / 2.f - 100.f, displaySize.y / 5.f};

        // Trigger buttons (above the trigger bars)
        m_buttons.emplaceBack(font, 7u).position = m_triggers[0].position + sf::Vec2f{0.f, -30.f};
        m_buttons.emplaceBack(font, 6u).position = m_triggers[1].position + sf::Vec2f{0.f, -30.f};

        // Shoulder buttons
        m_buttons.emplaceBack(font, 5u).position = {displaySize.x / 2.f + 100.f, displaySize.y / 5.f + 80.f};
        m_buttons.emplaceBack(font, 4u).position = {displaySize.x / 2.f - 100.f, displaySize.y / 5.f + 80.f};

        // D-pad mapped to PovX/PovY
        m_joysticks.emplaceBack(font, sf::Joystick::Axis::PovX, sf::Joystick::Axis::PovY)
            .position = {displaySize.x / 2.f - 250.f, displaySize.y / 2.f - 50.f};

        // Back / Start / Guide
        m_buttons.emplaceBack(font, 8u).position  = {displaySize.x / 2.f - 20.f, displaySize.y / 2.f};
        m_buttons.emplaceBack(font, 9u).position  = {displaySize.x / 2.f + 20.f, displaySize.y / 2.f};
        m_buttons.emplaceBack(font, 12u).position = {displaySize.x / 2.f, displaySize.y / 2.f + 30.f};
    }

    void handleEvent(const sf::Event& event)
    {
        if (const auto* pressed = event.getIf<sf::Event::JoystickButtonPressed>())
            setButtonPressed(pressed->button, true);
        else if (const auto* released = event.getIf<sf::Event::JoystickButtonReleased>())
            setButtonPressed(released->button, false);
        else if (const auto* moved = event.getIf<sf::Event::JoystickMoved>())
            setAxis(moved->axis, moved->position);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        for (const auto& joystick : m_joysticks)
            target.draw(joystick, states);

        for (const auto& trigger : m_triggers)
            target.draw(trigger, states);

        for (const auto& button : m_buttons)
            target.draw(button, states);

        target.draw(m_label, states);
    }

private:
    void setButtonPressed(unsigned int button, bool pressed)
    {
        for (auto& b : m_buttons)
            if (b.getButton() == button)
            {
                b.setPressed(pressed);
                return;
            }

        for (auto& j : m_joysticks)
            if (j.getButton().hasValue() && *j.getButton() == button)
            {
                j.setPressed(pressed);
                return;
            }
    }

    void setAxis(sf::Joystick::Axis axis, float value)
    {
        for (auto& j : m_joysticks)
        {
            if (j.getXAxis() == axis)
            {
                j.setX(value);
                return;
            }

            if (j.getYAxis() == axis)
            {
                j.setY(value);
                return;
            }
        }

        for (auto& t : m_triggers)
            if (t.getAxis() == axis)
            {
                t.setValue(value);
                return;
            }
    }

    sf::base::Vector<JoystickDisplay> m_joysticks;
    sf::base::Vector<AxisDisplay>     m_triggers;
    sf::base::Vector<ButtonDisplay>   m_buttons;
    sf::Text                          m_label;
};


////////////////////////////////////////////////////////////
// Generic display: a row of axes and two rows of buttons,
// for joysticks that do not match the gamepad layout
////////////////////////////////////////////////////////////
class GenericDisplay : public sf::Transformable
{
public:
    GenericDisplay(const sf::Font& font, unsigned int id, const sf::base::String& name) :
        m_label(font,
                {
                    .string        = sf::base::toString(id) + ": " + name,
                    .characterSize = 20u,
                })
    {
        // A row of axes
        constexpr float axisSpacing = displaySize.x / static_cast<float>(sf::Joystick::AxisCount + 1u);
        for (unsigned int i = 0u; i < sf::Joystick::AxisCount; ++i)
        {
            m_axisDisplays.emplaceBack(font, static_cast<sf::Joystick::Axis>(i));
            m_axisDisplays.back().position = {axisSpacing + static_cast<float>(i) * axisSpacing, displaySize.y * 0.25f};
        }

        // Two rows of buttons
        constexpr float buttonSpacing = displaySize.x / (static_cast<float>(sf::Joystick::ButtonCount) / 2.f + 1.f);
        for (unsigned int i = 0u; i < sf::Joystick::ButtonCount; ++i)
        {
            m_buttonDisplays.emplaceBack(font, i);
            m_buttonDisplays.back().position = {buttonSpacing + static_cast<float>(i) / 2.f * buttonSpacing,
                                                displaySize.y * 0.75f + static_cast<float>(i % 2u) * 50.f};
        }
    }

    void handleEvent(const sf::Event& event)
    {
        if (const auto* pressed = event.getIf<sf::Event::JoystickButtonPressed>())
        {
            for (auto& b : m_buttonDisplays)
                if (b.getButton() == pressed->button)
                {
                    b.setPressed(true);
                    return;
                }
        }
        else if (const auto* released = event.getIf<sf::Event::JoystickButtonReleased>())
        {
            for (auto& b : m_buttonDisplays)
                if (b.getButton() == released->button)
                {
                    b.setPressed(false);
                    return;
                }
        }
        else if (const auto* moved = event.getIf<sf::Event::JoystickMoved>())
        {
            for (auto& a : m_axisDisplays)
                if (a.getAxis() == moved->axis)
                {
                    a.setValue(moved->position);
                    return;
                }
        }
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        target.draw(m_label, states);

        for (const auto& axis : m_axisDisplays)
            target.draw(axis, states);

        for (const auto& button : m_buttonDisplays)
            target.draw(button, states);
    }

private:
    sf::Text                        m_label;
    sf::base::Vector<AxisDisplay>   m_axisDisplays;
    sf::base::Vector<ButtonDisplay> m_buttonDisplays;
};


////////////////////////////////////////////////////////////
// Get the joystick name, or an empty string if not connected
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::String getJoystickName(unsigned int id)
{
    if (const auto query = sf::Joystick::query(id))
        return sf::base::String{query->getName().asBytes()};

    return {};
}

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    // Create the window of the application (+200 vertical for header / instructions)
    constexpr sf::Vec2f windowSize{displaySize.x, displaySize.y + 200.f};

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size      = windowSize.toVec2u(),
                          .title     = "Joystick",
                          .resizable = true,
                          .vsync     = true,
                      })
                      .value();

    auto windowView = window.computeView();

    // Open the text font
    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    // Index indicators along the top: red if disconnected, green if connected
    constexpr float spacer = displaySize.x / static_cast<float>(sf::Joystick::MaxCount + 1u);

    sf::base::Vector<sf::Text> indexIndicators;
    indexIndicators.reserve(sf::Joystick::MaxCount);

    for (unsigned int i = 0u; i < sf::Joystick::MaxCount; ++i)
    {
        sf::Text indicator(font,
                           {
                               .position      = {spacer + static_cast<float>(i) * spacer, 60.f},
                               .string        = sf::base::toString(i),
                               .characterSize = 20u,
                           });

        const sf::Rect2f bounds = indicator.getLocalBounds();
        indicator.origin        = bounds.position + bounds.size / 2.f;

        indexIndicators.pushBack(SFML_BASE_MOVE(indicator));
    }

    unsigned int joystickIndex = 0u;

    const sf::Vec2f selectedSize = indexIndicators[joystickIndex].getGlobalBounds().size * 2.f;

    const auto updateIndicators = [&]
    {
        for (unsigned int i = 0u; i < indexIndicators.size(); ++i)
            indexIndicators[i].setFillColor(sf::Joystick::query(i).hasValue() ? sf::Color::Green : sf::Color::Red);
    };
    updateIndicators();

    // Instruction text
    sf::Text instructions(font,
                          {
                              .position = {displaySize.x / 2.f, 10.f},
                              .string = "Press 0-7 to select joystick, tab to switch between gamepad/generic displays",
                              .characterSize = 20u,
                          });
    {
        const sf::Rect2f bounds = instructions.getLocalBounds();
        instructions.origin     = bounds.position + bounds.size / 2.f;
    }

    // Text shown when the selected joystick is not connected
    sf::Text notConnected(font,
                          {
                              .position      = {displaySize.x / 2.f, displaySize.y / 2.f + 150.f},
                              .string        = "Joystick not connected",
                              .characterSize = 30u,
                          });
    {
        const sf::Rect2f bounds = notConnected.getLocalBounds();
        notConnected.origin     = bounds.position + bounds.size / 2.f;
    }

    // The actual displays - toggle between generic and gamepad with Tab
    bool                               showingGamepad = true;
    sf::base::Optional<GamepadDisplay> gamepadDisplay;
    sf::base::Optional<GenericDisplay> genericDisplay;

    constexpr float displayOffset = 150.f;

    const auto rebuildDisplays = [&]
    {
        const auto name = getJoystickName(joystickIndex);

        gamepadDisplay.emplace(font, joystickIndex, name);
        gamepadDisplay->position = {0.f, displayOffset};

        genericDisplay.emplace(font, joystickIndex, name);
        genericDisplay->position = {0.f, displayOffset};
    };
    rebuildDisplays();

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleNonScalingResize(*event, windowSize, windowView))
                continue;

            if (event->is<sf::Event::JoystickConnected>() || event->is<sf::Event::JoystickDisconnected>())
            {
                rebuildDisplays();
                updateIndicators();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code >= sf::Keyboard::Key::Num0 && keyPressed->code <= sf::Keyboard::Key::Num7)
                {
                    joystickIndex = static_cast<unsigned int>(keyPressed->code) -
                                    static_cast<unsigned int>(sf::Keyboard::Key::Num0);
                    rebuildDisplays();
                    updateIndicators();
                }
                else if (keyPressed->code == sf::Keyboard::Key::Tab)
                {
                    showingGamepad = !showingGamepad;
                }
            }
            else
            {
                // Forward axis/button events only when they target the currently selected joystick
                const auto matchesSelected = [&](unsigned int id) { return id == joystickIndex; };

                bool forward = false;
                if (const auto* pressed = event->getIf<sf::Event::JoystickButtonPressed>())
                    forward = matchesSelected(pressed->joystickId);
                else if (const auto* released = event->getIf<sf::Event::JoystickButtonReleased>())
                    forward = matchesSelected(released->joystickId);
                else if (const auto* moved = event->getIf<sf::Event::JoystickMoved>())
                    forward = matchesSelected(moved->joystickId);

                if (forward)
                {
                    gamepadDisplay->handleEvent(*event);
                    genericDisplay->handleEvent(*event);
                }
            }
        }

        // Render frame
        window.clear();

        const auto drawCtx = window.withRenderStates({.view = windowView});

        drawCtx.draw(instructions);

        drawCtx.draw(sf::RectangleShapeData{
            .position         = indexIndicators[joystickIndex].position,
            .origin           = selectedSize / 2.f,
            .fillColor        = sf::Color::Transparent,
            .outlineColor     = sf::Color::White,
            .outlineThickness = 3.f,
            .size             = selectedSize,
        });

        for (const auto& indicator : indexIndicators)
            drawCtx.draw(indicator);

        if (!sf::Joystick::query(joystickIndex).hasValue())
            drawCtx.draw(notConnected);
        else if (showingGamepad)
            drawCtx.draw(*gamepadDisplay);
        else
            drawCtx.draw(*genericDisplay);

        window.display();
    }
}
