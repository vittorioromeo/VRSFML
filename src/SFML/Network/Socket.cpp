// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Socket.hpp"

#include "SFML/Network/SocketHandle.hpp"
#include "SFML/Network/SocketImpl.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Exchange.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Socket::Socket(Type type, SocketHandle handle, bool isBlocking) :
    m_type(type),
    m_socket(handle),
    m_isBlocking(isBlocking)
{
    SFML_BASE_ASSERT(handle != priv::SocketImpl::invalidSocket());
}


////////////////////////////////////////////////////////////
Socket::~Socket()
{
    if (m_socket != priv::SocketImpl::invalidSocket())
        priv::SocketImpl::close(m_socket);
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
        priv::SocketImpl::close(m_socket);

    m_type       = rhs.m_type;
    m_socket     = base::exchange(rhs.m_socket, priv::SocketImpl::invalidSocket());
    m_isBlocking = rhs.m_isBlocking;

    return *this;
}


////////////////////////////////////////////////////////////
void Socket::setBlocking(bool blocking)
{
    SFML_BASE_ASSERT(m_socket != priv::SocketImpl::invalidSocket() &&
                     "Socket handle must be valid (constructed via factory, not moved-from or disconnected)");

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
void Socket::configureTcpHandle(SocketHandle handle, bool isBlocking)
{
    SFML_BASE_ASSERT(handle != priv::SocketImpl::invalidSocket());

    priv::SocketImpl::setBlocking(handle, isBlocking);

    // Disable the Nagle algorithm (i.e. removes buffering of TCP packets)
    if (!priv::SocketImpl::disableNagle(handle))
        priv::err() << "Failed to set socket option \"TCP_NODELAY\"; packets will be buffered";

    // On macOS, disable the SIGPIPE signal on disconnection
#ifdef SFML_SYSTEM_MACOS
    if (!priv::SocketImpl::disableSigpipe(handle))
        priv::err() << "Failed to set socket option \"SO_NOSIGPIPE\"";
#endif
}


////////////////////////////////////////////////////////////
SocketHandle Socket::createTcpHandle(bool isBlocking)
{
    const SocketHandle handle = priv::SocketImpl::tcpSocket();
    if (handle == priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Failed to create TCP socket";
        return handle;
    }

    configureTcpHandle(handle, isBlocking);
    return handle;
}


////////////////////////////////////////////////////////////
SocketHandle Socket::createUdpHandle(bool isBlocking)
{
    const SocketHandle handle = priv::SocketImpl::udpSocket();
    if (handle == priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Failed to create UDP socket";
        return handle;
    }

    priv::SocketImpl::setBlocking(handle, isBlocking);

    // Enable broadcast by default for UDP sockets
    if (!priv::SocketImpl::enableBroadcast(handle))
        priv::err() << "Failed to enable broadcast on UDP socket";

    return handle;
}


////////////////////////////////////////////////////////////
void Socket::closeHandle()
{
    if (m_socket != priv::SocketImpl::invalidSocket())
    {
        priv::SocketImpl::close(m_socket);
        m_socket = priv::SocketImpl::invalidSocket();
    }
}


////////////////////////////////////////////////////////////
unsigned short Socket::getLocalPortImpl(const char* socketTypeStr) const
{
    SFML_BASE_ASSERT(m_socket != priv::SocketImpl::invalidSocket() &&
                     "Socket handle must be valid (constructed via factory, not moved-from or disconnected)");

    priv::SockAddrIn address{};
    auto             size = address.size();

    if (!priv::SocketImpl::getSockName(m_socket, address, size))
    {
        priv::err() << "Failed to retrieve local port of " << socketTypeStr;
        return 0;
    }

    return priv::SocketImpl::networkToHost(address.sinPort());
}

} // namespace sf
