#include <SFML/Window/WindowBase.hpp>

// Other 1st party headers
#include <SFML/System/String.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <catch2/catch_test_macros.hpp>

#include <WindowUtil.hpp>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<sf::WindowBase>);
static_assert(!std::is_copy_assignable_v<sf::WindowBase>);
static_assert(!std::is_nothrow_move_constructible_v<sf::WindowBase>);
static_assert(!std::is_nothrow_move_assignable_v<sf::WindowBase>);

TEST_CASE("[Window] sf::WindowBase", runDisplayTests())
{
    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const sf::WindowBase windowBase;
            CHECK(!windowBase.isOpen());
            CHECK(windowBase.getPosition() == sf::Vector2i());
            CHECK(windowBase.getSize() == sf::Vector2u());
            CHECK(!windowBase.hasFocus());
            CHECK(windowBase.getSystemHandle() == sf::WindowHandle());
        }

        SECTION("Mode and title constructor")
        {
            const sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
            CHECK(windowBase.isOpen());
            CHECK(windowBase.getSize() == sf::Vector2u(360, 240));
            CHECK(windowBase.getSystemHandle() != sf::WindowHandle());
        }

        SECTION("Mode, title, and style constructor")
        {
            const sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests", sf::Style::Resize);
            CHECK(windowBase.isOpen());
            CHECK(windowBase.getSize() == sf::Vector2u(360, 240));
            CHECK(windowBase.getSystemHandle() != sf::WindowHandle());
        }
    }

    SECTION("create()")
    {
        sf::WindowBase windowBase;

        SECTION("Mode and title")
        {
            windowBase.create(sf::VideoMode({240, 360}), "WindowBase Tests");
            CHECK(windowBase.isOpen());
            CHECK(windowBase.getSize() == sf::Vector2u(240, 360));
            CHECK(windowBase.getSystemHandle() != sf::WindowHandle());
        }

        SECTION("Mode, title, and style")
        {
            windowBase.create(sf::VideoMode({240, 360}), "WindowBase Tests", sf::Style::Resize);
            CHECK(windowBase.isOpen());
            CHECK(windowBase.getSize() == sf::Vector2u(240, 360));
            CHECK(windowBase.getSystemHandle() != sf::WindowHandle());
        }
    }

    SECTION("close()")
    {
        sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
        windowBase.close();
        CHECK(!windowBase.isOpen());
    }

    SECTION("pollEvent()")
    {
        sf::WindowBase windowBase;
        sf::Event      event;
        CHECK(!windowBase.pollEvent(event));
    }

    SECTION("waitEvent()")
    {
        sf::WindowBase windowBase;
        sf::Event      event;
        CHECK(!windowBase.waitEvent(event));
    }

    SECTION("Get/set position")
    {
        sf::WindowBase windowBase;
        windowBase.setPosition({12, 34});
        CHECK(windowBase.getPosition() == sf::Vector2i());
    }

    SECTION("Set/get size")
    {
        SECTION("Uninitialized window")
        {
            sf::WindowBase windowBase;
            windowBase.setSize({128, 256});
            CHECK(windowBase.getSize() == sf::Vector2u());
        }

        SECTION("Initialized window")
        {
            sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
            windowBase.setSize({128, 256});
            CHECK(windowBase.getSize() == sf::Vector2u(128, 256));
        }
    }
}
