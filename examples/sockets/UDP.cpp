////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "UDP.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"


////////////////////////////////////////////////////////////
/// Launch a server, wait for a message, send an answer.
///
////////////////////////////////////////////////////////////
void runUdpServer(unsigned short port)
{
    // Create a socket to receive a message from anyone
    sf::UdpSocket socket(/* isBlocking */ true);

    // Listen to messages on the specified port
    if (socket.bind(port) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Server is listening to port " << port << ", waiting for a message... " << sf::endL;

    // Wait for a message
    char                              in[128];
    sf::base::SizeT                   received = 0;
    sf::base::Optional<sf::IpAddress> sender;
    unsigned short                    senderPort = 0;
    if (socket.receive(in, sizeof(in), received, sender, senderPort) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Message received from client " << sf::IpAddressUtils::toString(sender.value()) << ": \"" << in << '"'
               << sf::endL;

    // Send an answer to the client
    const char out[] = "Hi, I'm the server";
    if (socket.send(out, sizeof(out), sender.value(), senderPort) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Message sent to the client: \"" << out << '"' << sf::endL;
}


////////////////////////////////////////////////////////////
/// Send a message to the server, wait for the answer
///
////////////////////////////////////////////////////////////
void runUdpClient(unsigned short port)
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
    sf::UdpSocket socket(/* isBlocking */ true);

    // Send a message to the server
    const char out[] = "Hi, I'm a client";
    if (socket.send(out, sizeof(out), server.value(), port) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Message sent to the server: \"" << out << '"' << sf::endL;

    // Receive an answer from anyone (but most likely from the server)
    char                              in[128];
    sf::base::SizeT                   received = 0;
    sf::base::Optional<sf::IpAddress> sender;
    unsigned short                    senderPort = 0;
    if (socket.receive(in, sizeof(in), received, sender, senderPort) != sf::Socket::Status::Done)
        return;
    sf::cOut() << "Message received from " << sf::IpAddressUtils::toString(sender.value()) << ": \"" << in << '"'
               << sf::endL;
}
