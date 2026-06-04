////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TCP.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/TcpListener.hpp"
#include "Zancle/Network/TcpSocket.hpp"

#include "Zancle/System/Utf8String.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"
#include "ZancleBase/Scn/ScnString.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"


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

} // namespace


////////////////////////////////////////////////////////////
/// Launch a server, wait for an incoming connection,
/// send a message and wait for the answer.
///
////////////////////////////////////////////////////////////
void runTcpServer(unsigned short port, const bool tls)
{
    // Create a server socket that is already listening on `port`
    auto listenerOpt = za::TcpListener::create(port, /* isBlocking */ true);
    if (!listenerOpt.hasValue())
        return;

    zb::printLn("Server is listening to port {}, waiting for connections... ", port);

    // Wait for a connection
    auto acceptResult = listenerOpt->accept();
    if (acceptResult.status != za::Socket::Status::Done)
        return;

    auto& socket = *acceptResult.socket;
    zb::printLn("Client connected: {}", za::IpAddressUtils::toString(socket.getRemoteAddress().value()));

    if (tls)
    {
        // Setup TLS
        if (socket.setupTlsServer(certificate, privateKey) != za::TcpSocket::TlsStatus::HandshakeComplete)
        {
            zb::printLn("TLS handshake could not be completed");
            return;
        }

        zb::printLn("TLS set up");

        if (auto ciphersuite = socket.getCurrentCiphersuiteName(); ciphersuite)
            zb::printLn("Ciphersuite in use: {}", *ciphersuite);
    }

    // Send a message to the connected client
    const char out[] = "Hi, I'm the server";
    if (socket.send(out, sizeof(out)) != za::Socket::Status::Done)
        return;
    zb::printLn("Message sent to the client: \"{}{}", out, '"');

    // Receive a message back from the client
    char            in[128];
    zb::SizeT received = 0;
    if (socket.receive(in, sizeof(in), received) != za::Socket::Status::Done)
        return;
    zb::printLn("Answer received from the client: \"{}{}", in, '"');
}


////////////////////////////////////////////////////////////
/// Create a client, connect it to a server, display the
/// welcome message and send an answer.
///
////////////////////////////////////////////////////////////
void runTcpClient(unsigned short port, const bool tls)
{
    // Ask for the server address
    zb::Optional<za::IpAddress> server;
    do
    {
        zb::print("Type the address or name of the server to connect to: ");

        zb::String addressStr;
        (void)zb::scnStdinInto(addressStr);
        server = za::IpAddressUtils::resolve(addressStr);
    } while (!server.hasValue());

    // Create a socket for communicating with the server
    auto socketOpt = za::TcpSocket::create(/* isBlocking */ true);
    if (!socketOpt.hasValue())
        return;

    auto& socket = *socketOpt;

    // Connect to the server
    if (socket.connect(server.value(), port) != za::Socket::Status::Done)
        return;
    zb::printLn("Connected to server {}", za::IpAddressUtils::toString(server.value()));

    if (tls)
    {
        // Setup TLS
        if (socket.setupTlsClient(commonName.to<zb::String>(), certificate) !=
            za::TcpSocket::TlsStatus::HandshakeComplete)
            return;
        zb::printLn("TLS set up");

        if (auto ciphersuite = socket.getCurrentCiphersuiteName(); ciphersuite)
            zb::printLn("Ciphersuite in use: {}", *ciphersuite);
    }

    // Receive a message from the server
    char            in[128];
    zb::SizeT received = 0;
    if (socket.receive(in, sizeof(in), received) != za::Socket::Status::Done)
        return;
    zb::printLn("Message received from the server: \"{}{}", in, '"');

    // Send an answer to the server
    const char out[] = "Hi, I'm a client";
    if (socket.send(out, sizeof(out)) != za::Socket::Status::Done)
        return;
    zb::printLn("Message sent to the server: \"{}{}", out, '"');
}
