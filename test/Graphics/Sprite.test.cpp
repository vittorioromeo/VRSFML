#include "SFML/Graphics/Sprite.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"

// Other 1st party headers
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>

TEST_CASE("[Graphics] sf::Sprite" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Sprite));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Sprite));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Sprite));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Sprite));
    }

    const auto texture = sf::Texture::create(graphicsContext, {64, 64}).value();

    SECTION("Construction")
    {
        SECTION("Rectangle constructor")
        {
            const sf::Sprite sprite({{0, 0}, {40, 60}});
            CHECK(sprite.getTextureRect() == sf::IntRect({0, 0}, {40, 60}));
            CHECK(sprite.getColor() == sf::Color::White);
            CHECK(sprite.getLocalBounds() == sf::FloatRect({0, 0}, {40, 60}));
            CHECK(sprite.getGlobalBounds() == sf::FloatRect({0, 0}, {40, 60}));
        }

        SECTION("Negative-size texture rectangle")
        {
            const sf::Sprite sprite({{0, 0}, {-40, -60}});
            CHECK(sprite.getTextureRect() == sf::IntRect({0, 0}, {-40, -60}));
            CHECK(sprite.getColor() == sf::Color::White);
            CHECK(sprite.getLocalBounds() == sf::FloatRect({0, 0}, {40, 60}));
            CHECK(sprite.getGlobalBounds() == sf::FloatRect({0, 0}, {40, 60}));
        }
    }

    SECTION("Set/get texture rect")
    {
        sf::Sprite sprite({});
        sprite.setTextureRect({{1, 2}, {3, 4}});
        CHECK(sprite.getTextureRect() == sf::IntRect({1, 2}, {3, 4}));
    }

    SECTION("Set/get color")
    {
        sf::Sprite sprite({});
        sprite.setColor(sf::Color::Red);
        CHECK(sprite.getColor() == sf::Color::Red);
    }
}
