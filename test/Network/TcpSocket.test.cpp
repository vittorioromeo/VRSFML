#include "SFML/Network/TcpSocket.hpp"

// Other 1st party headers
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/Trait/HasVirtualDestructor.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


TEST_CASE("[Network] sf::TcpSocket")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::TcpSocket));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::TcpSocket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::TcpSocket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::TcpSocket));
        STATIC_CHECK(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::TcpSocket));
    }

    SECTION("Construction")
    {
        const sf::TcpSocket tcpSocket(/* isBlocking */ true);
        CHECK(tcpSocket.getLocalPort() == 0);
        CHECK(!tcpSocket.getRemoteAddress().hasValue());
        CHECK(tcpSocket.getRemotePort() == 0);
        CHECK(!tcpSocket.getCurrentCiphersuiteName().hasValue());
    }
}


#ifdef SFML_RUN_CONNECTION_TESTS

TEST_CASE("[Network] sf::TcpSocket Connection")
{
    SECTION("Connection")
    {
        const auto githubAddress = sf::IpAddressUtils::resolve("github.com");
        REQUIRE(githubAddress.hasValue());

        sf::TcpSocket tcpSocket{/* isBlocking */ true};
        CHECK(tcpSocket.setupTlsServer("", "") == sf::TcpSocket::TlsStatus::NotConnected);
        CHECK(tcpSocket.setupTlsClient("") == sf::TcpSocket::TlsStatus::NotConnected);

        SECTION("Non-TLS")
        {
            CHECK(tcpSocket.connect(*githubAddress, 80, sf::milliseconds(1000)) == sf::TcpSocket::Status::Done);
            CHECK_FALSE(tcpSocket.getCurrentCiphersuiteName().hasValue());
        }

        SECTION("TLS")
        {
            CHECK(tcpSocket.connect(*githubAddress, 443, sf::milliseconds(1000)) == sf::TcpSocket::Status::Done);
            CHECK(tcpSocket.setupTlsClient("github.com") == sf::TcpSocket::TlsStatus::HandshakeComplete);

            SECTION("Ciphersuite")
            {
                REQUIRE(tcpSocket.getCurrentCiphersuiteName().hasValue());
                CHECK_FALSE(tcpSocket.getCurrentCiphersuiteName()->empty());
            }
        }
    }
}

#endif
