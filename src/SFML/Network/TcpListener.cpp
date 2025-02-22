#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/SocketImpl.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Err.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
TcpListener::TcpListener(bool isBlocking) : Socket(Type::Tcp, isBlocking)
{
}


////////////////////////////////////////////////////////////
unsigned short TcpListener::getLocalPort() const
{
    return getLocalPortImpl("TCP listener");
}


////////////////////////////////////////////////////////////
Socket::Status TcpListener::listen(unsigned short port, IpAddress address)
{
    // Close the socket if it is already bound
    if (getNativeHandle() != priv::SocketImpl::invalidSocket())
        (void)close(); // Intentionally discard

    // Create the internal socket if it doesn't exist
    if (!create())
        return Status::Error;

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
bool TcpListener::close()
{
    // Simply close the socket
    return Socket::close();
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
    if (socket.getNativeHandle() != priv::SocketImpl::invalidSocket())
        (void)socket.close(); // Intentionally discard

    if (socket.create(remote))
        return Status::Done;

    return Status::Error;
}

} // namespace sf
