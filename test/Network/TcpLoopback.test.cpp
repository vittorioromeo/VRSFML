#include "StringifyIpAddressUtil.hpp"
#include "StringifyOptionalUtil.hpp"
#include "StringifyZbStringUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/SocketSelector.hpp"
#include "Zancle/Network/TcpListener.hpp"
#include "Zancle/Network/TcpSocket.hpp"

#include "Zancle/Chrono/Clock.hpp"
#include "Zancle/Chrono/Time.hpp"
#include "Zancle/String/Utf8String.hpp"

#include "Zancle/Base/Macros.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"
#include "Zancle/Container/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr za::StringView commonName = "Zancle TLS Example"; // Part of certificate


////////////////////////////////////////////////////////////
// Generated with: openssl ecparam -out key.pem -name secp384r1 -genkey
constexpr za::StringView privateKey = R"(-----BEGIN EC PARAMETERS-----
BgUrgQQAIg==
-----END EC PARAMETERS-----
-----BEGIN EC PRIVATE KEY-----
MIGkAgEBBDBk0sxXYgP7M/WWZt2k3AQOVSkQ2GKi6EMRsUSQlq1EUF7P2IT2Fc41
FBNPwOIHiOmgBwYFK4EEACKhZANiAATEdENJ5ESMznhouovGEkkJewpuP+tl0TY7
sygE7cUa9t8/ZYiJcu0wcokzwTKPbh1daMGq9W9rRmDhcNaI5uX4kSa8PHN36MRA
G6dkijFhCxjblBRE0kewNxgie6tUecg=
-----END EC PRIVATE KEY-----)";


////////////////////////////////////////////////////////////
// Generated with:
//   openssl req -new -key key.pem -x509 -nodes -days 36500 -sha384 \
//               -subj '/CN=Zancle TLS Example' -out cert.pem
constexpr za::StringView certificate = R"(-----BEGIN CERTIFICATE-----
MIIBzTCCAVSgAwIBAgIUB+ctqlbZ2JNceThZn8qGl/s7riowCgYIKoZIzj0EAwMw
HTEbMBkGA1UEAwwSWmFuY2xlIFRMUyBFeGFtcGxlMCAXDTI2MDYwNjE2MjkwOVoY
DzIxMjYwNTEzMTYyOTA5WjAdMRswGQYDVQQDDBJaYW5jbGUgVExTIEV4YW1wbGUw
djAQBgcqhkjOPQIBBgUrgQQAIgNiAATEdENJ5ESMznhouovGEkkJewpuP+tl0TY7
sygE7cUa9t8/ZYiJcu0wcokzwTKPbh1daMGq9W9rRmDhcNaI5uX4kSa8PHN36MRA
G6dkijFhCxjblBRE0kewNxgie6tUecijUzBRMB0GA1UdDgQWBBSsz0pqwlk7Yt8a
+Q1XD2tgjh6zUTAfBgNVHSMEGDAWgBSsz0pqwlk7Yt8a+Q1XD2tgjh6zUTAPBgNV
HRMBAf8EBTADAQH/MAoGCCqGSM49BAMDA2cAMGQCMBwvi2koqbLo604lrPEaxeDN
40i0E09FrLwHkj9Ve1yFol7Pdsf15WtrA+Gh1XE9uAIwAQKpxAv8C6u1A0zAJinX
J+GKVnopO18y+yqKdqFZadjJxKdwKGUUJarjgBJNrn/j
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

    constexpr za::SizeT nTestDataBytes = 1024 * 1024;
    za::Vector<Byte>    testData(nTestDataBytes);

    for (za::SizeT i = 0u; i < nTestDataBytes; ++i)
        testData[i] = static_cast<Byte>(i % 256u);

    const auto* sendEnd = testData.data() + testData.size();

    za::Vector<Byte> buffer(testData.size());
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

        za::Optional<za::TcpSocket> serverSocketOpt;
        while (true)
        {
            auto result = tcpListener.accept();

            REQUIRE(((result.status == za::TcpListener::Status::NotReady) ||
                     (result.status == za::TcpListener::Status::Done)));

            if (result.status == za::TcpListener::Status::Done)
            {
                serverSocketOpt = ZA_MOVE(result.socket);
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
                za::SizeT  sent{};
                const auto status = serverSocket.send(sendPtr, static_cast<za::SizeT>(sendEnd - sendPtr), sent);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Error);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Disconnected);
                sendPtr += sent;
            }
            else if (serverSocket.getRemoteAddress())
            {
                serverSocket.disconnect();
            }

            {
                za::SizeT  received{};
                const auto status = clientSocket.receive(recvPtr, static_cast<za::SizeT>(recvEnd - recvPtr), received);
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

        za::Optional<za::TcpSocket> serverSocketOpt;
        while (true)
        {
            auto result = tcpListener.accept();

            REQUIRE(((result.status == za::TcpListener::Status::NotReady) ||
                     (result.status == za::TcpListener::Status::Done)));

            if (result.status == za::TcpListener::Status::Done)
            {
                serverSocketOpt = ZA_MOVE(result.socket);
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

            const auto clientStatus = clientSocket.setupTlsClient(commonName.to<za::String>(), certificate);

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
                za::SizeT  sent{};
                const auto status = serverSocket.send(sendPtr, static_cast<za::SizeT>(sendEnd - sendPtr), sent);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Error);
                REQUIRE_FALSE(status == za::TcpSocket::Status::Disconnected);
                sendPtr += sent;
            }
            else if (serverSocket.getRemoteAddress())
            {
                serverSocket.disconnect();
            }

            {
                za::SizeT  received{};
                const auto status = clientSocket.receive(recvPtr, static_cast<za::SizeT>(recvEnd - recvPtr), received);
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
