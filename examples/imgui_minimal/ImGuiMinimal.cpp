#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Clock.hpp"

#include "SFML/Base/Optional.hpp"

#include <imgui.h>

int main()
{
    sf::GraphicsContext     graphicsContext;
    sf::ImGui::ImGuiContext imGuiContext(graphicsContext);

    sf::RenderWindow window(graphicsContext, {.size{1024u, 768u}, .title = "ImGui + SFML = <3", .vsync = true});

    if (!imGuiContext.init(window))
        return -1;

    const sf::CircleShape shape{{.fillColor = sf::Color::Green, .radius = 100.f}};

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
