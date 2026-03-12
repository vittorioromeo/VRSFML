#include "SFML/Base/UniquePtr.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>


TEST_CASE("[Base] Base/UniquePtr.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::UniquePtr<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::UniquePtr<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::UniquePtr<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::UniquePtr<int>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::UniquePtr<int>)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::UniquePtr<int>));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::base::UniquePtr<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::UniquePtr<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::UniquePtr<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::UniquePtr<int>, sf::base::UniquePtr<int>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::UniquePtr<int>));
    }
}
