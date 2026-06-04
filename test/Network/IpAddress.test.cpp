#include "StringifyIpAddressUtil.hpp"
#include "StringifySfBaseStringUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Network/IpAddress.hpp"

#include "Zancle/Network/IpAddressUtils.hpp"

#include "Zancle/System/Time.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"


using namespace zb::literals;


TEST_CASE("[Network] za::IpAddress")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::IpAddress));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(za::IpAddress));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::IpAddress));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::IpAddress));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::IpAddress));
    }

    SECTION("Construction")
    {
        SECTION("_static 'create' function")
        {
            const auto ipAddress = za::IpAddressUtils::resolve("203.0.113.2"_sv);
            REQUIRE(ipAddress.hasValue());
            CHECK(za::IpAddressUtils::toString(*ipAddress) == "203.0.113.2"_s);
            CHECK(ipAddress->toInteger() == 0xCB'00'71'02);
            CHECK(*ipAddress != za::IpAddress::Any);
            CHECK(*ipAddress != za::IpAddress::Broadcast);
            CHECK(*ipAddress != za::IpAddress::LocalHost);

            const auto broadcast = za::IpAddressUtils::resolve("255.255.255.255"_sv);
            REQUIRE(broadcast.hasValue());
            CHECK(za::IpAddressUtils::toString(*broadcast) == "255.255.255.255"_s);
            CHECK(broadcast->toInteger() == 0xFF'FF'FF'FF);
            CHECK(*broadcast == za::IpAddress::Broadcast);

            const auto any = za::IpAddressUtils::resolve("0.0.0.0"_sv);
            REQUIRE(any.hasValue());
            CHECK(za::IpAddressUtils::toString(*any) == "0.0.0.0"_s);
            CHECK(any->toInteger() == 0x00'00'00'00);
            CHECK(*any == za::IpAddress::Any);

            const auto localHost = za::IpAddressUtils::resolve("localhost"_s);
            REQUIRE(localHost.hasValue());
            CHECK(za::IpAddressUtils::toString(*localHost) == "127.0.0.1"_s);
            CHECK(localHost->toInteger() == 0x7F'00'00'01);
            CHECK(*localHost == za::IpAddress::LocalHost);

            CHECK(!za::IpAddressUtils::resolve("255.255.255.256"_s).hasValue());
            CHECK(!za::IpAddressUtils::resolve("").hasValue());
        }

        SECTION("Byte constructor")
        {
            const za::IpAddress ipAddress(198, 51, 100, 234);
            CHECK(za::IpAddressUtils::toString(ipAddress) == "198.51.100.234"_s);
            CHECK(ipAddress.toInteger() == 0xC6'33'64'EA);
        }

        SECTION("_sf::zb::U32 constructor")
        {
            const za::IpAddress ipAddress(0xCB'00'71'9A);
            CHECK(za::IpAddressUtils::toString(ipAddress) == "203.0.113.154"_s);
            CHECK(ipAddress.toInteger() == 0xCB'00'71'9A);
        }
    }

    SECTION("Static functions")
    {
        // These functions require external network access to work thus imposing an additional
        // requirement on our test suite of internet access. This causes issues for developers
        // trying to work offline and for package managers who may be building and running the
        // tests offline as well.
        (void)([]
        {
            const zb::Optional<za::IpAddress> ipAddress = za::IpAddress::getLocalAddress();
            REQUIRE(ipAddress.hasValue());
            CHECK(za::IpAddressUtils::toString(*ipAddress) != "0.0.0.0");
            CHECK(ipAddress->toInteger() != 0);
        });

        (void)([]
        {
            const zb::Optional<za::IpAddress> ipAddress = za::IpAddress::getPublicAddress(za::milliseconds(250));
            if (ipAddress.hasValue())
            {
                CHECK(za::IpAddressUtils::toString(*ipAddress) != "0.0.0.0");
                CHECK(ipAddress->toInteger() != 0);
            }
        });
    }

    SECTION("Static constants")
    {
        CHECK(za::IpAddressUtils::toString(za::IpAddress::Any) == "0.0.0.0"_s);
        CHECK(za::IpAddress::Any.toInteger() == 0);

        CHECK(za::IpAddressUtils::toString(za::IpAddress::LocalHost) == "127.0.0.1"_s);
        CHECK(za::IpAddress::LocalHost.toInteger() == 0x7F'00'00'01);

        CHECK(za::IpAddressUtils::toString(za::IpAddress::Broadcast) == "255.255.255.255"_s);
        CHECK(za::IpAddress::Broadcast.toInteger() == 0xFF'FF'FF'FF);
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(za::IpAddress(0xC6, 0x33, 0x64, 0x7B) == za::IpAddress(0xC6'33'64'7B));
            CHECK(za::IpAddress(0xCB'00'71'D2) == za::IpAddress(203, 0, 113, 210));
        }

        SECTION("operator!=")
        {
            CHECK(za::IpAddress(0x12'34'43'21) != za::IpAddress(1234));
            CHECK(za::IpAddress(198, 51, 100, 1) != za::IpAddress(198, 51, 100, 11));
        }

        SECTION("operator<")
        {
            CHECK(za::IpAddress(1) < za::IpAddress(2));
            CHECK(za::IpAddress(0, 0, 0, 0) < za::IpAddress(1, 0, 0, 0));
            CHECK(za::IpAddress(0, 1, 0, 0) < za::IpAddress(1, 0, 0, 0));
            CHECK(za::IpAddress(0, 0, 1, 0) < za::IpAddress(0, 1, 0, 0));
            CHECK(za::IpAddress(0, 0, 0, 1) < za::IpAddress(0, 0, 1, 0));
            CHECK(za::IpAddress(0, 0, 0, 1) < za::IpAddress(1, 0, 0, 1));
        }

        SECTION("operator>")
        {
            CHECK(za::IpAddress(2) > za::IpAddress(1));
            CHECK(za::IpAddress(1, 0, 0, 0) > za::IpAddress(0, 0, 0, 0));
            CHECK(za::IpAddress(1, 0, 0, 0) > za::IpAddress(0, 1, 0, 0));
            CHECK(za::IpAddress(0, 1, 0, 0) > za::IpAddress(0, 0, 1, 0));
            CHECK(za::IpAddress(0, 0, 1, 0) > za::IpAddress(0, 0, 0, 1));
            CHECK(za::IpAddress(1, 0, 0, 1) > za::IpAddress(0, 0, 0, 1));
        }

        SECTION("operator<=")
        {
            CHECK(za::IpAddress(1) <= za::IpAddress(2));
            CHECK(za::IpAddress(0, 0, 0, 0) <= za::IpAddress(1, 0, 0, 0));
            CHECK(za::IpAddress(0, 1, 0, 0) <= za::IpAddress(1, 0, 0, 0));
            CHECK(za::IpAddress(0, 0, 1, 0) <= za::IpAddress(0, 1, 0, 0));
            CHECK(za::IpAddress(0, 0, 0, 1) <= za::IpAddress(0, 0, 1, 0));
            CHECK(za::IpAddress(0, 0, 0, 1) <= za::IpAddress(1, 0, 0, 1));

            CHECK(za::IpAddress(0xC6, 0x33, 0x64, 0x7B) <= za::IpAddress(0xC6'33'64'7B));
            CHECK(za::IpAddress(0xCB'00'71'D2) <= za::IpAddress(203, 0, 113, 210));
        }

        SECTION("operator>=")
        {
            CHECK(za::IpAddress(2) >= za::IpAddress(1));
            CHECK(za::IpAddress(1, 0, 0, 0) >= za::IpAddress(0, 0, 0, 0));
            CHECK(za::IpAddress(1, 0, 0, 0) >= za::IpAddress(0, 1, 0, 0));
            CHECK(za::IpAddress(0, 1, 0, 0) >= za::IpAddress(0, 0, 1, 0));
            CHECK(za::IpAddress(0, 0, 1, 0) >= za::IpAddress(0, 0, 0, 1));
            CHECK(za::IpAddress(1, 0, 0, 1) >= za::IpAddress(0, 0, 0, 1));

            CHECK(za::IpAddress(0xC6, 0x33, 0x64, 0x7B) >= za::IpAddress(0xC6'33'64'7B));
            CHECK(za::IpAddress(0xCB'00'71'D2) >= za::IpAddress(203, 0, 113, 210));
        }
    }
}
