#include <SFML/Graphics/Glyph.hpp>

#include <catch2/catch_test_macros.hpp>

#include <GraphicsUtil.hpp>
#include <type_traits>

static_assert(std::is_copy_constructible_v<sf::Glyph>);
static_assert(std::is_copy_assignable_v<sf::Glyph>);
static_assert(std::is_nothrow_move_constructible_v<sf::Glyph>);
static_assert(std::is_nothrow_move_assignable_v<sf::Glyph>);

TEST_CASE("[Graphics] sf::Glyph")
{
    SECTION("Construction")
    {
        const sf::Glyph glyph;
        CHECK(glyph.advance == 0.f);
        CHECK(glyph.lsbDelta == 0);
        CHECK(glyph.rsbDelta == 0);
        CHECK(glyph.bounds == sf::FloatRect());
        CHECK(glyph.textureRect == sf::IntRect());
    }
}
