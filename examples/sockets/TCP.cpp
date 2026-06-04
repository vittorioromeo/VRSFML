////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TCP.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Utf8String.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Scn/ScnStdin.hpp"
#include "SFML/Base/Scn/ScnString.hpp"
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
    // Create a server socket that is already listening on `port`
    auto listenerOpt = sf::TcpListener::create(port, /* isBlocking */ true);
    if (!listenerOpt.hasValue())
        return;

    sf::base::printLn("Server is listening to port {}, waiting for connections... ", port);

    // Wait for a connection
    auto acceptResult = listenerOpt->accept();
    if (acceptResult.status != sf::Socket::Status::Done)
        return;

    auto& socket = *acceptResult.socket;
    sf::base::printLn("Client connected: {}", sf::IpAddressUtils::toString(socket.getRemoteAddress().value()));

    if (tls)
    {
        // Setup TLS
        if (socket.setupTlsServer(certificate, privateKey) != sf::TcpSocket::TlsStatus::HandshakeComplete)
        {
            sf::base::printLn("TLS handshake could not be completed");
            return;
        }

        sf::base::printLn("TLS set up");

        if (auto ciphersuite = socket.getCurrentCiphersuiteName(); ciphersuite)
            sf::base::printLn("Ciphersuite in use: {}", *ciphersuite);
    }

    // Send a message to the connected client
    const char out[] = "Hi, I'm the server";
    if (socket.send(out, sizeof(out)) != sf::Socket::Status::Done)
        return;
    sf::base::printLn("Message sent to the client: \"{}{}", out, '"');

    // Receive a message back from the client
    char            in[128];
    sf::base::SizeT received = 0;
    if (socket.receive(in, sizeof(in), received) != sf::Socket::Status::Done)
        return;
    sf::base::printLn("Answer received from the client: \"{}{}", in, '"');
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
        sf::base::print("Type the address or name of the server to connect to: ");

        sf::base::String addressStr;
        (void)sf::base::scnStdinInto(addressStr);
        server = sf::IpAddressUtils::resolve(addressStr);
    } while (!server.hasValue());

    // Create a socket for communicating with the server
    auto socketOpt = sf::TcpSocket::create(/* isBlocking */ true);
    if (!socketOpt.hasValue())
        return;

    auto& socket = *socketOpt;

    // Connect to the server
    if (socket.connect(server.value(), port) != sf::Socket::Status::Done)
        return;
    sf::base::printLn("Connected to server {}", sf::IpAddressUtils::toString(server.value()));

    if (tls)
    {
        // Setup TLS
        if (socket.setupTlsClient(commonName.to<sf::base::String>(), certificate) !=
            sf::TcpSocket::TlsStatus::HandshakeComplete)
            return;
        sf::base::printLn("TLS set up");

        if (auto ciphersuite = socket.getCurrentCiphersuiteName(); ciphersuite)
            sf::base::printLn("Ciphersuite in use: {}", *ciphersuite);
    }

    // Receive a message from the server
    char            in[128];
    sf::base::SizeT received = 0;
    if (socket.receive(in, sizeof(in), received) != sf::Socket::Status::Done)
        return;
    sf::base::printLn("Message received from the server: \"{}{}", in, '"');

    // Send an answer to the server
    const char out[] = "Hi, I'm a client";
    if (socket.send(out, sizeof(out)) != sf::Socket::Status::Done)
        return;
    sf::base::printLn("Message sent to the server: \"{}{}", out, '"');
}
