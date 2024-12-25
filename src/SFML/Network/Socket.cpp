#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/SocketImpl.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Algorithm.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Socket::Socket(Type type, bool isBlocking) :
m_type(type),
m_socket(priv::SocketImpl::invalidSocket()),
m_isBlocking(isBlocking)
{
}


////////////////////////////////////////////////////////////
Socket::~Socket()
{
    if (m_socket != priv::SocketImpl::invalidSocket())
    {
        [[maybe_unused]] const bool rc = close();
        SFML_BASE_ASSERT(rc);
    }
}


////////////////////////////////////////////////////////////
Socket::Socket(Socket&& rhs) noexcept :
m_type(rhs.m_type),
m_socket(base::exchange(rhs.m_socket, priv::SocketImpl::invalidSocket())),
m_isBlocking(rhs.m_isBlocking)
{
}


////////////////////////////////////////////////////////////
Socket& Socket::operator=(Socket&& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    if (m_socket != priv::SocketImpl::invalidSocket())
    {
        [[maybe_unused]] const bool rc = close();
        SFML_BASE_ASSERT(rc);
    }

    m_type       = rhs.m_type;
    m_socket     = base::exchange(rhs.m_socket, priv::SocketImpl::invalidSocket());
    m_isBlocking = rhs.m_isBlocking;

    return *this;
}


////////////////////////////////////////////////////////////
void Socket::setBlocking(bool blocking)
{
    // Apply if the socket is already created
    if (m_socket != priv::SocketImpl::invalidSocket())
        priv::SocketImpl::setBlocking(m_socket, blocking);

    m_isBlocking = blocking;
}


////////////////////////////////////////////////////////////
bool Socket::isBlocking() const
{
    return m_isBlocking;
}


////////////////////////////////////////////////////////////
SocketHandle Socket::getNativeHandle() const
{
    return m_socket;
}


////////////////////////////////////////////////////////////
bool Socket::create()
{
    if (m_socket != priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Attempted to create a previously created socket";
        return false;
    }

    const SocketHandle handle = m_type == Type::Tcp ? priv::SocketImpl::tcpSocket() : priv::SocketImpl::udpSocket();

    if (handle == priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Failed to create socket";
        return false;
    }

    return create(handle);
}


////////////////////////////////////////////////////////////
bool Socket::create(SocketHandle handle)
{
    if (m_socket != priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Attempted to create a previously created socket (from handle)";
        return false;
    }

    // Assign the new handle
    m_socket = handle;

    // Set the current blocking state
    setBlocking(m_isBlocking);

    if (m_type == Type::Tcp)
    {
        // Disable the Nagle algorithm (i.e. removes buffering of TCP packets)
        if (!priv::SocketImpl::disableNagle(m_socket))
            priv::err() << "Failed to set socket option \"TCP_NODELAY\" ; all your TCP packets will be buffered";

// On macOS, disable the SIGPIPE signal on disconnection
#ifdef SFML_SYSTEM_MACOS
        if (!priv::SocketImpl::disableSigpipe(m_socket))
            priv::err() << "Failed to set socket option \"SO_NOSIGPIPE\"";
#endif
    }
    else
    {
        // Enable broadcast by default for UDP sockets
        if (!priv::SocketImpl::enableBroadcast(m_socket))
            priv::err() << "Failed to enable broadcast on UDP socket";
    }

    return true;
}


////////////////////////////////////////////////////////////
bool Socket::close()
{
    if (m_socket == priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Attempted to close an invalid socket";
        return false;
    }

    priv::SocketImpl::close(m_socket);
    m_socket = priv::SocketImpl::invalidSocket();

    return true;
}


////////////////////////////////////////////////////////////
unsigned short Socket::getLocalPortImpl(const char* socketTypeStr) const
{
    if (getNativeHandle() == priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Attempted to get local port of invalid " << socketTypeStr;
        return 0;
    }

    // Retrieve information about the local end of the socket
    priv::SockAddrIn address{};
    auto             size = address.size();

    if (!priv::SocketImpl::getSockName(getNativeHandle(), address, size))
    {
        priv::err() << "Failed to retrieve local port of" << socketTypeStr;
        return 0;
    }

    return priv::SocketImpl::getNtohs(address.sinPort());
}

} // namespace sf
