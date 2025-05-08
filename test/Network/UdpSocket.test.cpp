#include "SFML/Network/UdpSocket.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

TEST_CASE("[Network] sf::UdpSocket")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::UdpSocket));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::UdpSocket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::UdpSocket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::UdpSocket));
        STATIC_CHECK(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::UdpSocket));
    }

    SECTION("Constants")
    {
        STATIC_CHECK(sf::UdpSocket::MaxDatagramSize == 65'507);
    }

    SECTION("Construction")
    {
        const sf::UdpSocket udpSocket(/* isBlocking */ true);
        CHECK(udpSocket.getLocalPort() == 0);
    }

    SECTION("bind()/unbind()")
    {
        sf::UdpSocket udpSocket(/* isBlocking */ true);
        CHECK(udpSocket.bind(sf::Socket::AnyPort, sf::IpAddress::Broadcast) == sf::Socket::Status::Error);
        CHECK(udpSocket.bind(sf::Socket::AnyPort) == sf::Socket::Status::Done);
        CHECK(udpSocket.getLocalPort() != 0);

        CHECK(udpSocket.unbind());
        CHECK(udpSocket.getLocalPort() == 0);
    }
}
