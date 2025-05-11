////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/String.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

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

    // Create the main window
    constexpr sf::Vec2f windowSize{800.f, 600.f};

    auto window = makeDPIScaledRenderWindow({
        .size      = windowSize.toVec2u(),
        .title     = "SFML Raw Mouse Input",
        .resizable = true,
        .vsync     = true,
    });

    // Open the application font and pass it to the Effect class
    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    // Create the mouse position and mouse raw movement texts
    sf::Text mousePosition(font, {.position = {400.f, 300.f}, .characterSize = 20u, .fillColor = sf::Color::White});
    sf::Text mouseRawMovement(font, {.characterSize = 20u, .fillColor = sf::Color::White});

    sf::base::Vector<std::string> log;

    while (true)
    {
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, window))
                continue;

            static const auto vec2ToString = [](const sf::Vec2i vec2)
            { return '(' + std::to_string(vec2.x) + ", " + std::to_string(vec2.y) + ')'; };

            if (const auto* const mouseMoved = event->getIf<sf::Event::MouseMoved>())
                mousePosition.setString("Mouse Position: " + vec2ToString(mouseMoved->position));

            if (const auto* const mouseMovedRaw = event->getIf<sf::Event::MouseMovedRaw>())
            {
                log.emplaceBack("Mouse Movement: " + vec2ToString(mouseMovedRaw->delta));

                if (log.size() > 24u)
                    log.erase(log.begin());
            }
        }

        window.clear();
        window.draw(mousePosition);

        for (sf::base::SizeT i = 0u; i < log.size(); ++i)
        {
            mouseRawMovement.position = {50.f, static_cast<float>(i * 20) + 50.f};
            mouseRawMovement.setString(log[i]);
            window.draw(mouseRawMovement);
        }

        window.display();
    }
}
