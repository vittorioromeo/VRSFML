#include "SFML/Base/Span.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>


TEST_CASE("[Base] Base/Span.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Span<int>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Span<int>)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Span<int>));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Span<int>, sf::base::Span<int>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Span<int>));
    }

    SECTION("Structured bindings")
    {
        sf::base::Span<int> span{nullptr, 0u};

        auto [data, size] = span;

        CHECK(data == nullptr);
        CHECK(size == 0u);
    }
}
