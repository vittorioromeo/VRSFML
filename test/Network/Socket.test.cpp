#include "SFML/Network/Socket.hpp"

#include "SFML/Network/UdpSocket.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Trait/HasVirtualDestructor.hpp"
#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


TEST_CASE("[Network] sf::Socket")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Socket));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Socket));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Socket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Socket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Socket));
        STATIC_CHECK(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::Socket));
    }

    SECTION("Constants")
    {
        STATIC_CHECK(sf::Socket::AnyPort == 0);
    }

    // `sf::Socket` is abstract (protected ctor); use `sf::UdpSocket` to exercise
    // the move/blocking behaviour inherited from the base.
    SECTION("Factory produces a valid socket")
    {
        auto socketOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(socketOpt.hasValue());

        CHECK(socketOpt->isBlocking());

        // Unbound: local port is 0 until `bind` is called.
        CHECK(socketOpt->getLocalPort() == 0);
    }

    SECTION("Set/get blocking")
    {
        auto socketOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(socketOpt.hasValue());

        socketOpt->setBlocking(false);
        CHECK(!socketOpt->isBlocking());
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            auto movedOpt = sf::UdpSocket::create(/* isBlocking */ true);
            REQUIRE(movedOpt.hasValue());

            movedOpt->setBlocking(false);


            const sf::UdpSocket socket(SFML_BASE_MOVE(*movedOpt));
            CHECK(!socket.isBlocking());
        }

        SECTION("Assignment")
        {
            auto movedOpt = sf::UdpSocket::create(/* isBlocking */ true);
            REQUIRE(movedOpt.hasValue());

            movedOpt->setBlocking(false);

            auto targetOpt = sf::UdpSocket::create(/* isBlocking */ true);
            REQUIRE(targetOpt.hasValue());

            *targetOpt = SFML_BASE_MOVE(*movedOpt);
            CHECK(!targetOpt->isBlocking());
        }
    }
}
