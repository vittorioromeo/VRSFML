////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TCP.hpp"
#include "UDP.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose an arbitrary port for opening sockets
    const unsigned short port = 50'001;

    // TCP, UDP or connected UDP ?
    zb::print("Do you want to use TCP (t) or UDP (u)? ");
    const char protocol = zb::scnStdin<char>().valueOr('u');
    zb::scnStdinIgnoreLine();

    // Client or server ?
    zb::print("Do you want to be a server (s) or a client (c)? ");
    const char who = zb::scnStdin<char>().valueOr('c');
    zb::scnStdinIgnoreLine();

    if (protocol == 't')
    {
        // Enable TLS ?
        zb::print("Do you want to enable TLS (y) or not (n)? ");
        const char tls = zb::scnStdin<char>().valueOr('n');
        zb::scnStdinIgnoreLine();

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
    zb::printLn("Press enter to exit...");
    zb::scnStdinIgnoreLine();
    zb::scnStdinIgnoreLine();
}
