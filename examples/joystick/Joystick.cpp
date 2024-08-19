////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Joystick.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"

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
        for (unsigned int j = 0; j < sf::Joystick::MaxAxisCount; ++j)
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

    // Create the window of the application
    sf::RenderWindow window(graphicsContext, {.size{400, 775}, .title = "Joystick", .style = sf::Style::Close});
    window.setVerticalSyncEnabled(true);

    // Set up our string conversion parameters
    sstr.precision(2);
    sstr.setf(std::ios::fixed | std::ios::boolalpha);

    // Set up our joystick identification sf::Text objects
    {
        auto [it, success]   = texts.emplace("ID", JoystickTexts{{font, "<Not Connected>"}, {font}});
        auto& [label, value] = it->second;
        label.setPosition({5.f, 5.f});
        value.setPosition({80.f, 5.f});
    }

    // Set up our threshold sf::Text objects
    sstr.str("");
    sstr << threshold << "  (Change with up/down arrow keys)";
    {
        auto [it, success]   = texts.emplace("Threshold", JoystickTexts{{font, "Threshold:"}, {font, sstr.str()}});
        auto& [label, value] = it->second;
        label.setPosition({5.f, 5.f + 2 * font.getLineSpacing(14)});
        value.setPosition({80.f, 5.f + 2 * font.getLineSpacing(14)});
    }

    // Set up our label-value sf::Text objects
    for (unsigned int i = 0; i < sf::Joystick::MaxAxisCount; ++i)
    {
        auto [it, success]   = texts.emplace(axisLabels[i], JoystickTexts{{font, axisLabels[i] + ":"}, {font, "N/A"}});
        auto& [label, value] = it->second;
        label.setPosition({5.f, 5.f + (static_cast<float>(i + 4) * font.getLineSpacing(14))});
        value.setPosition({80.f, 5.f + (static_cast<float>(i + 4) * font.getLineSpacing(14))});
    }

    for (unsigned int i = 0; i < sf::Joystick::MaxButtonCount; ++i)
    {
        sstr.str("");
        sstr << "Button " << i;

        auto [it, success]   = texts.emplace(sstr.str(), JoystickTexts{{font, sstr.str() + ":"}, {font, "N/A"}});
        auto& [label, value] = it->second;
        label.setPosition({5.f, 5.f + (static_cast<float>(sf::Joystick::MaxAxisCount + i + 4) * font.getLineSpacing(14))});
        value.setPosition({80.f, 5.f + (static_cast<float>(sf::Joystick::MaxAxisCount + i + 4) * font.getLineSpacing(14))});
    }

    for (auto& [label, joystickObject] : texts)
    {
        joystickObject.label.setCharacterSize(14);
        joystickObject.value.setCharacterSize(14);
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

                sstr.str("");
                sstr << threshold << "  (Change with up/down arrow keys)";

                texts.at("Threshold").value.setString(sstr.str());
            }
        }

        // Update threshold if the user wants to change it
        float newThreshold = threshold;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            newThreshold += 0.1f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            newThreshold -= 0.1f;

        newThreshold = std::clamp(newThreshold, 0.1f, 100.0f);

        if (newThreshold != threshold)
        {
            threshold = newThreshold;
            window.setJoystickThreshold(threshold);

            sstr.str("");
            sstr << threshold << "  (Change with up/down arrow keys)";

            texts.at("Threshold").value.setString(sstr.str());
        }

        // Clear the window
        window.clear();

        // Draw the label-value sf::Text objects
        for (const auto& [label, joystickObject] : texts)
        {
            window.draw(joystickObject.label);
            window.draw(joystickObject.value);
        }

        // Display things on screen
        window.display();
    }
}
