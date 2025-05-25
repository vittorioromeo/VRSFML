////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Joystick.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Clamp.hpp"

#include "ExampleUtils.hpp"

#include <string>


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    // Open the text font
    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    constexpr auto characterSize   = 16u;
    const auto     fontLineSpacing = font.getLineSpacing(characterSize);

    sf::OutStringStream oss;

    // Set up our string conversion parameters
    oss.setPrecision(2);
    oss.setFormatFlags(sf::FormatFlags::fixed | sf::FormatFlags::boolalpha);

    const auto toStr = [&](const auto&... xs)
    {
        oss.setStr("");
        (oss << ... << xs);
        return oss.to<sf::String>();
    };

    float threshold = 0.1f;

    // Axes labels in as strings
    const sf::base::Array<std::string, 8> axisLabels = {"X", "Y", "Z", "R", "U", "V", "PovX", "PovY"};

    constexpr sf::Vec2f windowSize{400.f, 775.f};

    // Create the window of the application
    auto window = makeDPIScaledRenderWindow({
        .size      = windowSize.toVec2u(),
        .title     = "Joystick",
        .resizable = true,
        .vsync     = true,
    });

    sf::Clock clock;

    sf::base::Vector<sf::String> eventLog;

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            // if (handleAspectRatioAwareResize(*event, windowSize, window))
            //     continue;

            if (const auto* joystickConnected = event->getIf<sf::Event::JoystickConnected>())
                eventLog.pushBack(toStr("[Joystick ", joystickConnected->joystickId, "]: connected"));
            else if (const auto* joystickDisconnected = event->getIf<sf::Event::JoystickDisconnected>())
                eventLog.pushBack(toStr("[Joystick ", joystickDisconnected->joystickId, "]: disconnected"));
            else if (const auto* joystickMoved = event->getIf<sf::Event::JoystickMoved>())
                eventLog.pushBack(toStr("[Joystick ",
                                        joystickMoved->joystickId,
                                        "] moved: axis ",
                                        axisLabels[static_cast<unsigned int>(joystickMoved->axis)],
                                        " -> ",
                                        joystickMoved->position));
            else if (const auto* joystickButtonPressed = event->getIf<sf::Event::JoystickButtonPressed>())
                eventLog.pushBack(
                    toStr("[Joystick ", joystickButtonPressed->joystickId, "] button pressed: ", joystickButtonPressed->button));
            else if (const auto* joystickButtonReleased = event->getIf<sf::Event::JoystickButtonReleased>())
                eventLog.pushBack(toStr("[Joystick ",
                                        joystickButtonReleased->joystickId,
                                        "] button released: ",
                                        joystickButtonReleased->button));
        }

        while (eventLog.size() > 10u)
            eventLog.erase(eventLog.begin());

        // Update the threshold
        threshold = sf::base::clamp(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)     ? threshold + 0.1f
                                    : sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ? threshold - 0.1f
                                                                                          : threshold,
                                    0.1f,
                                    100.f);

        window.setJoystickThreshold(threshold);

        clock.restart();

        unsigned int connectedJoystickCount = 0u;
        for (unsigned int i = 0; i < sf::Joystick::MaxCount; ++i)
            if (sf::Joystick::query(i))
                ++connectedJoystickCount;

        window.setSize({400u * sf::base::max(1u, connectedJoystickCount), 775u});
        window.setView(sf::View{{775.f / 2.f, 775.f / 2.f}, {775.f, 775.f}});

        float xOffset      = 0.f;
        float yOffset      = 0.f;
        float yEventOffset = 0.f;

        const auto drawLabelValue = [&](const sf::String& label, const sf::String& value)
        {
            window.draw(font,
                        {.position         = {5.f + 320.f * xOffset, 50.f + yOffset * fontLineSpacing},
                         .string           = label + ":",
                         .characterSize    = characterSize,
                         .outlineColor     = sf::Color::Blue,
                         .outlineThickness = 0.5f});

            window.draw(font,
                        {.position      = {80.f + 320.f * xOffset, 50.f + yOffset * fontLineSpacing},
                         .string        = value,
                         .characterSize = characterSize});

            yOffset += 1.f;
        };

        // Clear the window
        window.clear();

        window.draw(font,
                    {.position         = {5.f, 5.f},
                     .string           = toStr("Threshold: ", threshold, "  (Change with up/down arrow keys)"),
                     .characterSize    = characterSize,
                     .outlineColor     = sf::Color::Blue,
                     .outlineThickness = 0.5f});

        for (const auto& eventStr : eventLog)
        {
            window.draw(font,
                        {.position         = {5.f, 500.f + yEventOffset * fontLineSpacing},
                         .string           = eventStr,
                         .characterSize    = characterSize,
                         .outlineColor     = sf::Color::Blue,
                         .outlineThickness = 0.5f});

            ++yEventOffset;
        }

        for (unsigned int i = 0u; i < sf::Joystick::MaxCount; ++i)
        {
            auto query = sf::Joystick::query(i);
            if (!query.hasValue())
                continue;

            // Draw joystick identification
            drawLabelValue(toStr("Joystick ", query->getIndex()), query->getName());

            // Draw joystick axes
            for (unsigned int j = 0u; j < sf::Joystick::AxisCount; ++j)
                if (query->hasAxis(static_cast<sf::Joystick::Axis>(j)))
                    drawLabelValue(axisLabels[j], toStr(query->getAxisPosition(static_cast<sf::Joystick::Axis>(j))));

            // Draw joystick buttons
            for (unsigned int j = 0u; j < query->getButtonCount(); ++j)
                drawLabelValue(toStr("Button ", j), toStr(query->isButtonPressed(j)));

            xOffset += 1.f;
            yOffset = 0.f;
        }

        // Display things on screen
        window.display();
    }
}
