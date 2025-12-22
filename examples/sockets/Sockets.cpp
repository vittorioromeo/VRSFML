////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TCP.hpp"
#include "UDP.hpp"

#include "SFML/System/IO.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose an arbitrary port for opening sockets
    const unsigned short port = 50'001;

    // TCP, UDP or connected UDP ?
    char protocol{};
    sf::cOut() << "Do you want to use TCP (t) or UDP (u)? ";
    sf::cIn() >> protocol;

    // Client or server ?
    char who{};
    sf::cOut() << "Do you want to be a server (s) or a client (c)? ";
    sf::cIn() >> who;

    if (protocol == 't')
    {
        // Enable TLS ?
        char tls{};
        sf::cOut() << "Do you want to enable TLS (y) or not (n)? ";
        sf::cIn() >> tls;

        // Test the TCP protocol
        if (who == 's')
            runTcpServer(port, tls == 'y');
        else
            runTcpClient(port, tls == 'y');
    }
    else
    {
        // Test the unconnected UDP protocol
        if (who == 's')
            runUdpServer(port);
        else
            runUdpClient(port);
    }

    // Wait until the user presses 'enter' key
    sf::cOut() << "Press enter to exit..." << sf::endL;
    sf::cIn().ignore(10'000, '\n');
    sf::cIn().ignore(10'000, '\n');
}
