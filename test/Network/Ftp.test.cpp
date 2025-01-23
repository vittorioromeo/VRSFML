#include "SFML/Network/Ftp.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <StringifyPathUtil.hpp>

TEST_CASE("[Network] sf::Ftp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Ftp));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Ftp));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Ftp));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Ftp));
    }

    SECTION("Response")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Ftp::Response));
            STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Ftp::Response));
            STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Ftp::Response));
            STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Ftp::Response));
        }

        SECTION("Construction")
        {
            SECTION("Default constructor")
            {
                const sf::Ftp::Response response;
                CHECK(!response.isOk());
                CHECK(response.getStatus() == sf::Ftp::Response::Status::InvalidResponse);
                CHECK(response.getMessage().empty());
            }

            SECTION("Status constructor")
            {
                const sf::Ftp::Response response(sf::Ftp::Response::Status::InvalidFile);
                CHECK(!response.isOk());
                CHECK(response.getStatus() == sf::Ftp::Response::Status::InvalidFile);
                CHECK(response.getMessage().empty());
            }

            SECTION("Status and message constructor")
            {
                const sf::Ftp::Response response(sf::Ftp::Response::Status::Ok, "Ok");
                CHECK(response.isOk());
                CHECK(response.getStatus() == sf::Ftp::Response::Status::Ok);
                CHECK(response.getMessage() == "Ok");
            }
        }

        SECTION("isOk()")
        {
            CHECK(sf::Ftp::Response(sf::Ftp::Response::Status::RestartMarkerReply).isOk());
            CHECK(sf::Ftp::Response(sf::Ftp::Response::Status::Ok).isOk());
            CHECK(sf::Ftp::Response(sf::Ftp::Response::Status::NeedPassword).isOk());
            CHECK(!sf::Ftp::Response(sf::Ftp::Response::Status::ServiceUnavailable).isOk());
            CHECK(!sf::Ftp::Response(sf::Ftp::Response::Status::CommandUnknown).isOk());
            CHECK(!sf::Ftp::Response(sf::Ftp::Response::Status::InvalidResponse).isOk());
        }
    }

    SECTION("DirectoryResponse")
    {
        SECTION("Construction")
        {
            const sf::Ftp::DirectoryResponse directoryResponse(sf::Ftp::Response(sf::Ftp::Response::Status::Ok, "Ok"));
            CHECK(directoryResponse.isOk());
            CHECK(directoryResponse.getStatus() == sf::Ftp::Response::Status::Ok);
            CHECK(directoryResponse.getMessage() == "Ok");
            CHECK(directoryResponse.getDirectory() == "Ok");
        }

        SECTION("getDirectory()")
        {
            CHECK(sf::Ftp::DirectoryResponse(sf::Ftp::Response{}).getDirectory().empty());
            CHECK(sf::Ftp::DirectoryResponse(sf::Ftp::Response{sf::Ftp::Response::Status::Ok, "/usr/local/lib"}).getDirectory() ==
                  "/usr/local/lib");
        }
    }

    SECTION("ListingResponse")
    {
        SECTION("Construction")
        {
            const sf::Ftp::ListingResponse listingResponse(sf::Ftp::Response(sf::Ftp::Response::Status::Ok), "");
            CHECK(listingResponse.isOk());
            CHECK(listingResponse.getStatus() == sf::Ftp::Response::Status::Ok);
            CHECK(listingResponse.getMessage().empty());
            CHECK(listingResponse.getListing().empty());
        }

        SECTION("getListing()")
        {
            const sf::Ftp::ListingResponse listingResponse(sf::Ftp::Response(sf::Ftp::Response::Status::Ok),
                                                           "foo\r\nbar\r\nbaz");

            const std::string vec[]{"foo", "bar"};
            CHECK(listingResponse.getListing().valueEquals(vec, 2));
        }
    }
}
