// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/TcpListener.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/SocketHandle.hpp"
#include "Zancle/Network/SocketImpl.hpp"
#include "Zancle/Network/TcpSocket.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Vocabulary/Optional.hpp"


namespace za
{
////////////////////////////////////////////////////////////
TcpListener::TcpListener(SocketHandle handle, bool isBlocking) : Socket(Type::Tcp, handle, isBlocking)
{
}


////////////////////////////////////////////////////////////
za::Optional<TcpListener> TcpListener::create(unsigned short port, bool isBlocking, IpAddress address)
{
    if (address == IpAddress::Broadcast)
    {
        priv::errMsg("Cannot create TCP listener bound to broadcast address");
        return za::nullOpt;
    }

    const SocketHandle handle = createTcpHandle(isBlocking);
    if (handle == priv::SocketImpl::invalidSocket())
        return za::nullOpt;

    priv::SockAddrIn addr = priv::SocketImpl::createAddress(address.toInteger(), port);

    if (!priv::SocketImpl::bind(handle, addr))
    {
        priv::errMsg("Failed to bind listener socket to port {}", port);
        priv::SocketImpl::close(handle);
        return za::nullOpt;
    }

    if (!priv::SocketImpl::listen(handle))
    {
        priv::errMsg("Failed to listen on port {}", port);
        priv::SocketImpl::close(handle);
        return za::nullOpt;
    }

    return za::makeOptionalFromFunc([&] { return TcpListener(handle, isBlocking); });
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
        return {priv::SocketImpl::getErrorStatus(), za::nullOpt};

    // Apply the same configuration as `Socket::createTcpHandle` to the
    // freshly-accepted handle: blocking mode, TCP_NODELAY, SO_NOSIGPIPE (macOS).
    const bool blocking = isBlocking();
    configureTcpHandle(remote, blocking);

    return {Status::Done, za::makeOptionalFromFunc([&] { return TcpSocket(remote, blocking); })};
}

} // namespace za
