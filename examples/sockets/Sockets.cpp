////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TCP.hpp"
#include "UDP.hpp"

#include "Zancle/Scn/ScnStdin.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose an arbitrary port for opening sockets
    const unsigned short port = 50'001;

    // TCP, UDP or connected UDP ?
    za::print("Do you want to use TCP (t) or UDP (u)? ");
    const char protocol = za::scnStdin<char>().valueOr('u');
    za::scnStdinIgnoreLine();

    // Client or server ?
    za::print("Do you want to be a server (s) or a client (c)? ");
    const char who = za::scnStdin<char>().valueOr('c');
    za::scnStdinIgnoreLine();

    if (protocol == 't')
    {
        // Enable TLS ?
        za::print("Do you want to enable TLS (y) or not (n)? ");
        const char tls = za::scnStdin<char>().valueOr('n');
        za::scnStdinIgnoreLine();

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
    za::printLn("Press enter to exit...");
    za::scnStdinIgnoreLine();
    za::scnStdinIgnoreLine();
}
