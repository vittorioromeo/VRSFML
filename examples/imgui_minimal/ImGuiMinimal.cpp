#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Clock.hpp"

#include "SFML/Base/Optional.hpp"

#include <imgui.h>

int main()
{
    sf::GraphicsContext     graphicsContext;
    sf::ImGui::ImGuiContext imGuiContext(graphicsContext);

    sf::RenderWindow window(graphicsContext, {.size{1024u, 768u}, .title = "ImGui + SFML = <3", .style = sf::Style::Resize});
    window.setFramerateLimit(60);

    if (!imGuiContext.init(window))
        return -1;

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    sf::Clock deltaClock;

    while (true)
    {
        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;
        }

        imGuiContext.update(window, deltaClock.restart());

        ImGui::ShowDemoWindow();

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear();
        window.draw(shape, nullptr /* texture */);
        imGuiContext.render(window);
        window.display();
    }
}
