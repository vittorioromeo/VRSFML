#include <SFML/Graphics/Vertex.hpp>

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>

TEST_CASE("[Graphics] sf::Vertex")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Vertex));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Vertex));
    }

    SECTION("Construction")
    {
        SECTION("Aggregate initialization -- Nothing")
        {
            constexpr sf::Vertex vertex;
            STATIC_CHECK(vertex.position == sf::Vector2f(0.0f, 0.0f));
            STATIC_CHECK(vertex.color == sf::Color(255, 255, 255));
            STATIC_CHECK(vertex.texCoords == sf::Vector2f(0.0f, 0.0f));
        }

        SECTION("Aggregate initialization -- Position")
        {
            constexpr sf::Vertex vertex{{1.0f, 2.0f}};
            STATIC_CHECK(vertex.position == sf::Vector2f(1.0f, 2.0f));
            STATIC_CHECK(vertex.color == sf::Color(255, 255, 255));
            STATIC_CHECK(vertex.texCoords == sf::Vector2f(0.0f, 0.0f));
        }

        SECTION("Aggregate initialization -- Position and color")
        {
            constexpr sf::Vertex vertex{{1.0f, 2.0f}, {3, 4, 5, 6}};
            STATIC_CHECK(vertex.position == sf::Vector2f(1.0f, 2.0f));
            STATIC_CHECK(vertex.color == sf::Color(3, 4, 5, 6));
            STATIC_CHECK(vertex.texCoords == sf::Vector2f(0.0f, 0.0f));
        }

        SECTION("Aggregate initialization -- Position, color, and coords")
        {
            constexpr sf::Vertex vertex{{1.0f, 2.0f}, {3, 4, 5, 6}, {7.0f, 8.0f}};
            STATIC_CHECK(vertex.position == sf::Vector2f(1.0f, 2.0f));
            STATIC_CHECK(vertex.color == sf::Color(3, 4, 5, 6));
            STATIC_CHECK(vertex.texCoords == sf::Vector2f(7.0f, 8.0f));
        }
    }
}
