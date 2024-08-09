#include <SFML/Window/Window.hpp>
#include <SFML/Window/WindowContext.hpp>

// Other 1st party headers
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/String.hpp>

#include <SFML/Base/Optional.hpp>

#include <Doctest.hpp>

#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

#include <type_traits>

TEST_CASE("[Window] sf::Window" * doctest::skip(skipDisplayTests))
{
    sf::WindowContext windowContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::has_virtual_destructor_v<sf::Window>);
        STATIC_CHECK(!std::is_copy_constructible_v<sf::Window>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::Window>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Window>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Window>);
    }

    SECTION("Construction")
    {
        SECTION("Mode and title constructor")
        {
            const sf::Window window(windowContext, sf::VideoMode({360, 240}), "Window Tests");
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, and style constructor")
        {
            const sf::Window window(windowContext, sf::VideoMode({360, 240}), "Window Tests", sf::Style::Resize);
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, and state constructor")
        {
            const sf::Window window(windowContext, sf::VideoMode({360, 240}), "Window Tests", sf::Style::Resize, sf::State::Windowed);
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, state, and context settings constructor")
        {
            const sf::Window window(windowContext,
                                    sf::VideoMode({360, 240}),
                                    "Window Tests",
                                    sf::Style::Resize,
                                    sf::State::Windowed,
                                    sf::ContextSettings{/* depthBits*/ 1, /* stencilBits */ 1, /* antialiasingLevel */ 1});
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
            CHECK(window.getSettings().antialiasingLevel >= 1);
        }

        SECTION("Mode, title, and state")
        {
            const sf::Window window(windowContext, sf::VideoMode({360, 240}), "Window Tests", sf::State::Windowed);
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, state, and context settings constructor")
        {
            const sf::Window window(windowContext,
                                    sf::VideoMode({360, 240}),
                                    "Window Tests",
                                    sf::State::Windowed,
                                    sf::ContextSettings{/* depthBits*/ 1, /* stencilBits */ 1, /* antialiasingLevel */ 1});
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
        sf::Window                     window(windowContext, sf::VideoMode({256, 256}), "A");
        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace, windowContext, sf::VideoMode({256, 256}), "B");

        window.display();
        childWindow.reset();
        window.display();
    }

    SECTION("Multiple windows 2")
    {
        sf::Window                     window(windowContext, sf::VideoMode({256, 256}), "A");
        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace, windowContext, sf::VideoMode({256, 256}), "B");

        window.display();
        childWindow->display();
        window.display();
    }

    SECTION("Multiple windows 3")
    {
        sf::Window                     window(windowContext, sf::VideoMode({256, 256}), "A");
        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace, windowContext, sf::VideoMode({256, 256}), "B");

        childWindow->display();
        window.display();
    }

    SECTION("Multiple windows 4")
    {
        sf::Window                     window(windowContext, sf::VideoMode({256, 256}), "A");
        sf::base::Optional<sf::Window> childWindow(sf::base::inPlace, windowContext, sf::VideoMode({256, 256}), "B");

        childWindow->display();
        childWindow.reset();
        window.display();
    }
#endif
}
