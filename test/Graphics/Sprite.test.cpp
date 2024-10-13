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

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::Sprite)); // because of member initializers
        STATIC_CHECK(!SFML_BASE_IS_STANDARD_LAYOUT(sf::Sprite)); // "only one class in the hierarchy has non-static data members"
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Sprite));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Sprite));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::Sprite));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::Sprite, sf::Sprite));
    }

    const auto texture = sf::Texture::create(graphicsContext, {64, 64}).value();

    SECTION("Construction")
    {
        SECTION("Rectangle constructor")
        {
            const sf::Sprite sprite{.textureRect = {{0.f, 0.f}, {40.f, 60.f}}};
            CHECK(sprite.textureRect == sf::FloatRect({0.f, 0.f}, {40.f, 60.f}));
            CHECK(sprite.color == sf::Color::White);
            CHECK(sprite.getLocalBounds() == sf::FloatRect({0.f, 0.f}, {40.f, 60.f}));
            CHECK(sprite.getGlobalBounds() == sf::FloatRect({0.f, 0.f}, {40.f, 60.f}));
        }

        SECTION("Negative-size texture rectangle")
        {
            const sf::Sprite sprite{.textureRect = {{0.f, 0.f}, {-40.f, -60.f}}};
            CHECK(sprite.textureRect == sf::FloatRect({0.f, 0.f}, {-40.f, -60.f}));
            CHECK(sprite.color == sf::Color::White);
            CHECK(sprite.getLocalBounds() == sf::FloatRect({0.f, 0.f}, {40.f, 60.f}));
            CHECK(sprite.getGlobalBounds() == sf::FloatRect({0.f, 0.f}, {40.f, 60.f}));
        }
    }

    SECTION("Set/get texture rect")
    {
        sf::Sprite sprite{.textureRect = {}};
        sprite.textureRect = {{1, 2}, {3, 4}};
        CHECK(sprite.textureRect == sf::FloatRect({1, 2}, {3, 4}));
    }

    SECTION("Set/get color")
    {
        sf::Sprite sprite{.textureRect = {}};
        sprite.color = sf::Color::Red;
        CHECK(sprite.color == sf::Color::Red);
    }
}
