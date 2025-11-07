#include "SFML/Network/Http.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

#include <string>


TEST_CASE("[Network] sf::Http")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Http));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Http));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Http));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Http));
    }

    SECTION("Request")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Http::Request));
            STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Http::Request));
            STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::Http::Request));
            STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Http::Request));
        }
    }

    SECTION("Response")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Http::Response));
            STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Http::Response));
            STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::Http::Response));
            STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Http::Response));
        }

        SECTION("Construction")
        {
            const sf::Http::Response response;
            CHECK(response.getField("").empty());
            CHECK(response.getStatus() == sf::Http::Response::Status::ConnectionFailed);
            CHECK(response.getMajorHttpVersion() == 0);
            CHECK(response.getMinorHttpVersion() == 0);
            CHECK(response.getBody().empty());
        }
    }
}
