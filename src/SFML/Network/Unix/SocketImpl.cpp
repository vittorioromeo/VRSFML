#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/SocketImpl.hpp>

#include <SFML/System/Err.hpp>

#include <fcntl.h>

#include <cerrno>


namespace sf::priv
{
////////////////////////////////////////////////////////////
SockAddrIn SocketImpl::createAddress(std::uint32_t address, unsigned short port)
{
    auto addr            = sockaddr_in();
    addr.sin_addr.s_addr = priv::SocketImpl::htonl(address);
    addr.sin_family      = AF_INET;
    addr.sin_port        = priv::SocketImpl::htons(port);

#if defined(SFML_SYSTEM_MACOS)
    addr.sin_len = sizeof(addr);
#endif

    return addr;
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::invalidSocket()
{
    return -1;
}


////////////////////////////////////////////////////////////
void SocketImpl::close(SocketHandle sock)
{
    ::close(sock);
}


////////////////////////////////////////////////////////////
void SocketImpl::setBlocking(SocketHandle sock, bool block)
{
    const int status = fcntl(sock, F_GETFL);
    if (block)
    {
        if (fcntl(sock, F_SETFL, status & ~O_NONBLOCK) == -1)
            priv::err() << "Failed to set file status flags: " << errno;
    }
    else
    {
        if (fcntl(sock, F_SETFL, status | O_NONBLOCK) == -1)
            priv::err() << "Failed to set file status flags: " << errno;
    }
}


////////////////////////////////////////////////////////////
Socket::Status SocketImpl::getErrorStatus()
{
    // The following are sometimes equal to EWOULDBLOCK,
    // so we have to make a special case for them in order
    // to avoid having double values in the switch case
    if ((errno == EAGAIN) || (errno == EINPROGRESS))
        return Socket::Status::NotReady;

    // clang-format off
    switch (errno)
    {
        case EWOULDBLOCK:  return Socket::Status::NotReady;
        case ECONNABORTED: return Socket::Status::Disconnected;
        case ECONNRESET:   return Socket::Status::Disconnected;
        case ETIMEDOUT:    return Socket::Status::Disconnected;
        case ENETRESET:    return Socket::Status::Disconnected;
        case ENOTCONN:     return Socket::Status::Disconnected;
        case EPIPE:        return Socket::Status::Disconnected;
        default:           return Socket::Status::Error;
    }
    // clang-format on
}

} // namespace sf::priv
