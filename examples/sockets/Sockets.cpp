////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TCP.hpp"
#include "UDP.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Scn/ScnStdin.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose an arbitrary port for opening sockets
    const unsigned short port = 50'001;

    // TCP, UDP or connected UDP ?
    sf::base::print("Do you want to use TCP (t) or UDP (u)? ");
    const char protocol = sf::base::scnStdin<char>().valueOr('u');
    sf::base::scnStdinIgnoreLine();

    // Client or server ?
    sf::base::print("Do you want to be a server (s) or a client (c)? ");
    const char who = sf::base::scnStdin<char>().valueOr('c');
    sf::base::scnStdinIgnoreLine();

    if (protocol == 't')
    {
        // Enable TLS ?
        sf::base::print("Do you want to enable TLS (y) or not (n)? ");
        const char tls = sf::base::scnStdin<char>().valueOr('n');
        sf::base::scnStdinIgnoreLine();

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
    sf::base::printLn("Press enter to exit...");
    sf::base::scnStdinIgnoreLine();
    sf::base::scnStdinIgnoreLine();
}
