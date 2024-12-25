#include "SFML/Graphics/Glyph.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>

TEST_CASE("[Graphics] sf::Glyph")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Glyph));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Glyph));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Glyph));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Glyph));
    }

    SECTION("Construction")
    {
        constexpr sf::Glyph glyph{};
        STATIC_CHECK(glyph.advance == 0.f);
        STATIC_CHECK(glyph.lsbDelta == 0);
        STATIC_CHECK(glyph.rsbDelta == 0);
        STATIC_CHECK(glyph.bounds == sf::FloatRect());
        STATIC_CHECK(glyph.textureRect == sf::FloatRect());
    }
}
