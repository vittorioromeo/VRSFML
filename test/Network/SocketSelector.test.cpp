#include "SFML/Network/SocketSelector.hpp"

// Other 1st party headers
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


TEST_CASE("[Network] sf::SocketSelector")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SocketSelector));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SocketSelector));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SocketSelector));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SocketSelector));
    }

    auto socketOpt = sf::UdpSocket::create(/* isBlocking */ true);
    REQUIRE(socketOpt.hasValue());

    SECTION("Construction")
    {
        const sf::SocketSelector socketSelector;
        CHECK(!socketSelector.isReady(*socketOpt));
    }
}
