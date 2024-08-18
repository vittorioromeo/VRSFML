#include "SFML/Network/SocketSelector.hpp"

// Other 1st party headers
#include "SFML/Network/UdpSocket.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

TEST_CASE("[Network] sf::SocketSelector")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SocketSelector));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SocketSelector));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SocketSelector));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SocketSelector));
    }

    sf::UdpSocket socket(/* isBlocking */ true);

    SECTION("Construction")
    {
        const sf::SocketSelector socketSelector;
        CHECK(!socketSelector.isReady(socket));
    }
}
