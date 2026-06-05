#include "StringifyPathUtil.hpp"
#include "StringifyStringViewUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Network/Ftp.hpp"

#include "Zancle/String/String.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Network] za::Ftp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZA_IS_COPY_CONSTRUCTIBLE(za::Ftp));
        STATIC_CHECK(!ZA_IS_COPY_ASSIGNABLE(za::Ftp));
        STATIC_CHECK(!ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Ftp));
        STATIC_CHECK(!ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::Ftp));
    }

    SECTION("Response")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::Ftp::Response));
            STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::Ftp::Response));
            STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Ftp::Response));
            STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::Ftp::Response));
        }

        SECTION("Construction")
        {
            SECTION("Default constructor")
            {
                const za::Ftp::Response response;
                CHECK(!response.isOk());
                CHECK(response.getStatus() == za::Ftp::Response::Status::InvalidResponse);
                CHECK(response.getMessage().empty());
            }

            SECTION("Status constructor")
            {
                const za::Ftp::Response response(za::Ftp::Response::Status::InvalidFile);
                CHECK(!response.isOk());
                CHECK(response.getStatus() == za::Ftp::Response::Status::InvalidFile);
                CHECK(response.getMessage().empty());
            }

            SECTION("Status and message constructor")
            {
                const za::Ftp::Response response(za::Ftp::Response::Status::Ok, "Ok");
                CHECK(response.isOk());
                CHECK(response.getStatus() == za::Ftp::Response::Status::Ok);
                CHECK(response.getMessage() == "Ok");
            }
        }

        SECTION("isOk()")
        {
            CHECK(za::Ftp::Response(za::Ftp::Response::Status::RestartMarkerReply).isOk());
            CHECK(za::Ftp::Response(za::Ftp::Response::Status::Ok).isOk());
            CHECK(za::Ftp::Response(za::Ftp::Response::Status::NeedPassword).isOk());
            CHECK(!za::Ftp::Response(za::Ftp::Response::Status::ServiceUnavailable).isOk());
            CHECK(!za::Ftp::Response(za::Ftp::Response::Status::CommandUnknown).isOk());
            CHECK(!za::Ftp::Response(za::Ftp::Response::Status::InvalidResponse).isOk());
        }
    }

    SECTION("DirectoryResponse")
    {
        SECTION("Construction")
        {
            const za::Ftp::DirectoryResponse directoryResponse(za::Ftp::Response(za::Ftp::Response::Status::Ok, "Ok"));
            CHECK(directoryResponse.isOk());
            CHECK(directoryResponse.getStatus() == za::Ftp::Response::Status::Ok);
            CHECK(directoryResponse.getMessage() == "Ok");
            CHECK(directoryResponse.getDirectory() == "Ok");
        }

        SECTION("getDirectory()")
        {
            CHECK(za::Ftp::DirectoryResponse(za::Ftp::Response{}).getDirectory().empty());
            CHECK(za::Ftp::DirectoryResponse(za::Ftp::Response{za::Ftp::Response::Status::Ok, "/usr/local/lib"}).getDirectory() ==
                  "/usr/local/lib");
        }
    }

    SECTION("ListingResponse")
    {
        SECTION("Construction")
        {
            const za::Ftp::ListingResponse listingResponse(za::Ftp::Response(za::Ftp::Response::Status::Ok), "");
            CHECK(listingResponse.isOk());
            CHECK(listingResponse.getStatus() == za::Ftp::Response::Status::Ok);
            CHECK(listingResponse.getMessage().empty());
            CHECK(listingResponse.getListing().empty());
        }

        SECTION("getListing()")
        {
            const za::Ftp::ListingResponse listingResponse(za::Ftp::Response(za::Ftp::Response::Status::Ok),
                                                           "foo\r\nbar\r\nbaz");

            const za::String vec[]{"foo", "bar"};
            CHECK(listingResponse.getListing().valueEquals(vec, 2));
        }
    }
}
