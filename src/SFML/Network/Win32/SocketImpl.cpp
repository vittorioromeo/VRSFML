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

#include <SFML/System/Win32/WindowsHeader.hpp>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdint>


namespace sf::priv
{
////////////////////////////////////////////////////////////
sockaddr_in SocketImpl::createAddress(std::uint32_t address, unsigned short port)
{
    auto addr            = sockaddr_in();
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);

    return addr;
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::accept(SocketHandle handle, sockaddr_in& address, AddrLength& length)
{
    return ::accept(handle, reinterpret_cast<sockaddr*>(&address), &length);
}


////////////////////////////////////////////////////////////
bool SocketImpl::listen(SocketHandle handle)
{
    return ::listen(handle, SOMAXCONN) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::getSockName(SocketHandle handle, sockaddr_in& address, AddrLength& length)
{
    return ::getsockname(handle, reinterpret_cast<sockaddr*>(&address), &length) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::getPeerName(SocketHandle handle, sockaddr_in& address, AddrLength& length)
{
    return ::getpeername(handle, reinterpret_cast<sockaddr*>(&address), &length) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::bind(SocketHandle handle, sockaddr_in& address)
{
    return ::bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::connect(SocketHandle handle, sockaddr_in& address)
{
    return ::connect(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != -1;
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::invalidSocket()
{
    return INVALID_SOCKET;
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::ntohl(unsigned long netlong)
{
    return ::ntohl(netlong);
}


////////////////////////////////////////////////////////////
unsigned short SocketImpl::ntohs(unsigned short netshort)
{
    return ::ntohs(netshort);
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::ntohl(sockaddr_in addr)
{
    return ::ntohl(addr.sin_addr.s_addr);
}


////////////////////////////////////////////////////////////
bool SocketImpl::select(SocketHandle handle, long long timeoutUs)
{
    // Setup the selector
    fd_set selector;
    FD_ZERO(&selector);
    FD_SET(handle, &selector);

    // Setup the timeout
    timeval time{};
    time.tv_sec  = static_cast<long>(timeoutUs / 1000000);
    time.tv_usec = static_cast<int>(timeoutUs % 1000000);

    // Wait for something to write on our socket (which means that the connection request has returned)
    return ::select(static_cast<int>(handle + 1), nullptr, &selector, nullptr, &time) > 0;
}


////////////////////////////////////////////////////////////
void SocketImpl::close(SocketHandle sock)
{
    closesocket(sock);
}


////////////////////////////////////////////////////////////
void SocketImpl::setBlocking(SocketHandle sock, bool block)
{
    u_long blocking = block ? 0 : 1;
    ioctlsocket(sock, static_cast<long>(FIONBIO), &blocking);
}


////////////////////////////////////////////////////////////
Socket::Status SocketImpl::getErrorStatus()
{
    // clang-format off
    switch (WSAGetLastError())
    {
        case WSAEWOULDBLOCK:  return Socket::Status::NotReady;
        case WSAEALREADY:     return Socket::Status::NotReady;
        case WSAECONNABORTED: return Socket::Status::Disconnected;
        case WSAECONNRESET:   return Socket::Status::Disconnected;
        case WSAETIMEDOUT:    return Socket::Status::Disconnected;
        case WSAENETRESET:    return Socket::Status::Disconnected;
        case WSAENOTCONN:     return Socket::Status::Disconnected;
        case WSAEISCONN:      return Socket::Status::Done; // when connecting a non-blocking socket
        default:              return Socket::Status::Error;
    }
    // clang-format on
}


////////////////////////////////////////////////////////////
// Windows needs some initialization and cleanup to get
// sockets working properly... so let's create a class that will
// do it automatically
////////////////////////////////////////////////////////////
struct SocketInitializer
{
    SocketInitializer()
    {
        WSADATA init;
        WSAStartup(MAKEWORD(2, 2), &init);
    }

    ~SocketInitializer()
    {
        WSACleanup();
    }
};

SocketInitializer globalInitializer;

} // namespace sf::priv
