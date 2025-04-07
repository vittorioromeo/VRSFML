#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"

#include "SFML/Base/Optional.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

int main()
{
    auto                    graphicsContext = sf::GraphicsContext::create().value();
    sf::ImGui::ImGuiContext imGuiContext;

    sf::RenderWindow window({.size{1024u, 768u}, .title = "ImGui + SFML = <3", .vsync = true});

    if (!imGuiContext.init(window))
        return -1;

    const sf::CircleShape shape{{.fillColor = sf::Color::Green, .radius = 100.f}};


    const float width     = 128.f;
    const float height    = 64.f;
    const float halfWidth = width / 2.f;

    const sf::Vector2u size{static_cast<unsigned int>(width), static_cast<unsigned int>(height)};


    auto baseRenderTexture = sf::RenderTexture::create(size, {.antiAliasingLevel = 0, .sRgbCapable = true}).value();

    auto leftInnerRT = sf::RenderTexture::create(size, {.antiAliasingLevel = 4, .sRgbCapable = true}).value();

    const sf::Vertex leftVertexArray[6]{{{0.f, 0.f}, sf::Color::Red, {0.f, 0.f}},
                                        {{halfWidth, 0.f}, sf::Color::Red, {halfWidth, 0.f}},
                                        {{0.f, height}, sf::Color::Red, {0.f, height}},
                                        {{0.f, height}, sf::Color::Green, {0.f, height}},
                                        {{halfWidth, 0.f}, sf::Color::Green, {halfWidth, 0.f}},
                                        {{halfWidth, height}, sf::Color::Green, {halfWidth, height}}};

    leftInnerRT.clear();

    auto image   = sf::Image::create(size, sf::Color::White).value();
    auto texture = sf::Texture::loadFromImage(image).value();

    leftInnerRT.draw(texture);
    leftInnerRT.display();


    baseRenderTexture.clear();
    baseRenderTexture.draw(leftVertexArray, sf::PrimitiveType::Triangles, {.texture = &leftInnerRT.getTexture()});
    baseRenderTexture.display();

    sf::Clock    deltaClock;
    sf::Vector2i eventMousePosition;

    const auto modes = sf::VideoModeUtils::getFullscreenModes();
    for (const auto& mode : modes)
        sf::cOut() << "Fullscreen mode: " << mode.size.x << "x" << mode.size.y << '\n';

    while (true)
    {
        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (const auto* eResized = event->getIf<sf::Event::Resized>())
                sf::cOut() << "Resized event: " << eResized->size.x << ", " << eResized->size.y << '\n';

            if (const auto* eMouseMoved = event->getIf<sf::Event::MouseMoved>())
                eventMousePosition = eMouseMoved->position;
        }

        imGuiContext.update(window, deltaClock.restart());

        ImGui::ShowDemoWindow();

        ImGui::Begin("Hello, world!");

        const auto globalMousePosition = sf::Mouse::getPosition();
        ImGui::Text("Global mouse position: %d, %d", globalMousePosition.x, globalMousePosition.y);

        const auto relativeMousePosition = sf::Mouse::getPosition(window);
        ImGui::Text("Relative mouse position: %d, %d", relativeMousePosition.x, relativeMousePosition.y);

        ImGui::Text("Event mouse position: %d, %d", eventMousePosition.x, eventMousePosition.y);

        if (ImGui::Button("Set Window Size to 400x400"))
            window.setSize({400u, 400u});

        if (ImGui::Button("Set Window Size to 800x800"))
            window.setSize({800u, 800u});

        ImGui::Button("Look at this pretty button");
        imGuiContext.image(baseRenderTexture, size.toVector2f());

        ImGui::End();

        window.clear();
        window.draw(shape);
        imGuiContext.render(window);
        window.display();
    }
}
