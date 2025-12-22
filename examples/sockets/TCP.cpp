////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TCP.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"


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

} // namespace


////////////////////////////////////////////////////////////
/// Launch a server, wait for an incoming connection,
/// send a message and wait for the answer.
///
////////////////////////////////////////////////////////////
void runTcpServer(unsigned short port, const bool tls)
{
    // Create a server socket to accept new connections
    sf::TcpListener listener(/* isBlocking */ true);

    // Listen to the given port for incoming connections
    if (listener.listen(port) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Server is listening to port " << port << ", waiting for connections... " << sf::endL;

    // Wait for a connection
    sf::TcpSocket socket(/* isBlocking */ true);
    if (listener.accept(socket) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Client connected: " << sf::IpAddressUtils::toString(socket.getRemoteAddress().value()) << sf::endL;

    if (tls)
    {
        // Setup TLS
        if (socket.setupTlsServer(certificate, privateKey) != sf::TcpSocket::TlsStatus::HandshakeComplete)
        {
            sf::cOut() << "TLS handshake could not be completed" << sf::endL;
            return;
        }

        sf::cOut() << "TLS set up" << sf::endL;

        if (auto ciphersuite = socket.getCurrentCiphersuiteName(); ciphersuite)
            sf::cOut() << "Ciphersuite in use: " << *ciphersuite << sf::endL;
    }

    // Send a message to the connected client
    const char out[] = "Hi, I'm the server";
    if (socket.send(out, sizeof(out)) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Message sent to the client: \"" << out << '"' << sf::endL;

    // Receive a message back from the client
    char            in[128];
    sf::base::SizeT received = 0;
    if (socket.receive(in, sizeof(in), received) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Answer received from the client: \"" << in << '"' << sf::endL;
}


////////////////////////////////////////////////////////////
/// Create a client, connect it to a server, display the
/// welcome message and send an answer.
///
////////////////////////////////////////////////////////////
void runTcpClient(unsigned short port, const bool tls)
{
    // Ask for the server address
    sf::base::Optional<sf::IpAddress> server;
    do
    {
        sf::cOut() << "Type the address or name of the server to connect to: ";

        sf::base::String addressStr;
        sf::cIn() >> addressStr;
        server = sf::IpAddressUtils::resolve(addressStr);
    } while (!server.hasValue());

    // Create a socket for communicating with the server
    sf::TcpSocket socket(/* isBlocking */ true);

    // Connect to the server
    if (socket.connect(server.value(), port) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Connected to server " << sf::IpAddressUtils::toString(server.value()) << sf::endL;

    if (tls)
    {
        // Setup TLS
        if (socket.setupTlsClient(commonName.to<sf::base::String>(), certificate) !=
            sf::TcpSocket::TlsStatus::HandshakeComplete)
            return;
        sf::cOut() << "TLS set up" << sf::endL;

        if (auto ciphersuite = socket.getCurrentCiphersuiteName(); ciphersuite)
            sf::cOut() << "Ciphersuite in use: " << *ciphersuite << sf::endL;
    }

    // Receive a message from the server
    char            in[128];
    sf::base::SizeT received = 0;
    if (socket.receive(in, sizeof(in), received) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Message received from the server: \"" << in << '"' << sf::endL;

    // Send an answer to the server
    const char out[] = "Hi, I'm a client";
    if (socket.send(out, sizeof(out)) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Message sent to the server: \"" << out << '"' << sf::endL;
}
