#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <StringifyOptionalUtil.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Graphics] MultiContext" * doctest::skip(skipDisplayTests))
{
    sf::Vertex   vertices[]{{.position = {0.f, 0.f}}};
    unsigned int indices[] = {0};

    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Test")
    {
        auto wnd = sf::RenderWindow::create({.size = {1024u, 1024u}, .title = "Window A"}).value();
        wnd.drawIndexedVertices({
            .vertexData    = vertices,
            .vertexCount   = 1u,
            .indexData     = indices,
            .indexCount    = 1u,
            .primitiveType = sf::PrimitiveType::Points,
            .renderStates  = {.view = wnd.makeView()},
        });

        wnd.display();

        auto rt1 = sf::RenderTexture::create({1024u, 1024u}).value();
        wnd.drawIndexedVertices({
            .vertexData    = vertices,
            .vertexCount   = 1u,
            .indexData     = indices,
            .indexCount    = 1u,
            .primitiveType = sf::PrimitiveType::Points,
            .renderStates  = {.view = rt1.makeView()},
        });

        rt1.display();
    }

    SECTION("Test2")
    {
        sf::base::Optional<sf::RenderWindow>  optWnd;
        sf::base::Optional<sf::RenderTexture> optRT0;

        for (int i = 0; i < 2; ++i)
        {
            optRT0.emplace(sf::RenderTexture::create({1024u, 1024u}).value());
            optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0.reset();
            optRT0.emplace(sf::RenderTexture::create({1024u, 1024u}).value());
            optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0->drawIndexedVertices({
                .vertexData    = vertices,
                .vertexCount   = 1u,
                .indexData     = indices,
                .indexCount    = 1u,
                .primitiveType = sf::PrimitiveType::Points,
                .renderStates  = {.view = optWnd->makeView()},
            });
            optRT0->display();
        }
    }

    SECTION("Test3")
    {
        sf::base::Optional<sf::RenderWindow> optWnd;

        auto rt = sf::RenderTexture::create({1024u, 1024u});

        optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices({
            .vertexData    = vertices,
            .vertexCount   = 1u,
            .indexData     = indices,
            .indexCount    = 1u,
            .primitiveType = sf::PrimitiveType::Points,
            .renderStates  = {.view = rt->makeView()},
        });
        rt->display();

        optWnd->drawIndexedVertices({
            .vertexData    = vertices,
            .vertexCount   = 1u,
            .indexData     = indices,
            .indexCount    = 1u,
            .primitiveType = sf::PrimitiveType::Points,
            .renderStates  = {.view = optWnd->makeView()},
        });
        optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices({
            .vertexData    = vertices,
            .vertexCount   = 1u,
            .indexData     = indices,
            .indexCount    = 1u,
            .primitiveType = sf::PrimitiveType::Points,
            .renderStates  = {.view = rt->makeView()},
        });
        rt->display();
    }

    SECTION("Test4")
    {
        auto window = sf::RenderWindow::create({.size{1024u, 768u}, .title = "Window C"}).value();

        auto baseRenderTexture = sf::RenderTexture::create({100u, 100u});
        auto leftInnerRT       = sf::RenderTexture::create({100u, 100u});

        leftInnerRT->draw(sf::RectangleShapeData{}, {.view = leftInnerRT->makeView()});
        leftInnerRT->display();
    }
}
