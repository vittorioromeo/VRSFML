#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Clock.hpp"

#include "SFML/Base/Optional.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

int main()
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    sf::RenderWindow window({
        .size  = {1280u, 720u},
        .title = "ImGui + SFML = <3",
        .vsync = true,
    });

    sf::ImGuiContext imGuiContext;

    sf::base::Optional<sf::RenderWindow> childWindow(sf::base::inPlace,
                                                     sf::RenderWindow::Settings{
                                                         .size  = {640u, 480u},
                                                         .title = "ImGui-SFML Child window",
                                                         .vsync = true,
                                                     });

    sf::base::Optional<sf::ImGuiContext> childImGuiContext{sf::base::inPlace};

    sf::Clock deltaClock;
    while (true)
    {
        // Main window event processing
        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;
        }

        // Update
        const sf::Time dt = deltaClock.restart();

        imGuiContext.update(window, dt);

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        ImGui::ShowDemoWindow();

        // Main window drawing
        const sf::CircleShape shape{{.fillColor = sf::Color::Green, .radius = 100.f}};

        window.clear();
        window.draw(shape);
        imGuiContext.render(window);
        window.display();

        const auto processChildWindow = [&](sf::RenderWindow& childWindowRef)
        {
            while (const sf::base::Optional event = childWindowRef.pollEvent())
            {
                childImGuiContext->processEvent(childWindowRef, *event);

                if (event->is<sf::Event::Closed>())
                {
                    childImGuiContext.reset();
                    childWindow.reset();
                    return;
                }
            }

            childImGuiContext->update(childWindowRef, dt);

            ImGui::Begin("Works in a second window!");
            ImGui::Button("Example button");
            ImGui::End();

            const sf::CircleShape shape2{{.fillColor = sf::Color::Red, .radius = 50.f}};

            childWindowRef.clear();
            childWindowRef.draw(shape2);
            childImGuiContext->render(childWindowRef);
            childWindowRef.display();
        };

        // Child window event processing
        if (childWindow.hasValue())
            processChildWindow(*childWindow);
    }
}
