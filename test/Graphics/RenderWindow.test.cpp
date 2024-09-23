#include "SFML/Graphics/RenderWindow.hpp"

// Other 1st party headers
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/String.hpp"

#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Graphics] sf::RenderWindow" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::RenderWindow)); // because of RenderTarget, not WindowBase
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::RenderWindow));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::RenderWindow));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::RenderWindow));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::RenderWindow));
    }

    SECTION("Construction")
    {
        SECTION("Style, state, and settings")
        {
            const sf::RenderWindow window(graphicsContext,
                                          {.size{256u, 256u}, .bitsPerPixel = 24, .title = "RenderWindow Tests"});

            CHECK(window.getSize() == sf::Vector2u{256, 256});
#ifndef SFML_SYSTEM_EMSCRIPTEN
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
#else
            CHECK(window.getNativeHandle() == sf::WindowHandle());
#endif
            CHECK(!window.isSrgb());
            CHECK(window.getView().center == sf::Vector2f{128, 128});
            CHECK(window.getView().size == sf::Vector2f{256, 256});
            CHECK(window.getView().rotation == sf::Angle::Zero);
            CHECK(window.getView().viewport == sf::FloatRect({0, 0}, {1, 1}));
            CHECK(window.getView().getTransform() == sf::Transform(0.0078125f, 0, -1, 0, -0.0078125f, 1));
        }

        SECTION("State and settings")
        {
            const sf::RenderWindow window(graphicsContext,
                                          {.size{240u, 300u}, .bitsPerPixel = 24, .title = "RenderWindow Tests"});

            CHECK(window.getSize() == sf::Vector2u{240, 300});
#ifndef SFML_SYSTEM_EMSCRIPTEN
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
#else
            CHECK(window.getNativeHandle() == sf::WindowHandle());
#endif
            CHECK(!window.isSrgb());
            CHECK(window.getView().center == sf::Vector2f{120, 150});
            CHECK(window.getView().size == sf::Vector2f{240, 300});
            CHECK(window.getView().rotation == sf::Angle::Zero);
            CHECK(window.getView().viewport == sf::FloatRect({0, 0}, {1, 1}));
            CHECK(window.getView().getTransform() == Approx(sf::Transform(0.00833333f, 0, -1, 0, -0.00666667f, 1)));
        }
    }

    SECTION("Clear")
    {
        sf::RenderWindow window(graphicsContext, {.size{256u, 256u}, .bitsPerPixel = 24, .title = "RenderWindow Tests"});

        REQUIRE(window.getSize() == sf::Vector2u{256, 256});

        auto texture = sf::Texture::create(graphicsContext, sf::Vector2u{256, 256}).value();

        window.clear(sf::Color::Red);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vector2u{64, 64}) == sf::Color::Red);

        window.clear(sf::Color::Green);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vector2u{128, 128}) == sf::Color::Green);

        window.clear(sf::Color::Blue);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vector2u{196, 196}) == sf::Color::Blue);
    }

// Creating multiple windows in Emscripten is not supported
#ifndef SFML_SYSTEM_EMSCRIPTEN
    SECTION("Multiple windows 1")
    {
        sf::RenderWindow window(graphicsContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::RenderWindow> childWindow(sf::base::inPlace,
                                                         graphicsContext,
                                                         sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        window.clear();
        window.display();

        childWindow.reset();

        window.clear();
        window.display();
    }

    SECTION("Multiple windows 2")
    {
        sf::RenderWindow window(graphicsContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::RenderWindow> childWindow(sf::base::inPlace,
                                                         graphicsContext,
                                                         sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        window.clear();
        window.display();

        childWindow->clear();
        childWindow->display();

        window.clear();
        window.display();
    }

    SECTION("Multiple windows 3")
    {
        sf::RenderWindow window(graphicsContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::RenderWindow> childWindow(sf::base::inPlace,
                                                         graphicsContext,
                                                         sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        childWindow->clear();
        childWindow->display();

        window.clear();
        window.display();
    }

    SECTION("Multiple windows 4")
    {
        sf::RenderWindow window(graphicsContext, {.size{256u, 256u}, .title = "A"});

        sf::base::Optional<sf::RenderWindow> childWindow(sf::base::inPlace,
                                                         graphicsContext,
                                                         sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        childWindow->clear();
        childWindow->display();

        childWindow.reset();

        window.clear();
        window.display();
    }
#endif
}
