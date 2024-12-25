#include "SFML/Network/TcpSocket.hpp"

// Other 1st party headers
#include "SFML/Network/IpAddress.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

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
    }
}
