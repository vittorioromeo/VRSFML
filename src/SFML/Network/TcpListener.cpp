// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/TcpListener.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/SocketHandle.hpp"
#include "SFML/Network/SocketImpl.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
TcpListener::TcpListener(SocketHandle handle, bool isBlocking) : Socket(Type::Tcp, handle, isBlocking)
{
}


////////////////////////////////////////////////////////////
base::Optional<TcpListener> TcpListener::create(unsigned short port, bool isBlocking, IpAddress address)
{
    if (address == IpAddress::Broadcast)
    {
        priv::err() << "Cannot create TCP listener bound to broadcast address";
        return base::nullOpt;
    }

    const SocketHandle handle = createTcpHandle(isBlocking);
    if (handle == priv::SocketImpl::invalidSocket())
        return base::nullOpt;

    priv::SockAddrIn addr = priv::SocketImpl::createAddress(address.toInteger(), port);

    if (!priv::SocketImpl::bind(handle, addr))
    {
        priv::err() << "Failed to bind listener socket to port " << port;
        priv::SocketImpl::close(handle);
        return base::nullOpt;
    }

    if (!priv::SocketImpl::listen(handle))
    {
        priv::err() << "Failed to listen on port " << port;
        priv::SocketImpl::close(handle);
        return base::nullOpt;
    }

    return base::makeOptionalFromFunc([&] { return TcpListener(handle, isBlocking); });
}


////////////////////////////////////////////////////////////
unsigned short TcpListener::getLocalPort() const
{
    return getLocalPortImpl("TCP listener");
}


////////////////////////////////////////////////////////////
TcpListener::AcceptResult TcpListener::accept()
{
    priv::SockAddrIn   address{};
    auto               length = address.size();
    const SocketHandle remote = priv::SocketImpl::accept(getNativeHandle(), address, length);

    if (remote == priv::SocketImpl::invalidSocket())
        return {priv::SocketImpl::getErrorStatus(), base::nullOpt};

    // Apply the same configuration as `Socket::createTcpHandle` to the
    // freshly-accepted handle: blocking mode, TCP_NODELAY, SO_NOSIGPIPE (macOS).
    const bool blocking = isBlocking();
    configureTcpHandle(remote, blocking);

    return {Status::Done, base::makeOptionalFromFunc([&] { return TcpSocket(remote, blocking); })};
}

} // namespace sf
