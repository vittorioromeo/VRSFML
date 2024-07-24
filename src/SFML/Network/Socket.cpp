#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/SocketImpl.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Algorithm.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
Socket::Socket(Type type) : m_type(type), m_socket(priv::SocketImpl::invalidSocket())
{
}


////////////////////////////////////////////////////////////
Socket::~Socket()
{
    // Close the socket before it gets destructed
    close();
}


////////////////////////////////////////////////////////////
Socket::Socket(Socket&& socket) noexcept :
m_type(socket.m_type),
m_socket(base::exchange(socket.m_socket, priv::SocketImpl::invalidSocket())),
m_isBlocking(socket.m_isBlocking)
{
}


////////////////////////////////////////////////////////////
Socket& Socket::operator=(Socket&& socket) noexcept
{
    if (&socket == this)
        return *this;

    close();

    m_type       = socket.m_type;
    m_socket     = base::exchange(socket.m_socket, priv::SocketImpl::invalidSocket());
    m_isBlocking = socket.m_isBlocking;
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
void Socket::create()
{
    // Don't create the socket if it already exists
    if (m_socket == priv::SocketImpl::invalidSocket())
    {
        const SocketHandle handle = m_type == Type::Tcp ? priv::SocketImpl::tcpSocket() : priv::SocketImpl::udpSocket();

        if (handle == priv::SocketImpl::invalidSocket())
        {
            priv::err() << "Failed to create socket";
            return;
        }

        create(handle);
    }
}


////////////////////////////////////////////////////////////
void Socket::create(SocketHandle handle)
{
    // Don't create the socket if it already exists
    if (m_socket == priv::SocketImpl::invalidSocket())
    {
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
    }
}


////////////////////////////////////////////////////////////
void Socket::close()
{
    // Close the socket
    if (m_socket != priv::SocketImpl::invalidSocket())
    {
        priv::SocketImpl::close(m_socket);
        m_socket = priv::SocketImpl::invalidSocket();
    }
}

} // namespace sf
