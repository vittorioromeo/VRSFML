////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

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
        sockaddr_in                  address{};
        priv::SocketImpl::AddrLength size = sizeof(address);
        if (priv::SocketImpl::getSockName(getNativeHandle(), address, size))
        {
            return priv::SocketImpl::ntohs(address.sin_port);
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
    sockaddr_in addr = priv::SocketImpl::createAddress(address.toInteger(), port);
    if (!priv::SocketImpl::bind(getNativeHandle(), addr))
    {
        // Not likely to happen, but...
        priv::err() << "Failed to bind listener socket to port " << port << priv::errEndl;
        return Status::Error;
    }

    // Listen to the bound port
    if (!priv::SocketImpl::listen(getNativeHandle()))
    {
        // Oops, socket is deaf
        priv::err() << "Failed to listen to port " << port << priv::errEndl;
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
        priv::err() << "Failed to accept a new connection, the socket is not listening" << priv::errEndl;
        return Status::Error;
    }

    // Accept a new connection
    sockaddr_in                  address{};
    priv::SocketImpl::AddrLength length = sizeof(address);
    const SocketHandle           remote = priv::SocketImpl::accept(getNativeHandle(), address, length);

    // Check for errors
    if (remote == priv::SocketImpl::invalidSocket())
        return priv::SocketImpl::getErrorStatus();

    // Initialize the new connected socket
    socket.close();
    socket.create(remote);

    return Status::Done;
}

} // namespace sf
