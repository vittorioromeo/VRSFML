#include <SFML/Network/TcpListener.hpp>

// Other 1st party headers
#include <SFML/Network/TcpSocket.hpp>

#include <Doctest.hpp>

#include <CommonTraits.hpp>

TEST_CASE("[Network] sf::TcpListener")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::TcpListener));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::TcpListener));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::TcpListener));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::TcpListener));
    }

    SECTION("Construction")
    {
        const sf::TcpListener tcpListener(/* isBlocking */ true);
        CHECK(tcpListener.getLocalPort() == 0);
    }

    SECTION("listen()")
    {
        sf::TcpListener tcpListener(/* isBlocking */ true);

        SECTION("Valid")
        {
            CHECK(tcpListener.listen(0) == sf::Socket::Status::Done);
            CHECK(tcpListener.getLocalPort() != 0);
        }

        SECTION("Invalid")
        {
            CHECK(tcpListener.listen(0, sf::IpAddress::Broadcast) == sf::Socket::Status::Error);
            CHECK(tcpListener.getLocalPort() == 0);
        }
    }

    SECTION("close()")
    {
        sf::TcpListener tcpListener(/* isBlocking */ true);
        CHECK(tcpListener.listen(0) == sf::Socket::Status::Done);
        CHECK(tcpListener.getLocalPort() != 0);
        CHECK(tcpListener.close());
        CHECK(tcpListener.getLocalPort() == 0);
    }

    SECTION("accept()")
    {
        sf::TcpListener tcpListener(/* isBlocking */ true);
        sf::TcpSocket   tcpSocket(/* isBlocking */ true);
        CHECK(tcpListener.accept(tcpSocket) == sf::Socket::Status::Error);
    }
}
