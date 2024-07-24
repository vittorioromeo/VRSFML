#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/SocketImpl.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <SFML/System/Err.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
TcpListener::TcpListener() : Socket(Type::Tcp)
{
}


////////////////////////////////////////////////////////////
unsigned short TcpListener::getLocalPort() const
{
    if (getNativeHandle() != priv::SocketImpl::invalidSocket())
    {
        // Retrieve information about the local end of the socket
        priv::SockAddrIn address{};
        auto             size = address.size();
        if (priv::SocketImpl::getSockName(getNativeHandle(), address, size))
        {
            return priv::SocketImpl::ntohs(address.sinPort());
        }
    }

    // We failed to retrieve the port
    return 0;
}


////////////////////////////////////////////////////////////
Socket::Status TcpListener::listen(unsigned short port, const IpAddress& address)
{
    // Close the socket if it is already bound
    close();

    // Create the internal socket if it doesn't exist
    create();

    // Check if the address is valid
    if (address == IpAddress::Broadcast)
        return Status::Error;

    // Bind the socket to the specified port
    priv::SockAddrIn addr = priv::SocketImpl::createAddress(address.toInteger(), port);
    if (!priv::SocketImpl::bind(getNativeHandle(), addr))
    {
        // Not likely to happen, but...
        priv::err() << "Failed to bind listener socket to port " << port;
        return Status::Error;
    }

    // Listen to the bound port
    if (!priv::SocketImpl::listen(getNativeHandle()))
    {
        // Oops, socket is deaf
        priv::err() << "Failed to listen to port " << port;
        return Status::Error;
    }

    return Status::Done;
}


////////////////////////////////////////////////////////////
void TcpListener::close()
{
    // Simply close the socket
    Socket::close();
}


////////////////////////////////////////////////////////////
Socket::Status TcpListener::accept(TcpSocket& socket)
{
    // Make sure that we're listening
    if (getNativeHandle() == priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Failed to accept a new connection, the socket is not listening";
        return Status::Error;
    }

    // Accept a new connection
    priv::SockAddrIn   address{};
    auto               length = address.size();
    const SocketHandle remote = priv::SocketImpl::accept(getNativeHandle(), address, length);

    // Check for errors
    if (remote == priv::SocketImpl::invalidSocket())
        return priv::SocketImpl::getErrorStatus();

    // Initialize the new connected socket
    socket.close();
    socket.create(remote);

    return Status::Done;
}

} // namespace sf
