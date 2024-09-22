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

    sf::RenderWindow window(graphicsContext, {.size{1280u, 720u}, .title = "ImGui + SFML = <3", .vsync = true});
    auto             imGuiWindowContext = imGuiContext.init(window).value();

    sf::base::Optional<sf::RenderWindow>
        childWindow(sf::base::inPlace,
                    graphicsContext,
                    sf::WindowSettings{.size{640u, 480u}, .title = "ImGui-SFML Child window", .vsync = true});

    if (!imGuiContext.init(*childWindow))
        return -1;

    sf::Clock deltaClock;
    while (true)
    {
        // Main window event processing
        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiWindowContext.processEvent(*event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;
        }

        // Update
        const sf::Time dt = deltaClock.restart();
        imGuiWindowContext.update(dt);

        // Add ImGui widgets in the first window
        imGuiWindowContext.activate();
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        ImGui::ShowDemoWindow();

        // Main window drawing
        sf::CircleShape shape(100.f);
        shape.setFillColor(sf::Color::Green);

        window.clear();
        window.draw(shape, /* texture */ nullptr);
        imGuiWindowContext.render();
        window.display();

        const auto processChildWindow = [&](sf::RenderWindow& childWindowRef)
        {
            while (const sf::base::Optional event = childWindowRef.pollEvent())
            {
                imGuiContext.processEvent(childWindowRef, *event);

                if (event->is<sf::Event::Closed>())
                {
                    imGuiContext.shutdown(childWindowRef);
                    childWindow.reset();
                    return;
                }
            }

            imGuiContext.update(childWindowRef, dt);

            imGuiContext.setCurrentWindow(childWindowRef);
            ImGui::Begin("Works in a second window!");
            ImGui::Button("Example button");
            ImGui::End();

            sf::CircleShape shape2(50.f);
            shape2.setFillColor(sf::Color::Red);

            childWindowRef.clear();
            childWindowRef.draw(shape2, /* texture */ nullptr);
            imGuiContext.render(childWindowRef);
            childWindowRef.display();
        };

        // Child window event processing
        if (childWindow.hasValue())
            processChildWindow(*childWindow);
    }
}
