#include "StringifyIpAddressUtil.hpp"

#include "SFML/Network/UdpSocket.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/HasVirtualDestructor.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


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

    SECTION("Factory construction")
    {
        auto udpSocketOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(udpSocketOpt.hasValue());

        CHECK(udpSocketOpt->getLocalPort() == 0);
    }

    SECTION("bind()")
    {
        auto udpSocketOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(udpSocketOpt.hasValue());

        // Binding to the broadcast address is invalid.
        CHECK(udpSocketOpt->bind(sf::Socket::AnyPort, sf::IpAddress::Broadcast) == sf::Socket::Status::Error);
        CHECK(udpSocketOpt->getLocalPort() == 0);

        // Binding to `AnyPort` picks an available port.
        CHECK(udpSocketOpt->bind(sf::Socket::AnyPort) == sf::Socket::Status::Done);
        CHECK(udpSocketOpt->getLocalPort() != 0);
    }

    SECTION("send() after bind() (regression: factory refactor)")
    {
        // Regression test: prior to the factory refactor, calling `send` on a
        // UdpSocket that had already been bound failed with `Status::Error`,
        // because `send` unconditionally re-entered the lazy `create()` path
        // which rejected already-initialized sockets. The factory-based
        // design eliminates the lazy-create step entirely.

        auto senderOpt   = sf::UdpSocket::create(/* isBlocking */ true);
        auto receiverOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(senderOpt.hasValue());
        REQUIRE(receiverOpt.hasValue());

        // Bind both to ephemeral ports on the loopback address. Both must
        // succeed; `send` must still work afterwards on the bound sender.
        REQUIRE(senderOpt->bind(sf::Socket::AnyPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done);
        REQUIRE(receiverOpt->bind(sf::Socket::AnyPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done);

        const unsigned short receiverPort = receiverOpt->getLocalPort();
        REQUIRE(receiverPort != 0);

        // Make the receiver non-blocking so that any `receive` failure surfaces
        // quickly if the datagram never arrived.
        receiverOpt->setBlocking(false);

        const char payload[] = "ping";
        CHECK(senderOpt->send(payload, sizeof(payload), sf::IpAddress::LocalHost, receiverPort) == sf::Socket::Status::Done);

        // Poll a few times to ride out loopback scheduling jitter.
        char                              buffer[sizeof(payload)]{};
        sf::base::SizeT                   received{};
        sf::base::Optional<sf::IpAddress> sender;
        unsigned short                    senderPort{};

        sf::Socket::Status receiveStatus = sf::Socket::Status::NotReady;
        for (int attempt = 0; attempt < 100 && receiveStatus != sf::Socket::Status::Done; ++attempt)
            receiveStatus = receiverOpt->receive(buffer, sizeof(buffer), received, sender, senderPort);

        REQUIRE(receiveStatus == sf::Socket::Status::Done);
        CHECK(received == sizeof(payload));

        for (sf::base::SizeT i = 0; i < sizeof(payload); ++i)
            CHECK(buffer[i] == payload[i]);

        REQUIRE(sender.hasValue());
        CHECK(*sender == sf::IpAddress::LocalHost);
        CHECK(senderPort == senderOpt->getLocalPort());
    }
}
