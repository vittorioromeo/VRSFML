#include "SFML/Network/Http.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <StringifyIpAddressUtil.hpp>
#include <StringifySfBaseStringUtil.hpp>


TEST_CASE("[Network] sf::Http")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Http));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Http));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Http));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Http));
    }

    SECTION("setHost")
    {
        sf::Http http;

        SECTION("Valid host w/ prefix")
        {
            CHECK(http.setHost("http://google.com"));
        }

        SECTION("Valid host w/o prefix")
        {
            CHECK(http.setHost("google.com"));
        }

        SECTION("Invalid host w/ prefix")
        {
            CHECK(!http.setHost("http://dummy"));
        }

        SECTION("Invalid host w/o prefix")
        {
            CHECK(!http.setHost("dummy"));
        }
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


#ifdef SFML_RUN_CONNECTION_TESTS

TEST_CASE("[Network] sf::Http Connection")
{
    SECTION("HTTP Connection")
    {
        sf::Http http("http://github.com");

        SECTION("Request Index")
        {
            const sf::Http::Response         response = http.sendRequest(sf::Http::Request{}, sf::milliseconds(250));
            const sf::Http::Response::Status status   = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == sf::Http::Response::Status::MovedPermanently);
            CHECK(response.getField("Location") == "https://github.com/");
            CHECK(response.getField("location") == "https://github.com/");
        }

        SECTION("Request Resource")
        {
            const sf::Http::Response response = http.sendRequest(sf::Http::Request("SFML/SFML"), sf::milliseconds(250));
            const sf::Http::Response::Status status = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == sf::Http::Response::Status::MovedPermanently);
            CHECK(response.getField("Location") == "https://github.com/SFML/SFML");
            CHECK(response.getField("location") == "https://github.com/SFML/SFML");
        }
    }

    SECTION("HTTPS Connection")
    {
        sf::Http http("https://github.com");

        SECTION("Request Index")
        {
            const sf::Http::Response         response = http.sendRequest(sf::Http::Request{}, sf::milliseconds(250));
            const sf::Http::Response::Status status   = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == sf::Http::Response::Status::Ok);
            CHECK(!response.getField("Server").empty());
            CHECK(!response.getField("server").empty());
            CHECK(!response.getField("Content-Type").empty());
            CHECK(!response.getField("content-type").empty());
            CHECK(!response.getBody().empty());
        }

        SECTION("Request Resource")
        {
            const sf::Http::Response response = http.sendRequest(sf::Http::Request("SFML/SFML"), sf::milliseconds(250));
            const sf::Http::Response::Status status = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == sf::Http::Response::Status::Ok);
            CHECK(!response.getField("Server").empty());
            CHECK(!response.getField("server").empty());
            CHECK(!response.getField("Content-Type").empty());
            CHECK(!response.getField("content-type").empty());
            CHECK(response.getBody().toStringView().find("SFML") != sf::base::StringView::nPos);
        }

        SECTION("Request Non-Existant Resource")
        {
            const sf::Http::Response response = http.sendRequest(sf::Http::Request("SFML/REPOSITORYTHATDOESNOTEXIST"),
                                                                 sf::milliseconds(250));
            const sf::Http::Response::Status status = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == sf::Http::Response::Status::NotFound);
            CHECK(!response.getField("Server").empty());
            CHECK(!response.getField("server").empty());
        }

        SECTION("HEAD Request")
        {
            http.setHost("https://codeload.github.com");

            sf::Http::Request request("SFML/SFML/zip/refs/heads/master", sf::Http::Request::Method::Head);
            request.setHttpVersion(1, 1);

            const sf::Http::Response         response = http.sendRequest(request, sf::milliseconds(250));
            const sf::Http::Response::Status status   = response.getStatus();

            CHECK(response.getMajorHttpVersion() == 1);
            CHECK(response.getMinorHttpVersion() == 1);
            CHECK(status == sf::Http::Response::Status::Ok);
            CHECK(response.getField("Content-Type") == "application/zip");
            CHECK(response.getField("content-type") == "application/zip");
            CHECK(response.getField("Content-Disposition").toStringView().find("SFML-master.zip") !=
                  sf::base::StringView::nPos);
            CHECK(response.getField("content-disposition").toStringView().find("SFML-master.zip") !=
                  sf::base::StringView::nPos);
        }
    }
}

#endif
