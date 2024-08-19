////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Joystick.hpp"

#include "SFML/Graphics/CoordinateType.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Path.hpp"

#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstdlib>


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    sf::GraphicsContext graphicsContext;

    // Open the text font
    const auto font = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();

    constexpr auto characterSize   = 14u;
    const auto     fontLineSpacing = font.getLineSpacing(characterSize);

    struct JoystickTexts
    {
        sf::Text label;
        sf::Text value;
    };

    std::unordered_map<std::string, JoystickTexts> texts;

    std::ostringstream sstr;
    float              threshold = 0.1f;

    // Axes labels in as strings
    const std::array<std::string, 8> axisLabels = {"X", "Y", "Z", "R", "U", "V", "PovX", "PovY"};

    // Helper to set text entries to a specified value
    const auto set = [&](const std::string& label, const auto& value)
    {
        sstr.str("");
        sstr << value;

        texts.at(label).value.setString(sstr.str());
    };

    // Update joystick identification
    const auto updateIdentification = [&](const sf::Joystick::Query& query)
    {
        sstr.str("");
        sstr << "Joystick " << query.getIndex() << ":";

        auto& [label, value] = texts.at("ID");
        label.setString(sstr.str());
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
            sstr.str("");
            sstr << "Button " << j;

            set(sstr.str(), query.isButtonPressed(j));
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
        sstr.str("");
        sstr << threshold << "  (Change with up/down arrow keys)";

        texts.at("Threshold").value.setString(sstr.str());
    };

    // Create the window of the application
    sf::RenderWindow window(graphicsContext, {.size{400, 775}, .title = "Joystick", .style = sf::Style::Close});
    window.setVerticalSyncEnabled(true);

    // Set up our string conversion parameters
    sstr.precision(2);
    sstr.setf(std::ios::fixed | std::ios::boolalpha);

    // TODO P0:
    const auto emplaceTexts = [&](const std::string& labelStr, const std::string& valueStr, const float yOffset) -> auto&
    {
        auto [it, success] = texts.emplace(labelStr,
                                           JoystickTexts{{font, labelStr + ":", characterSize},
                                                         {font, valueStr, characterSize}});

        auto& [label, value] = it->second;
        label.setPosition({5.f, 5.f + yOffset * fontLineSpacing});
        value.setPosition({80.f, 5.f + yOffset * fontLineSpacing});

        return it->second;
    };

    // Set up our joystick identification sf::Text objects
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
        sstr.str("");
        sstr << "Button " << i;

        emplaceTexts(sstr.str(), "N/A", static_cast<float>(sf::Joystick::AxisCount + i + 4));
    }

    // Update initially displayed joystick values if a joystick is already connected on startup
    for (unsigned int i = 0; i < sf::Joystick::MaxCount; ++i)
        if (updateValues(i))
            break;

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;

            if (const auto* joystickButtonPressed = event->getIf<sf::Event::JoystickButtonPressed>())
            {
                updateValues(joystickButtonPressed->joystickId);
            }
            else if (const auto* joystickButtonReleased = event->getIf<sf::Event::JoystickButtonReleased>())
            {
                updateValues(joystickButtonReleased->joystickId);
            }
            else if (const auto* joystickMoved = event->getIf<sf::Event::JoystickMoved>())
            {
                updateValues(joystickMoved->joystickId);
            }
            else if (const auto* joystickConnected = event->getIf<sf::Event::JoystickConnected>())
            {
                updateValues(joystickConnected->joystickId);
            }
            else if (event->is<sf::Event::JoystickDisconnected>())
            {
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
        const float newThreshold = std::clamp(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)   ? threshold + 0.1f
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

        // Clear the window
        window.clear();

        // Draw the label-value sf::Text objects
#if 0
        for (const auto& [label, joystickObject] : texts)
        {
            window.draw(joystickObject.label);
            window.draw(joystickObject.value);
        }
#else // TODO P0: batching experiments
        static std::vector<sf::Vertex> batch;
        batch.clear();

        const auto addToBatch = [&](const sf::Text& text)
        {
            const auto [data, size] = text.getVertices();
            const auto& transform   = text.getTransform();

            auto it = batch.insert(batch.end(), data, data + size);
            for (auto targetIt = it + static_cast<long long>(size); it != targetIt; ++it)
                it->position.setXY(transform * it->position.xy());
        };

        for (const auto& [label, joystickObject] : texts)
        {
            addToBatch(joystickObject.label);
            addToBatch(joystickObject.value);
        }

        sf::RenderStates states;

        states.texture        = &font.getTexture(characterSize);
        states.coordinateType = sf::CoordinateType::Pixels;

        window.draw(batch.data(), batch.size(), sf::PrimitiveType::Triangles, states);
#endif

        // Display things on screen
        window.display();
    }
}
