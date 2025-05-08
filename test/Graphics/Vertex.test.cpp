#include "SFML/Graphics/Vertex.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>

TEST_CASE("[Graphics] sf::Vertex")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Vertex));
    }

    SECTION("Construction")
    {
        SECTION("Aggregate initialization -- Nothing")
        {
            constexpr sf::Vertex vertex{};
            STATIC_CHECK(vertex.position == sf::Vec2f(0.f, 0.f));
            STATIC_CHECK(vertex.color == sf::Color(255, 255, 255));
            STATIC_CHECK(vertex.texCoords == sf::Vec2f(0.f, 0.f));
        }

        SECTION("Aggregate initialization -- Position")
        {
            constexpr sf::Vertex vertex{{1.f, 2.f}};
            STATIC_CHECK(vertex.position == sf::Vec2f(1.f, 2.f));
            STATIC_CHECK(vertex.color == sf::Color(255, 255, 255));
            STATIC_CHECK(vertex.texCoords == sf::Vec2f(0.f, 0.f));
        }

        SECTION("Aggregate initialization -- Position and color")
        {
            constexpr sf::Vertex vertex{{1.f, 2.f}, {3, 4, 5, 6}};
            STATIC_CHECK(vertex.position == sf::Vec2f(1.f, 2.f));
            STATIC_CHECK(vertex.color == sf::Color(3, 4, 5, 6));
            STATIC_CHECK(vertex.texCoords == sf::Vec2f(0.f, 0.f));
        }

        SECTION("Aggregate initialization -- Position, color, and coords")
        {
            constexpr sf::Vertex vertex{{1.f, 2.f}, {3, 4, 5, 6}, {7.f, 8.f}};
            STATIC_CHECK(vertex.position == sf::Vec2f(1.f, 2.f));
            STATIC_CHECK(vertex.color == sf::Color(3, 4, 5, 6));
            STATIC_CHECK(vertex.texCoords == sf::Vec2f(7.f, 8.f));
        }
    }
}
