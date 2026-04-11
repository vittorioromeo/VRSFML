#include "StringifyIpAddressUtil.hpp"
#include "StringifyOptionalUtil.hpp"
#include "StringifySfBaseStringUtil.hpp"

#include "SFML/Network/SocketSelector.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::base::StringView commonName = "SFML TLS Example"; // Part of certificate


////////////////////////////////////////////////////////////
// Generated with: openssl ecparam -out key.pem -name secp384r1 -genkey
constexpr sf::base::StringView privateKey = R"(-----BEGIN EC PARAMETERS-----
BgUrgQQAIg==
-----END EC PARAMETERS-----
-----BEGIN EC PRIVATE KEY-----
MIGkAgEBBDD4ybj384qP07KjAqKoU4XwVyRH/Sy2kMYrtCr0SU/R0sCSxSPNwAXW
8Z5Z5CV/uDugBwYFK4EEACKhZANiAATpYJyfUQMS5YUOS8yaf2svRqebVAyqZcNh
lUKEP2OPwacLVul1OlW9QYJh4VFvtGvKyNEd1YCpBFUyASfeev8ZNBaLpMb20e3S
ztTSrGCF1rJynnEoGJrIh3trvRAKD0E=
-----END EC PRIVATE KEY-----)";


////////////////////////////////////////////////////////////
// Generated with: openssl req -new -key key.pem -x509 -nodes -days 36500 -sha384 -out cert.pem
// Country Name (2 letter code) [AU]:.
// State or Province Name (full name) [Some-State]:.
// Locality Name (eg, city) []:.
// Organization Name (eg, company) [Internet Widgits Pty Ltd]:.
// Organizational Unit Name (eg, section) []:.
// Common Name (e.g. server FQDN or YOUR name) []:SFML TLS Example
// Email Address []:.
constexpr sf::base::StringView certificate = R"(-----BEGIN CERTIFICATE-----
MIIByTCCAVCgAwIBAgIUKT3iSj7kJlvzxEGvfK1/yAYzRPcwCgYIKoZIzj0EAwMw
GzEZMBcGA1UEAwwQU0ZNTCBUTFMgRXhhbXBsZTAgFw0yNTA4MjMxMjI4MDJaGA8y
MTI1MDczMDEyMjgwMlowGzEZMBcGA1UEAwwQU0ZNTCBUTFMgRXhhbXBsZTB2MBAG
ByqGSM49AgEGBSuBBAAiA2IABOlgnJ9RAxLlhQ5LzJp/ay9Gp5tUDKplw2GVQoQ/
Y4/BpwtW6XU6Vb1BgmHhUW+0a8rI0R3VgKkEVTIBJ956/xk0FoukxvbR7dLO1NKs
YIXWsnKecSgYmsiHe2u9EAoPQaNTMFEwHQYDVR0OBBYEFImks65Kr96a8vcyjMgT
WJHWyPpQMB8GA1UdIwQYMBaAFImks65Kr96a8vcyjMgTWJHWyPpQMA8GA1UdEwEB
/wQFMAMBAf8wCgYIKoZIzj0EAwMDZwAwZAIwEwYg6elb6OPPkWaYdMAk4u05pngs
jN4bnBb1ChFgSeEnQmfOA1yOeAd0zUMiK5pGAjAdauGlH5+eE6939umuH6p3VMNc
RZlDlROT9eBnJ76WeMDiPMz+7E/oUdvGCAhuZb0=
-----END CERTIFICATE-----)";


////////////////////////////////////////////////////////////
[[nodiscard]] bool rangesAreEqual(auto itBegin, auto itEnd, auto it2Begin)
{
    for (auto it = itBegin; it != itEnd; ++it, ++it2Begin)
        if (*it != *it2Begin)
            return false;

    return true;
}

} // namespace


#ifdef SFML_RUN_LOOPBACK_TESTS

TEST_CASE("[Network] sf::Tcp Loopback")
{
    using Byte = unsigned char;

    constexpr sf::base::SizeT nTestDataBytes = 1024 * 1024;
    sf::base::Vector<Byte>    testData(nTestDataBytes);

    for (sf::base::SizeT i = 0u; i < nTestDataBytes; ++i)
        testData[i] = static_cast<Byte>(i % 256u);

    const auto* sendEnd = testData.data() + testData.size();

    sf::base::Vector<Byte> buffer(testData.size());
    const auto*            recvEnd = buffer.data() + buffer.size();

    sf::TcpListener tcpListener{/* isBlocking */ false};
    REQUIRE(tcpListener.listen(sf::Socket::AnyPort) == sf::TcpListener::Status::Done);

    const auto localPort = tcpListener.getLocalPort();
    CHECK_FALSE(localPort == 0);

    SECTION("Non-TLS")
    {
        sf::TcpSocket serverSocket{/* isBlocking */ true};
        sf::TcpSocket clientSocket{/* isBlocking */ false};

        CHECK(clientSocket.connect(sf::IpAddress(127, 0, 0, 1), localPort, sf::milliseconds(750)) ==
              sf::TcpSocket::Status::NotReady);

        auto start = sf::Clock::now();

        while (true)
        {
            const auto result = tcpListener.accept(serverSocket);

            REQUIRE(((result == sf::TcpListener::Status::NotReady) || (result == sf::TcpListener::Status::Done)));

            if (result == sf::TcpListener::Status::Done)
                break;

            REQUIRE((sf::Clock::now() - start < sf::milliseconds(750)));
        }

        serverSocket.setBlocking(false);
        CHECK(clientSocket.getRemoteAddress().hasValue());
        CHECK(serverSocket.getRemoteAddress().hasValue());
        CHECK_FALSE(clientSocket.getLocalPort() == 0);
        CHECK_FALSE(serverSocket.getLocalPort() == 0);
        CHECK_FALSE(clientSocket.getRemotePort() == 0);
        CHECK_FALSE(serverSocket.getRemotePort() == 0);

        CHECK(!serverSocket.getCurrentCiphersuiteName().hasValue());
        CHECK(!clientSocket.getCurrentCiphersuiteName().hasValue());

        const auto* sendPtr = testData.data();
        auto*       recvPtr = buffer.data();

        start = sf::Clock::now();

        while (true)
        {
            if (sendPtr != sendEnd)
            {
                sf::base::SizeT sent{};
                const auto status = serverSocket.send(sendPtr, static_cast<sf::base::SizeT>(sendEnd - sendPtr), sent);
                REQUIRE_FALSE(status == sf::TcpSocket::Status::Error);
                REQUIRE_FALSE(status == sf::TcpSocket::Status::Disconnected);
                sendPtr += sent;
            }
            else if (serverSocket.getRemoteAddress())
            {
                CHECK(serverSocket.disconnect());
            }

            {
                sf::base::SizeT received{};
                const auto status = clientSocket.receive(recvPtr, static_cast<sf::base::SizeT>(recvEnd - recvPtr), received);
                REQUIRE_FALSE(status == sf::TcpSocket::Status::Error);
                if (received > 0)
                    REQUIRE_FALSE(status == sf::TcpSocket::Status::Disconnected);
                recvPtr += received;

                if (status == sf::TcpSocket::Status::Disconnected)
                {
                    CHECK(clientSocket.disconnect());
                    break;
                }
            }

            REQUIRE((sf::Clock::now() - start < sf::milliseconds(750)));
        }

        CHECK(rangesAreEqual(buffer.begin(), buffer.end(), testData.begin()));
    }

    SECTION("TLS")
    {
        sf::TcpSocket serverSocket{/* isBlocking */ true};
        sf::TcpSocket clientSocket{/* isBlocking */ false};

        REQUIRE(clientSocket.connect(sf::IpAddress(127, 0, 0, 1), localPort, sf::milliseconds(750)) ==
                sf::TcpSocket::Status::NotReady);

        auto start = sf::Clock::now();

        while (true)
        {
            const auto result = tcpListener.accept(serverSocket);

            REQUIRE(((result == sf::TcpListener::Status::NotReady) || (result == sf::TcpListener::Status::Done)));

            if (result == sf::TcpListener::Status::Done)
                break;

            REQUIRE((sf::Clock::now() - start < sf::milliseconds(750)));
        }

        serverSocket.setBlocking(false);
        CHECK(clientSocket.getRemoteAddress().hasValue());
        CHECK(serverSocket.getRemoteAddress().hasValue());
        CHECK_FALSE(clientSocket.getLocalPort() == 0);
        CHECK_FALSE(serverSocket.getLocalPort() == 0);
        CHECK_FALSE(clientSocket.getRemotePort() == 0);
        CHECK_FALSE(serverSocket.getRemotePort() == 0);

        const auto* sendPtr = testData.data();
        auto*       recvPtr = buffer.data();

        start = sf::Clock::now();

        while (true)
        {
            const auto serverStatus = serverSocket.setupTlsServer(certificate, privateKey);

            REQUIRE_FALSE(serverStatus == sf::TcpSocket::TlsStatus::Error);
            REQUIRE_FALSE(serverStatus == sf::TcpSocket::TlsStatus::NotConnected);

            const auto clientStatus = clientSocket.setupTlsClient(commonName.to<sf::base::String>(), certificate);

            REQUIRE_FALSE(clientStatus == sf::TcpSocket::TlsStatus::Error);
            REQUIRE_FALSE(clientStatus == sf::TcpSocket::TlsStatus::NotConnected);

            if ((serverStatus == sf::TcpSocket::TlsStatus::HandshakeComplete) &&
                (clientStatus == sf::TcpSocket::TlsStatus::HandshakeComplete))
                break;

            REQUIRE((sf::Clock::now() - start < sf::milliseconds(750)));
        }

        CHECK(serverSocket.getCurrentCiphersuiteName().hasValue());
        CHECK(clientSocket.getCurrentCiphersuiteName().hasValue());
        CHECK(serverSocket.getCurrentCiphersuiteName() == clientSocket.getCurrentCiphersuiteName());

        start = sf::Clock::now();

        while (true)
        {
            if (sendPtr != sendEnd)
            {
                sf::base::SizeT sent{};
                const auto status = serverSocket.send(sendPtr, static_cast<sf::base::SizeT>(sendEnd - sendPtr), sent);
                REQUIRE_FALSE(status == sf::TcpSocket::Status::Error);
                REQUIRE_FALSE(status == sf::TcpSocket::Status::Disconnected);
                sendPtr += sent;
            }
            else if (serverSocket.getRemoteAddress())
            {
                CHECK(serverSocket.disconnect());
            }

            {
                sf::base::SizeT received{};
                const auto status = clientSocket.receive(recvPtr, static_cast<sf::base::SizeT>(recvEnd - recvPtr), received);
                REQUIRE_FALSE(status == sf::TcpSocket::Status::Error);
                if (received > 0)
                    REQUIRE_FALSE(status == sf::TcpSocket::Status::Disconnected);
                recvPtr += received;

                if (status == sf::TcpSocket::Status::Disconnected)
                {
                    CHECK(clientSocket.disconnect());
                    break;
                }
            }

            REQUIRE((sf::Clock::now() - start < sf::milliseconds(750)));
        }

        CHECK(rangesAreEqual(buffer.begin(), buffer.end(), testData.begin()));
    }
}

#endif
