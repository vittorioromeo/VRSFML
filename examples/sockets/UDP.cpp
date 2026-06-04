////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "UDP.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/UdpSocket.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"
#include "ZancleBase/Scn/ScnString.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"


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
    zb::printLn("Server is listening to port {}, waiting for a message... ", port);

    // Wait for a message
    char                        in[128];
    zb::SizeT                   received = 0;
    zb::Optional<za::IpAddress> sender;
    unsigned short              senderPort = 0;
    if (socket.receive(in, sizeof(in), received, sender, senderPort) != za::Socket::Status::Done)
        return;
    zb::printLn("Message received from client {}: \"{}{}", za::IpAddressUtils::toString(sender.value()), in, '"');

    // Send an answer to the client
    const char out[] = "Hi, I'm the server";
    if (socket.send(out, sizeof(out), sender.value(), senderPort) != za::Socket::Status::Done)
        return;
    zb::printLn("Message sent to the client: \"{}{}", out, '"');
}


////////////////////////////////////////////////////////////
/// Send a message to the server, wait for the answer
///
////////////////////////////////////////////////////////////
void runUdpClient(unsigned short port)
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
    auto socketOpt = za::UdpSocket::create(/* isBlocking */ true);
    if (!socketOpt.hasValue())
        return;

    auto& socket = *socketOpt;

    // Send a message to the server
    const char out[] = "Hi, I'm a client";
    if (socket.send(out, sizeof(out), server.value(), port) != za::Socket::Status::Done)
        return;
    zb::printLn("Message sent to the server: \"{}{}", out, '"');

    // Receive an answer from anyone (but most likely from the server)
    char                        in[128];
    zb::SizeT                   received = 0;
    zb::Optional<za::IpAddress> sender;
    unsigned short              senderPort = 0;
    if (socket.receive(in, sizeof(in), received, sender, senderPort) != za::Socket::Status::Done)
        return;
    zb::printLn("Message received from {}: \"{}{}", za::IpAddressUtils::toString(sender.value()), in, '"');
}
