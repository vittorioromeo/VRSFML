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
#include <unordered_map>

#include <cstdlib>


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

    constexpr auto characterSize   = 14u;
    const auto     fontLineSpacing = font.getLineSpacing(characterSize);

    struct JoystickTexts
    {
        sf::Text label;
        sf::Text value;
    };

    std::unordered_map<std::string, JoystickTexts> texts;

    sf::OutStringStream oss;
    float               threshold = 0.1f;

    // Axes labels in as strings
    const sf::base::Array<std::string, 8> axisLabels = {"X", "Y", "Z", "R", "U", "V", "PovX", "PovY"};

    // Helper to set text entries to a specified value
    const auto set = [&](const std::string& label, const auto& value)
    {
        oss.setStr("");
        oss << value;

        texts.at(label).value.setString(oss.to<sf::String>());
    };

    // Update joystick identification
    const auto updateIdentification = [&](const sf::Joystick::Query& query)
    {
        oss.setStr("");
        oss << "Joystick " << query.getIndex() << ":";

        auto& [label, value] = texts.at("ID");
        label.setString(oss.to<sf::String>());
        value.setString(query.getName());
    };

    // Update joystick axes
    const auto updateAxes = [&](const sf::Joystick::Query& query)
    {
        for (unsigned int j = 0; j < sf::Joystick::AxisCount; ++j)
            if (query.hasAxis(static_cast<sf::Joystick::Axis>(j)))
                set(axisLabels[j], query.getAxisPosition(static_cast<sf::Joystick::Axis>(j)));
    };

    // Update joystick buttons
    const auto updateButtons = [&](const sf::Joystick::Query& query)
    {
        for (unsigned int j = 0; j < query.getButtonCount(); ++j)
        {
            oss.setStr("");
            oss << "Button " << j;

            set(oss.to<std::string>(), query.isButtonPressed(j));
        }
    };

    // Update the label-value sf::Text objects based on the current joystick state
    const auto updateValues = [&](unsigned int index)
    {
        const auto query = sf::Joystick::query(index);
        if (!query.hasValue())
            return false;

        updateIdentification(*query);
        updateAxes(*query);
        updateButtons(*query);

        return true;
    };

    const auto updateThresholdText = [&]
    {
        oss.setStr("");
        oss << threshold << "  (Change with up/down arrow keys)";

        texts.at("Threshold").value.setString(oss.to<sf::String>());
    };

    constexpr sf::Vec2f windowSize{400.f, 775.f};

    // Create the window of the application
    auto window = makeDPIScaledRenderWindow({
        .size      = windowSize.toVec2u(),
        .title     = "Joystick",
        .resizable = true,
        .vsync     = true,
    });

    // Set up our string conversion parameters
    oss.setPrecision(2);
    oss.setFormatFlags(sf::FormatFlags::fixed | sf::FormatFlags::boolalpha);

    // Utility function to create text objects
    const auto emplaceTexts = [&](const std::string& labelStr, const std::string& valueStr, const float yOffset) -> auto&
    {
        auto [it, success] = texts.try_emplace(labelStr,
                                               sf::Text{font,
                                                        {.position         = {5.f, 5.f + yOffset * fontLineSpacing},
                                                         .string           = labelStr + ":",
                                                         .characterSize    = characterSize,
                                                         .outlineColor     = sf::Color::Blue,
                                                         .outlineThickness = 0.5f}},
                                               sf::Text{font,
                                                        {.position      = {80.f, 5.f + yOffset * fontLineSpacing},
                                                         .string        = valueStr,
                                                         .characterSize = characterSize}});

        return it->second;
    };

    // Set up our joystick identification `sf::Text` objects
    {
        auto& [label, value] = emplaceTexts("ID", "", 0.f);
        label.setString("<Not Connected>");
    }

    // Set up our threshold sf::Text objects
    emplaceTexts("Threshold", "", 2.f);
    updateThresholdText();

    // Set up our label-value sf::Text objects
    for (unsigned int i = 0; i < sf::Joystick::AxisCount; ++i)
        emplaceTexts(axisLabels[i], "N/A", static_cast<float>(i + 4));

    for (unsigned int i = 0; i < sf::Joystick::ButtonCount; ++i)
    {
        oss.setStr("");
        oss << "Button " << i;

        emplaceTexts(oss.to<std::string>(), "N/A", static_cast<float>(sf::Joystick::AxisCount + i + 4));
    }

    // Update initially displayed joystick values if a joystick is already connected on startup
    for (unsigned int i = 0; i < sf::Joystick::MaxCount; ++i)
        if (updateValues(i))
            break;

    // Create drawable batch to optimize rendering
    sf::CPUDrawableBatch drawableBatch;

    sf::Clock clock;

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, window))
                continue;

            if (const auto* joystickButtonPressed = event->getIf<sf::Event::JoystickButtonPressed>())
                updateValues(joystickButtonPressed->joystickId);
            else if (const auto* joystickButtonReleased = event->getIf<sf::Event::JoystickButtonReleased>())
                updateValues(joystickButtonReleased->joystickId);
            else if (const auto* joystickMoved = event->getIf<sf::Event::JoystickMoved>())
                updateValues(joystickMoved->joystickId);
            else if (const auto* joystickConnected = event->getIf<sf::Event::JoystickConnected>())
            {
                sf::cOut() << "Connected joystick: " << joystickConnected->joystickId << '\n';
                updateValues(joystickConnected->joystickId);
            }
            else if (const auto* joystickDisconnected = event->getIf<sf::Event::JoystickDisconnected>())
            {
                sf::cOut() << "Disconnected joystick: " << joystickDisconnected->joystickId << '\n';

                // Reset displayed joystick values to empty
                for (auto& [label, joystickObject] : texts)
                    joystickObject.value.setString("N/A");

                auto& [label, value] = texts.at("ID");
                label.setString("<Not Connected>");
                value.setString("");

                updateThresholdText();
            }
        }

        // Update threshold if the user wants to change it
        // clang-format off
        const float newThreshold = sf::base::clamp(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)   ? threshold + 0.1f
                                            : sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ? threshold - 0.1f
                                            : threshold,
                                        0.1f, 100.f);
        // clang-format on

        if (newThreshold != threshold)
        {
            threshold = newThreshold;
            window.setJoystickThreshold(threshold);

            updateThresholdText();
        }

        clock.restart();

        // Clear the window
        window.clear();

        // Draw the label-value sf::Text objects
        {
            drawableBatch.clear();

            for (const auto& [label, joystickObject] : texts)
            {
                drawableBatch.add(joystickObject.label);
                drawableBatch.add(joystickObject.value);
            }

            window.draw(drawableBatch, {.texture = &font.getTexture()});
        }

        // Display things on screen
        window.display();
    }
}
