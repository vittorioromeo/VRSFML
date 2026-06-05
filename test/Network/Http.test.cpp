#include "StringifyIpAddressUtil.hpp"
#include "StringifyZbStringUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Network/Http.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsMoveConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Network] za::Http")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZA_IS_COPY_CONSTRUCTIBLE(za::Http));
        STATIC_CHECK(!ZA_IS_COPY_ASSIGNABLE(za::Http));
        STATIC_CHECK(!ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Http));
        STATIC_CHECK(!ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::Http));
    }

    SECTION("setHost")
    {
        za::Http http;

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
            STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::Http::Request));
            STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::Http::Request));
            STATIC_CHECK(ZA_IS_MOVE_CONSTRUCTIBLE(za::Http::Request));
            STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::Http::Request));
        }
    }

    SECTION("Response")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::Http::Response));
            STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::Http::Response));
            STATIC_CHECK(ZA_IS_MOVE_CONSTRUCTIBLE(za::Http::Response));
            STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::Http::Response));
        }

        SECTION("Construction")
        {
            const za::Http::Response response;
            CHECK(response.getField("").empty());
            CHECK(response.getStatus() == za::Http::Response::Status::ConnectionFailed);
            CHECK(response.getMajorHttpVersion() == 0);
            CHECK(response.getMinorHttpVersion() == 0);
            CHECK(response.getBody().empty());
        }
    }
}


#ifdef ZA_RUN_CONNECTION_TESTS

TEST_CASE("[Network] za::Http Connection")
{
    SECTION("HTTP Connection")
    {
        za::Http http("http://github.com");

        SECTION("Request Index")
        {
            const za::Http::Response         response = http.sendRequest(za::Http::Request{}, za::milliseconds(250));
            const za::Http::Response::Status status   = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == za::Http::Response::Status::MovedPermanently);
            CHECK(response.getField("Location") == "https://github.com/");
            CHECK(response.getField("location") == "https://github.com/");
        }

        SECTION("Request Resource")
        {
            const za::Http::Response response = http.sendRequest(za::Http::Request("Zancle/Zancle"), za::milliseconds(250));
            const za::Http::Response::Status status = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == za::Http::Response::Status::MovedPermanently);
            CHECK(response.getField("Location") == "https://github.com/SFML/SFML");
            CHECK(response.getField("location") == "https://github.com/SFML/SFML");
        }
    }

    SECTION("HTTPS Connection")
    {
        za::Http http("https://github.com");

        SECTION("Request Index")
        {
            const za::Http::Response         response = http.sendRequest(za::Http::Request{}, za::milliseconds(250));
            const za::Http::Response::Status status   = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == za::Http::Response::Status::Ok);
            CHECK(!response.getField("Server").empty());
            CHECK(!response.getField("server").empty());
            CHECK(!response.getField("Content-Type").empty());
            CHECK(!response.getField("content-type").empty());
            CHECK(!response.getBody().empty());
        }

        SECTION("Request Resource")
        {
            const za::Http::Response response = http.sendRequest(za::Http::Request("Zancle/Zancle"), za::milliseconds(250));
            const za::Http::Response::Status status = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == za::Http::Response::Status::Ok);
            CHECK(!response.getField("Server").empty());
            CHECK(!response.getField("server").empty());
            CHECK(!response.getField("Content-Type").empty());
            CHECK(!response.getField("content-type").empty());
            CHECK(response.getBody().find("Zancle") != za::StringView::nPos);
        }

        SECTION("Request Non-Existant Resource")
        {
            const za::Http::Response response = http.sendRequest(za::Http::Request("Zancle/REPOSITORYTHATDOESNOTEXIST"),
                                                                 za::milliseconds(250));
            const za::Http::Response::Status status = response.getStatus();

            CHECK(response.getMajorHttpVersion() != 0);
            CHECK(status == za::Http::Response::Status::NotFound);
            CHECK(!response.getField("Server").empty());
            CHECK(!response.getField("server").empty());
        }

        SECTION("HEAD Request")
        {
            http.setHost("https://codeload.github.com");

            za::Http::Request request("Zancle/Zancle/zip/refs/heads/master", za::Http::Request::Method::Head);
            request.setHttpVersion(1, 1);

            const za::Http::Response         response = http.sendRequest(request, za::milliseconds(250));
            const za::Http::Response::Status status   = response.getStatus();

            CHECK(response.getMajorHttpVersion() == 1);
            CHECK(response.getMinorHttpVersion() == 1);
            CHECK(status == za::Http::Response::Status::Ok);
            CHECK(response.getField("Content-Type") == "application/zip");
            CHECK(response.getField("content-type") == "application/zip");
            CHECK(response.getField("Content-Disposition").find("Zancle-master.zip") != za::StringView::nPos);
            CHECK(response.getField("content-disposition").find("Zancle-master.zip") != za::StringView::nPos);
        }
    }
}

#endif
