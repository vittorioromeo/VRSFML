#include "StringifyIpAddressUtil.hpp"
#include "StringifyOptionalUtil.hpp"
#include "StringifyZbStringUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/SocketSelector.hpp"
#include "Zancle/Network/TcpListener.hpp"
#include "Zancle/Network/TcpSocket.hpp"

#include "Zancle/System/Clock.hpp"
#include "Zancle/System/Time.hpp"
#include "Zancle/System/Utf8String.hpp"

#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr zb::StringView commonName = "Zancle TLS Example"; // Part of certificate


////////////////////////////////////////////////////////////
// Generated with: openssl ecparam -out key.pem -name secp384r1 -genkey
constexpr zb::StringView privateKey = R"(-----BEGIN EC PARAMETERS-----
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
constexpr zb::StringView certificate = R"(-----BEGIN CERTIFICATE-----
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


#ifdef ZA_RUN_LOOPBACK_TESTS

TEST_CASE("[Network] za::Tcp Loopback")
{
    using Byte = unsigned char;

    constexpr zb::SizeT nTestDataBytes = 1024 * 1024;
    zb::Vector<Byte>    testData(nTestDataBytes);

    for (zb::SizeT i = 0u; i < nTestDataBytes; ++i)
        testData[i] = static_cast<Byte>(i % 256u);

    const auto* sendEnd = testData.data() + testData.size();

    zb::Vector<Byte> buffer(testData.size());
    const auto*      recvEnd = buffer.data() + buffer.size();

    auto tcpListenerOpt = za::TcpListener::create(za::Socket::AnyPort, /* isBlocking */ false);
    REQUIRE(tcpListenerOpt.hasValue());

    auto& tcpListener = *tcpListenerOpt;

    const auto localPort = tcpListener.getLocalPort();
    CHECK_FALSE(localPort == 0);

    SECTION("Non-TLS")
    {
        auto clientSocketOpt = za::TcpSocket::create(/* isBlocking */ false);
        REQUIRE(clientSocketOpt.hasValue());

        auto& clientSocket = *clientSocketOpt;

        CHECK(clientSocket.connect(za::IpAddress(127, 0, 0, 1), localPort, za::milliseconds(750)) ==
              za::TcpSocket::Status::NotReady);

        auto start = za::Clock::now();

        zb::Optional<za::TcpSocket> serverSocketOpt;
        while (true)
        {
            auto result = tcpListener.accept();

            REQUIRE(((result.status == za::TcpListener::Status::NotReady) ||
                     (result.status == za::TcpListener::Status::Done)));

            if (result.status == za::TcpListener::Status::Done)
            {
                serverSocketOpt = ZB_MOVE(result.socket);
                break;
            }

            REQUIRE((za::Clock::now() - start < za::milliseconds(750)));
        }

        REQUIRE(serverSocketOpt.hasValue());
        auto& serverSocket = *serverSocketOpt;

        // The accepted connection already inherits the listener's non-blocking state.
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

        start = za::Clock::now();

        while (true)
        {
            if (sendPtr != sendEnd)
            {
                zb::SizeT  sent{};
                const auto status = serverSocket.send(sendPtr, static_cast<zb::SizeT>(sendEnd - sendPtr), sent);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Error);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Disconnected);
                sendPtr += sent;
            }
            else if (serverSocket.getRemoteAddress())
            {
                serverSocket.disconnect();
            }

            {
                zb::SizeT  received{};
                const auto status = clientSocket.receive(recvPtr, static_cast<zb::SizeT>(recvEnd - recvPtr), received);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Error);
                if (received > 0)
                    REQUIRE_FALSE(status == za::TcpSocket::Status::Disconnected);
                recvPtr += received;

                if (status == za::TcpSocket::Status::Disconnected)
                {
                    clientSocket.disconnect();
                    break;
                }
            }

            REQUIRE((za::Clock::now() - start < za::milliseconds(750)));
        }

        CHECK(rangesAreEqual(buffer.begin(), buffer.end(), testData.begin()));
    }

    SECTION("TLS")
    {
        auto clientSocketOpt = za::TcpSocket::create(/* isBlocking */ false);
        REQUIRE(clientSocketOpt.hasValue());
        auto& clientSocket = *clientSocketOpt;

        REQUIRE(clientSocket.connect(za::IpAddress(127, 0, 0, 1), localPort, za::milliseconds(750)) ==
                za::TcpSocket::Status::NotReady);

        auto start = za::Clock::now();

        zb::Optional<za::TcpSocket> serverSocketOpt;
        while (true)
        {
            auto result = tcpListener.accept();

            REQUIRE(((result.status == za::TcpListener::Status::NotReady) ||
                     (result.status == za::TcpListener::Status::Done)));

            if (result.status == za::TcpListener::Status::Done)
            {
                serverSocketOpt = ZB_MOVE(result.socket);
                break;
            }

            REQUIRE((za::Clock::now() - start < za::milliseconds(750)));
        }

        REQUIRE(serverSocketOpt.hasValue());
        auto& serverSocket = *serverSocketOpt;

        // The accepted connection already inherits the listener's non-blocking state.
        CHECK(clientSocket.getRemoteAddress().hasValue());
        CHECK(serverSocket.getRemoteAddress().hasValue());
        CHECK_FALSE(clientSocket.getLocalPort() == 0);
        CHECK_FALSE(serverSocket.getLocalPort() == 0);
        CHECK_FALSE(clientSocket.getRemotePort() == 0);
        CHECK_FALSE(serverSocket.getRemotePort() == 0);

        const auto* sendPtr = testData.data();
        auto*       recvPtr = buffer.data();

        start = za::Clock::now();

        while (true)
        {
            const auto serverStatus = serverSocket.setupTlsServer(certificate, privateKey);

            REQUIRE_FALSE(serverStatus == za::TcpSocket::TlsStatus::Error);
            REQUIRE_FALSE(serverStatus == za::TcpSocket::TlsStatus::NotConnected);

            const auto clientStatus = clientSocket.setupTlsClient(commonName.to<zb::String>(), certificate);

            REQUIRE_FALSE(clientStatus == za::TcpSocket::TlsStatus::Error);
            REQUIRE_FALSE(clientStatus == za::TcpSocket::TlsStatus::NotConnected);

            if ((serverStatus == za::TcpSocket::TlsStatus::HandshakeComplete) &&
                (clientStatus == za::TcpSocket::TlsStatus::HandshakeComplete))
                break;

            REQUIRE((za::Clock::now() - start < za::milliseconds(750)));
        }

        CHECK(serverSocket.getCurrentCiphersuiteName().hasValue());
        CHECK(clientSocket.getCurrentCiphersuiteName().hasValue());
        CHECK(serverSocket.getCurrentCiphersuiteName() == clientSocket.getCurrentCiphersuiteName());

        start = za::Clock::now();

        while (true)
        {
            if (sendPtr != sendEnd)
            {
                zb::SizeT  sent{};
                const auto status = serverSocket.send(sendPtr, static_cast<zb::SizeT>(sendEnd - sendPtr), sent);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Error);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Disconnected);
                sendPtr += sent;
            }
            else if (serverSocket.getRemoteAddress())
            {
                serverSocket.disconnect();
            }

            {
                zb::SizeT  received{};
                const auto status = clientSocket.receive(recvPtr, static_cast<zb::SizeT>(recvEnd - recvPtr), received);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Error);
                if (received > 0)
                    REQUIRE_FALSE(status == za::TcpSocket::Status::Disconnected);
                recvPtr += received;

                if (status == za::TcpSocket::Status::Disconnected)
                {
                    clientSocket.disconnect();
                    break;
                }
            }

            REQUIRE((za::Clock::now() - start < za::milliseconds(750)));
        }

        CHECK(rangesAreEqual(buffer.begin(), buffer.end(), testData.begin()));
    }
}

#endif
