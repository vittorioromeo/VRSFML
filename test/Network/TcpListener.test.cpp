#include "SFML/Network/TcpListener.hpp"

// Other 1st party headers
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


TEST_CASE("[Network] sf::TcpListener")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::TcpListener));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::TcpListener));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::TcpListener));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::TcpListener));
    }

    SECTION("Factory: valid port")
    {
        auto listenerOpt = sf::TcpListener::create(sf::Socket::AnyPort, /* isBlocking */ true);
        REQUIRE(listenerOpt.hasValue());

        CHECK(listenerOpt->getLocalPort() != 0);
    }

    SECTION("Factory: rejects broadcast address")
    {
        CHECK(!sf::TcpListener::create(sf::Socket::AnyPort, /* isBlocking */ true, sf::IpAddress::Broadcast).hasValue());
    }

    SECTION("accept() without a pending connection (non-blocking)")
    {
        auto listenerOpt = sf::TcpListener::create(sf::Socket::AnyPort, /* isBlocking */ false);
        REQUIRE(listenerOpt.hasValue());

        const auto result = listenerOpt->accept();
        CHECK(result.status == sf::Socket::Status::NotReady);
        CHECK(!result.socket.hasValue());
    }
}
