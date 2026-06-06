////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "UDP.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/UdpSocket.hpp"

#include "Zancle/Scn/ScnStdin.hpp"
#include "Zancle/Scn/ScnString.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"

#include "Zancle/String/String.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
/// Launch a server, wait for a message, send an answer.
///
////////////////////////////////////////////////////////////
void runUdpServer(unsigned short port)
{
    // Create a socket to receive a message from anyone
    auto socketOpt = za::UdpSocket::create(/* isBlocking */ true);
    if (!socketOpt.hasValue())
        return;

    auto& socket = *socketOpt;

    // Listen to messages on the specified port
    if (socket.bind(port) != za::Socket::Status::Done)
        return;
    za::printLn("Server is listening to port {}, waiting for a message... ", port);

    // Wait for a message
    char                        in[128];
    za::SizeT                   received = 0;
    za::Optional<za::IpAddress> sender;
    unsigned short              senderPort = 0;
    if (socket.receive(in, sizeof(in), received, sender, senderPort) != za::Socket::Status::Done)
        return;
    za::printLn("Message received from client {}: \"{}{}", za::IpAddressUtils::toString(sender.value()), in, '"');

    // Send an answer to the client
    const char out[] = "Hi, I'm the server";
    if (socket.send(out, sizeof(out), sender.value(), senderPort) != za::Socket::Status::Done)
        return;
    za::printLn("Message sent to the client: \"{}{}", out, '"');
}


////////////////////////////////////////////////////////////
/// Send a message to the server, wait for the answer
///
////////////////////////////////////////////////////////////
void runUdpClient(unsigned short port)
{
    // Ask for the server address
    za::Optional<za::IpAddress> server;
    do
    {
        za::print("Type the address or name of the server to connect to: ");

        za::String addressStr;
        (void)za::scnStdinInto(addressStr);
        server = za::IpAddressUtils::resolve(addressStr);
    } while (!server.hasValue());

    // Create a socket for communicating with the server
    auto socketOpt = za::UdpSocket::create(/* isBlocking */ true);
    if (!socketOpt.hasValue())
        return;

    auto& socket = *socketOpt;

    // Send a message to the server
    const char out[] = "Hi, I'm a client";
    if (socket.send(out, sizeof(out), server.value(), port) != za::Socket::Status::Done)
        return;
    za::printLn("Message sent to the server: \"{}{}", out, '"');

    // Receive an answer from anyone (but most likely from the server)
    char                        in[128];
    za::SizeT                   received = 0;
    za::Optional<za::IpAddress> sender;
    unsigned short              senderPort = 0;
    if (socket.receive(in, sizeof(in), received, sender, senderPort) != za::Socket::Status::Done)
        return;
    za::printLn("Message received from {}: \"{}{}", za::IpAddressUtils::toString(sender.value()), in, '"');
}
