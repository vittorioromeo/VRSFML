#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <StringifyOptionalUtil.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

#include <iostream>


TEST_CASE("[Graphics] MultiContext" * doctest::skip(skipDisplayTests))
{
    sf::Vertex   vertices[]{{.position = {0.f, 0.f}}};
    unsigned int indices[] = {0};

    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Test")
    {
        sf::RenderWindow wnd{{.size = {1024u, 1024u}, .title = "Window A"}};
        wnd.drawIndexedVertices(vertices, 1, indices, 1, sf::PrimitiveType::Points);

        wnd.display();

        auto rt1 = sf::RenderTexture::create({1024u, 1024u}).value();
        wnd.drawIndexedVertices(vertices, 1, indices, 1, sf::PrimitiveType::Points);

        rt1.display();
    }

    SECTION("Test2")
    {
        sf::base::Optional<sf::RenderWindow>  optWnd;
        sf::base::Optional<sf::RenderTexture> optRT0;

        for (int i = 0; i < 2; ++i)
        {
            optRT0.emplace(sf::RenderTexture::create({1024u, 1024u}).value());
            optWnd.emplace(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0.reset();
            optRT0.emplace(sf::RenderTexture::create({1024u, 1024u}).value());
            optWnd.emplace(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0->drawIndexedVertices(vertices, 1, indices, 1, sf::PrimitiveType::Points);
            optRT0->display();
        }
    }

    SECTION("Test3")
    {
        sf::base::Optional<sf::RenderWindow> optWnd;

        auto rt = sf::RenderTexture::create({1024u, 1024u});

        optWnd.emplace(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices(vertices, 1, indices, 1, sf::PrimitiveType::Points);
        rt->display();

        optWnd->drawIndexedVertices(vertices, 1, indices, 1, sf::PrimitiveType::Points);
        optWnd.emplace(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices(vertices, 1, indices, 1, sf::PrimitiveType::Points);
        rt->display();
    }

    SECTION("Test4")
    {
        sf::RenderWindow window({.size{1024u, 768u}, .title = "Window C"});

        auto baseRenderTexture = sf::RenderTexture::create({100u, 100u});
        auto leftInnerRT       = sf::RenderTexture::create({100u, 100u});

        leftInnerRT->draw(sf::RectangleShapeData{});
        leftInnerRT->display();
    }
}
