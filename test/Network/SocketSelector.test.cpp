#include "SFML/Network/SocketSelector.hpp"

// Other 1st party headers
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


namespace
{
////////////////////////////////////////////////////////////
/// \brief Poll a predicate with bounded retries, yielding
///        via `wait(shortTimeout)` until it becomes true
///        or the deadline expires.
///
/// Useful to avoid depending on the exact timing of kernel
/// socket-ready notifications; loopback is fast but not
/// instantaneous.
///
////////////////////////////////////////////////////////////
template <typename Predicate>
[[nodiscard]] bool waitUntil(sf::SocketSelector& selector, const sf::Time overallTimeout, Predicate&& pred)
{
    const auto start = sf::Clock::now();
    while (sf::Clock::now() - start < overallTimeout)
    {
        (void)selector.wait(sf::milliseconds(25));
        if (pred())
            return true;
    }
    return false;
}

} // namespace


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
        const sf::SocketSelector selector;
        CHECK(!selector.isReady(*socketOpt));
    }

    SECTION("isReady before wait")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));
        // `isReady` must not report a socket as ready until `wait` has run.
        CHECK(!selector.isReady(*socketOpt));
    }

    SECTION("wait on empty selector times out quickly")
    {
        sf::SocketSelector selector;
        const auto         start = sf::Clock::now();
        CHECK(!selector.wait(sf::milliseconds(50)));
        const auto elapsed = sf::Clock::now() - start;
        // Must not block forever on an empty fd_set.
        CHECK(elapsed.asMilliseconds() < 500);
    }

    SECTION("wait with no traffic times out")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));

        const auto start = sf::Clock::now();
        CHECK(!selector.wait(sf::milliseconds(50)));
        const auto elapsed = sf::Clock::now() - start;
        CHECK(elapsed.asMilliseconds() >= 40);
        CHECK(elapsed.asMilliseconds() < 500);

        // After a timed-out wait, no socket should be ready.
        CHECK(!selector.isReady(*socketOpt));
    }

    SECTION("add returns true for valid socket")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));
    }

    SECTION("add is idempotent")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));
        CHECK(selector.add(*socketOpt)); // Adding again does not fail
    }

    SECTION("remove returns true for an added socket")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));
        CHECK(selector.remove(*socketOpt));
    }

    SECTION("remove is tolerant of never-added sockets")
    {
        sf::SocketSelector selector;
        // Removing something that was never added must not crash or error out.
        CHECK(selector.remove(*socketOpt));
    }

    SECTION("clear removes all sockets")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));
        selector.clear();

        // After clear, a timed-out wait must not report the socket as ready.
        CHECK(!selector.wait(sf::milliseconds(25)));
        CHECK(!selector.isReady(*socketOpt));
    }

    SECTION("re-add after clear")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));
        selector.clear();
        CHECK(selector.add(*socketOpt));
        // Re-added socket behaves like a freshly added one.
        CHECK(!selector.isReady(*socketOpt));
    }

    SECTION("copy-constructed selector is independent")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));

        sf::SocketSelector copy(selector);

        // Mutating one must not affect the other.
        selector.clear();
        CHECK(copy.remove(*socketOpt));
    }

    SECTION("move-constructed selector inherits state")
    {
        sf::SocketSelector selector;
        CHECK(selector.add(*socketOpt));

        sf::SocketSelector moved(SFML_BASE_MOVE(selector));
        // `moved` owns the registration; `wait` on it still times out (no traffic)
        // but must not error out.
        CHECK(!moved.wait(sf::milliseconds(25)));
    }
}


#ifdef SFML_RUN_LOOPBACK_TESTS

TEST_CASE("[Network] sf::SocketSelector (loopback)")
{
    SECTION("UDP: ready after datagram arrives")
    {
        auto receiverOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(receiverOpt.hasValue());

        auto& receiver = *receiverOpt;
        REQUIRE(receiver.bind(sf::Socket::AnyPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done);

        const auto receiverPort = receiver.getLocalPort();
        REQUIRE(receiverPort != 0);

        auto senderOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(senderOpt.hasValue());

        auto& sender = *senderOpt;

        sf::SocketSelector selector;
        REQUIRE(selector.add(receiver));

        // Nothing sent yet -- wait must time out.
        CHECK(!selector.wait(sf::milliseconds(50)));
        CHECK(!selector.isReady(receiver));

        const char payload[] = "hello";
        REQUIRE(sender.send(payload, sizeof(payload), sf::IpAddress::LocalHost, receiverPort) == sf::Socket::Status::Done);

        // Loopback delivery is fast but not instantaneous; allow a generous deadline.
        REQUIRE(waitUntil(selector, sf::milliseconds(500), [&] { return selector.isReady(receiver); }));

        // A ready receiver's `receive` must return immediately with the payload.
        char                              buffer[64]{};
        sf::base::SizeT                   received{};
        sf::base::Optional<sf::IpAddress> remoteAddress;
        unsigned short                    remotePort{};
        CHECK(receiver.receive(buffer, sizeof(buffer), received, remoteAddress, remotePort) == sf::Socket::Status::Done);
        CHECK(received == sizeof(payload));
    }

    SECTION("UDP: only the recipient is reported ready")
    {
        auto recvAOpt = sf::UdpSocket::create(/* isBlocking */ true);
        auto recvBOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(recvAOpt.hasValue());
        REQUIRE(recvBOpt.hasValue());

        auto& recvA = *recvAOpt;
        auto& recvB = *recvBOpt;

        REQUIRE(recvA.bind(sf::Socket::AnyPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done);
        REQUIRE(recvB.bind(sf::Socket::AnyPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done);
        REQUIRE(recvA.getLocalPort() != recvB.getLocalPort());

        auto senderOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(senderOpt.hasValue());

        sf::SocketSelector selector;
        REQUIRE(selector.add(recvA));
        REQUIRE(selector.add(recvB));

        const char payload[] = "x";
        REQUIRE(senderOpt->send(payload, sizeof(payload), sf::IpAddress::LocalHost, recvA.getLocalPort()) ==
                sf::Socket::Status::Done);

        REQUIRE(waitUntil(selector, sf::milliseconds(500), [&] { return selector.isReady(recvA); }));
        CHECK(!selector.isReady(recvB));
    }

    SECTION("UDP: removed socket is not reported ready")
    {
        auto receiverOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(receiverOpt.hasValue());
        auto& receiver = *receiverOpt;
        REQUIRE(receiver.bind(sf::Socket::AnyPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done);

        auto senderOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(senderOpt.hasValue());

        sf::SocketSelector selector;
        REQUIRE(selector.add(receiver));
        REQUIRE(selector.remove(receiver));

        const char payload[] = "x";
        REQUIRE(senderOpt->send(payload, sizeof(payload), sf::IpAddress::LocalHost, receiver.getLocalPort()) ==
                sf::Socket::Status::Done);

        // Even though the datagram arrived, the removed socket must never be reported ready.
        CHECK(!selector.wait(sf::milliseconds(100)));
        CHECK(!selector.isReady(receiver));
    }

    SECTION("TCP listener: ready when a connection arrives")
    {
        auto listenerOpt = sf::TcpListener::create(sf::Socket::AnyPort, /* isBlocking */ false);
        REQUIRE(listenerOpt.hasValue());

        auto&      listener     = *listenerOpt;
        const auto listenerPort = listener.getLocalPort();
        REQUIRE(listenerPort != 0);

        sf::SocketSelector selector;
        REQUIRE(selector.add(listener));

        // Nothing pending -- wait times out.
        CHECK(!selector.wait(sf::milliseconds(50)));
        CHECK(!selector.isReady(listener));

        auto clientOpt = sf::TcpSocket::create(/* isBlocking */ false);
        REQUIRE(clientOpt.hasValue());
        // Kick off a non-blocking connect; the three-way handshake happens asynchronously.
        const auto status = clientOpt->connect(sf::IpAddress(127, 0, 0, 1), listenerPort, sf::milliseconds(750));
        REQUIRE(((status == sf::Socket::Status::Done) || (status == sf::Socket::Status::NotReady)));

        REQUIRE(waitUntil(selector, sf::milliseconds(500), [&] { return selector.isReady(listener); }));

        auto acceptResult = listener.accept();
        CHECK(acceptResult.status == sf::Socket::Status::Done);
    }

    SECTION("TCP connected peers: receiver becomes ready on send")
    {
        auto listenerOpt = sf::TcpListener::create(sf::Socket::AnyPort, /* isBlocking */ false);
        REQUIRE(listenerOpt.hasValue());

        auto&      listener     = *listenerOpt;
        const auto listenerPort = listener.getLocalPort();

        auto clientOpt = sf::TcpSocket::create(/* isBlocking */ false);
        REQUIRE(clientOpt.hasValue());
        auto& client = *clientOpt;

        (void)client.connect(sf::IpAddress(127, 0, 0, 1), listenerPort, sf::milliseconds(750));

        // Drive the accept loop until we have the accepted server-side socket.
        sf::base::Optional<sf::TcpSocket> serverSocketOpt;
        const auto                        acceptStart = sf::Clock::now();

        while (sf::Clock::now() - acceptStart < sf::milliseconds(750))
        {
            auto r = listener.accept();
            if (r.status == sf::Socket::Status::Done)
            {
                serverSocketOpt = SFML_BASE_MOVE(r.socket);
                break;
            }
        }

        REQUIRE(serverSocketOpt.hasValue());
        auto& serverSocket = *serverSocketOpt;

        sf::SocketSelector selector;
        REQUIRE(selector.add(client));

        // Nothing sent yet -- client must not be ready.
        CHECK(!selector.wait(sf::milliseconds(50)));
        CHECK(!selector.isReady(client));

        const char      payload[] = "ping";
        sf::base::SizeT sent{};
        REQUIRE(serverSocket.send(payload, sizeof(payload), sent) == sf::Socket::Status::Done);
        CHECK(sent == sizeof(payload));

        REQUIRE(waitUntil(selector, sf::milliseconds(500), [&] { return selector.isReady(client); }));

        char            buffer[64]{};
        sf::base::SizeT received{};
        CHECK(client.receive(buffer, sizeof(buffer), received) == sf::Socket::Status::Done);
        CHECK(received == sizeof(payload));
    }

    SECTION("multiple waits on the same selector remain consistent")
    {
        auto receiverOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(receiverOpt.hasValue());

        auto& receiver = *receiverOpt;
        REQUIRE(receiver.bind(sf::Socket::AnyPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done);

        sf::SocketSelector selector;
        REQUIRE(selector.add(receiver));

        // First wait: empty, times out.
        CHECK(!selector.wait(sf::milliseconds(25)));
        CHECK(!selector.isReady(receiver));

        // Second wait, still empty.
        CHECK(!selector.wait(sf::milliseconds(25)));
        CHECK(!selector.isReady(receiver));

        // Now send and make sure a subsequent wait flips readiness on.
        auto senderOpt = sf::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(senderOpt.hasValue());
        const char payload[] = "tick";
        REQUIRE(senderOpt->send(payload, sizeof(payload), sf::IpAddress::LocalHost, receiver.getLocalPort()) ==
                sf::Socket::Status::Done);

        REQUIRE(waitUntil(selector, sf::milliseconds(500), [&] { return selector.isReady(receiver); }));

        // Drain the datagram so we return to a "no data pending" state.
        char                              buffer[64]{};
        sf::base::SizeT                   received{};
        sf::base::Optional<sf::IpAddress> remoteAddress;
        unsigned short                    remotePort{};
        CHECK(receiver.receive(buffer, sizeof(buffer), received, remoteAddress, remotePort) == sf::Socket::Status::Done);

        // After draining, a new wait should time out again.
        CHECK(!selector.wait(sf::milliseconds(50)));
        CHECK(!selector.isReady(receiver));
    }
}

#endif
