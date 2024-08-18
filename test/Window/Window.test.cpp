#include "SFML/Window/Window.hpp"

#include "SFML/Window/WindowContext.hpp"

// Other 1st party headers
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/String.hpp"

#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

TEST_CASE("[Window] sf::Window" * doctest::skip(skipDisplayTests))
{
    sf::WindowContext windowContext;

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
            const sf::Window window(windowContext, {.size{360u, 240u}, .title = "Window Tests"});

            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, and style constructor")
        {
            const sf::Window window(windowContext, {.size{360u, 240u}, .title = "Window Tests", .style = sf::Style::Resize});

            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, and state constructor")
        {
            const sf::Window window(windowContext,
                                    {.size{360u, 240u},
                                     .title = "Window Tests",
                                     .style = sf::Style::Resize,
                                     .state = sf::State::Windowed});

            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, state, and context settings constructor")
        {
            const sf::Window window(windowContext,
                                    {.size{360u, 240u},
                                     .title = "Window Tests",
                                     .style = sf::Style::Resize,
                                     .state = sf::State::Windowed,
                                     .contextSettings{.depthBits = 1, .stencilBits = 1, .antialiasingLevel = 1}});

            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
            CHECK(window.getSettings().antialiasingLevel >= 1);
        }

        SECTION("Mode, title, and state")
        {
            const sf::Window window(windowContext,
                                    {.size{360u, 240u}, .title = "Window Tests", .state = sf::State::Windowed});

            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, state, and context settings constructor")
        {
            const sf::Window window(windowContext,
                                    {.size{360u, 240u},
                                     .title = "Window Tests",
                                     .state = sf::State::Windowed,
                                     .contextSettings{.depthBits = 1, .stencilBits = 1, .antialiasingLevel = 1}});

            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
            CHECK(window.getSettings().antialiasingLevel >= 1);
        }
    }

// Creating multiple windows in Emscripten is not supported
#ifndef SFML_SYSTEM_EMSCRIPTEN
    SECTION("Multiple windows 1")
    {
        sf::Window window(windowContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace,
                                                   windowContext,
                                                   sf::WindowSettings{.size{256u, 256u}, .title = "B"});

        window.display();
        childWindow.reset();
        window.display();
    }

    SECTION("Multiple windows 2")
    {
        sf::Window window(windowContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace,
                                                   windowContext,
                                                   sf::WindowSettings{.size{256u, 256u}, .title = "B"});

        window.display();
        childWindow->display();
        window.display();
    }

    SECTION("Multiple windows 3")
    {
        sf::Window window(windowContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace,
                                                   windowContext,
                                                   sf::WindowSettings{.size{256u, 256u}, .title = "B"});

        childWindow->display();
        window.display();
    }

    SECTION("Multiple windows 4")
    {
        sf::Window window(windowContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace,
                                                   windowContext,
                                                   sf::WindowSettings{.size{256u, 256u}, .title = "B"});

        childWindow->display();
        childWindow.reset();
        window.display();
    }
#endif
}
