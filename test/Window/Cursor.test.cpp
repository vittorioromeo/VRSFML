#include <SFML/Window/Cursor.hpp>

#include <Doctest.hpp>

#include <StringifyOptionalUtil.hpp>
#include <WindowUtil.hpp>

#include <type_traits>

TEST_CASE("[Window] sf::Cursor" * doctest::skip(skipDisplayTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_default_constructible_v<sf::Cursor>);
        STATIC_CHECK(!std::is_copy_constructible_v<sf::Cursor>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::Cursor>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Cursor>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Cursor>);
    }

    SECTION("loadFromPixels()")
    {
        static constexpr std::uint8_t pixels[4]{};

        CHECK(!sf::Cursor::loadFromPixels(nullptr, {}, {}).hasValue());
        CHECK(!sf::Cursor::loadFromPixels(pixels, {0, 1}, {}).hasValue());
        CHECK(!sf::Cursor::loadFromPixels(pixels, {1, 0}, {}).hasValue());
        CHECK(sf::Cursor::loadFromPixels(pixels, {1, 1}, {}).hasValue());
    }

    SECTION("loadFromSystem()")
    {
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::Hand).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeHorizontal).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeVertical).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeLeft).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeRight).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeTop).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeBottom).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeTopLeft).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeTopRight).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeBottomLeft).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeBottomRight).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::Cross).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::Help).hasValue());
        CHECK(sf::Cursor::loadFromSystem(sf::Cursor::Type::NotAllowed).hasValue());
    }
}
