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


////////////////////////////////////////////////////////////
/// Launch a server, wait for an incoming connection,
/// send a message and wait for the answer.
///
////////////////////////////////////////////////////////////
void runTcpServer(unsigned short port)
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
void runTcpClient(unsigned short port)
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
