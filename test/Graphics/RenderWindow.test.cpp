#include "SFML/Graphics/RenderWindow.hpp"

// Other 1st party headers
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Graphics] sf::RenderWindow" * doctest::skip(skipDisplayTests))
{
    auto graphicsContext = sf::GraphicsContext::create().value();

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
            const auto window = sf::RenderWindow::create(
                                    {.size{256u, 256u}, .bitsPerPixel = 24, .title = "RenderWindow Tests"})
                                    .value();

            CHECK(window.getSize() == sf::Vec2u{256, 256});
#ifndef SFML_SYSTEM_EMSCRIPTEN
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
#else
            CHECK(window.getNativeHandle() == sf::WindowHandle());
#endif
            CHECK(!window.isSrgb());
            CHECK(window.getView().center == sf::Vec2f{128, 128});
            CHECK(window.getView().size == sf::Vec2f{256, 256});
            CHECK(window.getView().rotation == sf::Angle::Zero);
            CHECK(window.getView().viewport == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(window.getView().getTransform() == Approx(sf::Transform(0.0078125f, 0, -1, 0, -0.0078125f, 1)));
        }

        SECTION("State and settings")
        {
            const auto window = sf::RenderWindow::create(
                                    {.size{240u, 300u}, .bitsPerPixel = 24, .title = "RenderWindow Tests"})
                                    .value();

            CHECK(window.getSize() == sf::Vec2u{240, 300});
#ifndef SFML_SYSTEM_EMSCRIPTEN
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
#else
            CHECK(window.getNativeHandle() == sf::WindowHandle());
#endif
            CHECK(!window.isSrgb());
            CHECK(window.getView().center == sf::Vec2f{120, 150});
            CHECK(window.getView().size == sf::Vec2f{240, 300});
            CHECK(window.getView().rotation == sf::Angle::Zero);
            CHECK(window.getView().viewport == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(window.getView().getTransform() == Approx(sf::Transform(0.00833333f, 0, -1, 0, -0.00666667f, 1)));
        }
    }

    SECTION("Clear")
    {
        auto window = sf::RenderWindow::create({.size{256u, 256u}, .title = "RenderWindow Tests"}).value();

        REQUIRE(window.getSize() == sf::Vec2u{256, 256});

        auto texture = sf::Texture::create(sf::Vec2u{256, 256}).value();

        window.clear(sf::Color::Red);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vec2u{64, 64}) == sf::Color::Red);

        window.clear(sf::Color::Green);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vec2u{128, 128}) == sf::Color::Green);

        window.clear(sf::Color::Blue);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vec2u{196, 196}) == sf::Color::Blue);
    }

// Creating multiple windows in Emscripten is not supported
#ifndef SFML_SYSTEM_EMSCRIPTEN
    SECTION("Move assignment")
    {
        auto window0 = sf::RenderWindow::create({.size{128u, 128u}, .title = "A"}).value();
        auto window1 = sf::RenderWindow::create({.size{256u, 256u}, .title = "B"}).value();

        window1 = SFML_BASE_MOVE(window0);
        CHECK(window1.getSize() == sf::Vec2u{128u, 128u});
    }

    SECTION("Multiple windows 1")
    {
        auto window      = sf::RenderWindow::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::RenderWindow::create(sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        window.clear();
        window.display();

        childWindow.reset();

        window.clear();
        window.display();
    }

    SECTION("Multiple windows 2")
    {
        auto window      = sf::RenderWindow::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::RenderWindow::create(sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        window.clear();
        window.display();

        childWindow->clear();
        childWindow->display();

        window.clear();
        window.display();
    }

    SECTION("Multiple windows 3")
    {
        auto window      = sf::RenderWindow::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::RenderWindow::create(sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        childWindow->clear();
        childWindow->display();

        window.clear();
        window.display();
    }

    SECTION("Multiple windows 4")
    {
        auto window      = sf::RenderWindow::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = sf::RenderWindow::create(sf::RenderWindow::Settings{.size{256u, 256u}, .title = "B"});

        childWindow->clear();
        childWindow->display();

        childWindow.reset();

        window.clear();
        window.display();
    }
#endif
}
