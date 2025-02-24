#include "SFML/Network/IpAddress.hpp"

#include "SFML/Network/IpAddressUtils.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/StringView.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

#include <sstream>
#include <string>


using namespace sf::base::literals;
using namespace std::string_literals;

TEST_CASE("[Network] sf::IpAddress")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::IpAddress));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::IpAddress));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::IpAddress));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::IpAddress));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::IpAddress));
    }

    SECTION("Construction")
    {
        SECTION("static 'create' function")
        {
            const auto ipAddress = sf::IpAddressUtils::resolve("203.0.113.2"_sv);
            REQUIRE(ipAddress.hasValue());
            CHECK(sf::IpAddressUtils::toString(*ipAddress) == "203.0.113.2"s);
            CHECK(ipAddress->toInteger() == 0xCB'00'71'02);
            CHECK(*ipAddress != sf::IpAddress::Any);
            CHECK(*ipAddress != sf::IpAddress::Broadcast);
            CHECK(*ipAddress != sf::IpAddress::LocalHost);

            const auto broadcast = sf::IpAddressUtils::resolve("255.255.255.255"_sv);
            REQUIRE(broadcast.hasValue());
            CHECK(sf::IpAddressUtils::toString(*broadcast) == "255.255.255.255"s);
            CHECK(broadcast->toInteger() == 0xFF'FF'FF'FF);
            CHECK(*broadcast == sf::IpAddress::Broadcast);

            const auto any = sf::IpAddressUtils::resolve("0.0.0.0"_sv);
            REQUIRE(any.hasValue());
            CHECK(sf::IpAddressUtils::toString(*any) == "0.0.0.0"s);
            CHECK(any->toInteger() == 0x00'00'00'00);
            CHECK(*any == sf::IpAddress::Any);

            const auto localHost = sf::IpAddressUtils::resolve("localhost"s);
            REQUIRE(localHost.hasValue());
            CHECK(sf::IpAddressUtils::toString(*localHost) == "127.0.0.1"s);
            CHECK(localHost->toInteger() == 0x7F'00'00'01);
            CHECK(*localHost == sf::IpAddress::LocalHost);

            CHECK(!sf::IpAddressUtils::resolve("255.255.255.256"s).hasValue());
            CHECK(!sf::IpAddressUtils::resolve("").hasValue());
        }

        SECTION("Byte constructor")
        {
            const sf::IpAddress ipAddress(198, 51, 100, 234);
            CHECK(sf::IpAddressUtils::toString(ipAddress) == "198.51.100.234"s);
            CHECK(ipAddress.toInteger() == 0xC6'33'64'EA);
        }

        SECTION("sf::base::U32 constructor")
        {
            const sf::IpAddress ipAddress(0xCB'00'71'9A);
            CHECK(sf::IpAddressUtils::toString(ipAddress) == "203.0.113.154"s);
            CHECK(ipAddress.toInteger() == 0xCB'00'71'9A);
        }
    }

    SECTION("Static functions")
    {
        // These functions require external network access to work thus imposing an additional
        // requirement on our test suite of internet access. This causes issues for developers
        // trying to work offline and for package managers who may be building and running the
        // tests offline as well.
        (void)[]
        {
            const sf::base::Optional<sf::IpAddress> ipAddress = sf::IpAddress::getLocalAddress();
            REQUIRE(ipAddress.hasValue());
            CHECK(sf::IpAddressUtils::toString(*ipAddress) != "0.0.0.0");
            CHECK(ipAddress->toInteger() != 0);
        };

        (void)[]
        {
            const sf::base::Optional<sf::IpAddress> ipAddress = sf::IpAddress::getPublicAddress(sf::milliseconds(250));
            if (ipAddress.hasValue())
            {
                CHECK(sf::IpAddressUtils::toString(*ipAddress) != "0.0.0.0");
                CHECK(ipAddress->toInteger() != 0);
            }
        };
    }

    SECTION("Static constants")
    {
        CHECK(sf::IpAddressUtils::toString(sf::IpAddress::Any) == "0.0.0.0"s);
        CHECK(sf::IpAddress::Any.toInteger() == 0);

        CHECK(sf::IpAddressUtils::toString(sf::IpAddress::LocalHost) == "127.0.0.1"s);
        CHECK(sf::IpAddress::LocalHost.toInteger() == 0x7F'00'00'01);

        CHECK(sf::IpAddressUtils::toString(sf::IpAddress::Broadcast) == "255.255.255.255"s);
        CHECK(sf::IpAddress::Broadcast.toInteger() == 0xFF'FF'FF'FF);
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(sf::IpAddress(0xC6, 0x33, 0x64, 0x7B) == sf::IpAddress(0xC6'33'64'7B));
            CHECK(sf::IpAddress(0xCB'00'71'D2) == sf::IpAddress(203, 0, 113, 210));
        }

        SECTION("operator!=")
        {
            CHECK(sf::IpAddress(0x12'34'43'21) != sf::IpAddress(1234));
            CHECK(sf::IpAddress(198, 51, 100, 1) != sf::IpAddress(198, 51, 100, 11));
        }

        SECTION("operator<")
        {
            CHECK(sf::IpAddress(1) < sf::IpAddress(2));
            CHECK(sf::IpAddress(0, 0, 0, 0) < sf::IpAddress(1, 0, 0, 0));
            CHECK(sf::IpAddress(0, 1, 0, 0) < sf::IpAddress(1, 0, 0, 0));
            CHECK(sf::IpAddress(0, 0, 1, 0) < sf::IpAddress(0, 1, 0, 0));
            CHECK(sf::IpAddress(0, 0, 0, 1) < sf::IpAddress(0, 0, 1, 0));
            CHECK(sf::IpAddress(0, 0, 0, 1) < sf::IpAddress(1, 0, 0, 1));
        }

        SECTION("operator>")
        {
            CHECK(sf::IpAddress(2) > sf::IpAddress(1));
            CHECK(sf::IpAddress(1, 0, 0, 0) > sf::IpAddress(0, 0, 0, 0));
            CHECK(sf::IpAddress(1, 0, 0, 0) > sf::IpAddress(0, 1, 0, 0));
            CHECK(sf::IpAddress(0, 1, 0, 0) > sf::IpAddress(0, 0, 1, 0));
            CHECK(sf::IpAddress(0, 0, 1, 0) > sf::IpAddress(0, 0, 0, 1));
            CHECK(sf::IpAddress(1, 0, 0, 1) > sf::IpAddress(0, 0, 0, 1));
        }

        SECTION("operator<=")
        {
            CHECK(sf::IpAddress(1) <= sf::IpAddress(2));
            CHECK(sf::IpAddress(0, 0, 0, 0) <= sf::IpAddress(1, 0, 0, 0));
            CHECK(sf::IpAddress(0, 1, 0, 0) <= sf::IpAddress(1, 0, 0, 0));
            CHECK(sf::IpAddress(0, 0, 1, 0) <= sf::IpAddress(0, 1, 0, 0));
            CHECK(sf::IpAddress(0, 0, 0, 1) <= sf::IpAddress(0, 0, 1, 0));
            CHECK(sf::IpAddress(0, 0, 0, 1) <= sf::IpAddress(1, 0, 0, 1));

            CHECK(sf::IpAddress(0xC6, 0x33, 0x64, 0x7B) <= sf::IpAddress(0xC6'33'64'7B));
            CHECK(sf::IpAddress(0xCB'00'71'D2) <= sf::IpAddress(203, 0, 113, 210));
        }

        SECTION("operator>=")
        {
            CHECK(sf::IpAddress(2) >= sf::IpAddress(1));
            CHECK(sf::IpAddress(1, 0, 0, 0) >= sf::IpAddress(0, 0, 0, 0));
            CHECK(sf::IpAddress(1, 0, 0, 0) >= sf::IpAddress(0, 1, 0, 0));
            CHECK(sf::IpAddress(0, 1, 0, 0) >= sf::IpAddress(0, 0, 1, 0));
            CHECK(sf::IpAddress(0, 0, 1, 0) >= sf::IpAddress(0, 0, 0, 1));
            CHECK(sf::IpAddress(1, 0, 0, 1) >= sf::IpAddress(0, 0, 0, 1));

            CHECK(sf::IpAddress(0xC6, 0x33, 0x64, 0x7B) >= sf::IpAddress(0xC6'33'64'7B));
            CHECK(sf::IpAddress(0xCB'00'71'D2) >= sf::IpAddress(203, 0, 113, 210));
        }

        SECTION("operator>>")
        {
            sf::base::Optional<sf::IpAddress> ipAddress;
            std::istringstream("198.51.100.4") >> ipAddress;
            REQUIRE(ipAddress.hasValue());
            CHECK(sf::IpAddressUtils::toString(*ipAddress) == "198.51.100.4"s);
            CHECK(ipAddress->toInteger() == 0xC6'33'64'04);

            std::istringstream("203.0.113.72") >> ipAddress;
            REQUIRE(ipAddress.hasValue());
            CHECK(sf::IpAddressUtils::toString(*ipAddress) == "203.0.113.72"s);
            CHECK(ipAddress->toInteger() == 0xCB'00'71'48);

            std::istringstream("") >> ipAddress;
            CHECK(!ipAddress.hasValue());
        }

        SECTION("operator<<")
        {
            std::ostringstream out;
            out << sf::IpAddress(192, 0, 2, 10);
            CHECK(out.str() == "192.0.2.10"s);
        }
    }
}
