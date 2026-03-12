#include "SFML/Window/Window.hpp"

#include "SFML/Window/WindowContext.hpp"

// Other 1st party headers
#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

TEST_CASE("[Window] sf::Window" * doctest::skip(skipDisplayTests))
{
    auto windowContext = sf::WindowContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::Window));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Window));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Window));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Window));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Window));
    }

    SECTION("Construction")
    {
        SECTION("Mode and title constructor")
        {
            const auto window = sf::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == sf::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, and style constructor")
        {
            const auto window = sf::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == sf::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, and state constructor")
        {
            const auto window = sf::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == sf::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, state, and context settings constructor")
        {
            const auto window = sf::Window::create({.size{360u, 240u},
                                                    .title = "Window Tests",
                                                    .contextSettings{.depthBits = 1, .stencilBits = 1}})
                                    .value();

            CHECK(window.getSize() == sf::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
        }

        SECTION("Mode, title, and state")
        {
            const auto window = sf::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == sf::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, state, and context settings constructor")
        {
            const auto window = sf::Window::create({.size{360u, 240u},
                                                    .title = "Window Tests",
                                                    .contextSettings{.depthBits = 1, .stencilBits = 1}})
                                    .value();

            CHECK(window.getSize() == sf::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
        }
    }

// Creating multiple windows in Emscripten is not supported
#ifndef SFML_SYSTEM_EMSCRIPTEN
    SECTION("Multiple windows 1")
    {
        auto window      = sf::Window::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::Window::create(sf::Window::Settings{.size{256u, 256u}, .title = "B"});

        window.display();
        childWindow.reset();
        window.display();
    }

    SECTION("Multiple windows 2")
    {
        auto window      = sf::Window::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::Window::create(sf::Window::Settings{.size{256u, 256u}, .title = "B"});

        window.display();
        childWindow->display();
        window.display();
    }

    SECTION("Multiple windows 3")
    {
        auto window      = sf::Window::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::Window::create(sf::Window::Settings{.size{256u, 256u}, .title = "B"});

        childWindow->display();
        window.display();
    }

    SECTION("Multiple windows 4")
    {
        auto window      = sf::Window::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::Window::create(sf::Window::Settings{.size{256u, 256u}, .title = "B"});

        childWindow->display();
        childWindow.reset();
        window.display();
    }
#endif
}
