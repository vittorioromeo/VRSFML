#include <SFML/Copyright.hpp>

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/SocketImpl.hpp>

#include <SFML/System/Win32/WindowsHeader.hpp>

#include <SFML/Base/Optional.hpp>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdint>
#include <cstring>


namespace sf::priv
{
////////////////////////////////////////////////////////////
SockAddrIn::SockAddrIn() : m_impl(sockaddr_in{})
{
}


////////////////////////////////////////////////////////////
SockAddrIn::~SockAddrIn() = default;


////////////////////////////////////////////////////////////
SockAddrIn::SockAddrIn(const SockAddrIn&) = default;


////////////////////////////////////////////////////////////
SockAddrIn::SockAddrIn(const sockaddr_in& in) : m_impl(in)
{
}


////////////////////////////////////////////////////////////
unsigned short SockAddrIn::sinPort() const
{
    return m_impl->sin_port;
}


////////////////////////////////////////////////////////////
unsigned long SockAddrIn::sAddr() const
{
    return m_impl->sin_addr.s_addr;
}


////////////////////////////////////////////////////////////
AddrLength SockAddrIn::size() const
{
    return sizeof(sockaddr_in);
}


////////////////////////////////////////////////////////////
SockAddrIn SocketImpl::createAddress(std::uint32_t address, unsigned short port)
{
    auto addr            = sockaddr_in();
    addr.sin_addr.s_addr = ::htonl(address);
    addr.sin_family      = AF_INET;
    addr.sin_port        = ::htons(port);

    return addr;
}


////////////////////////////////////////////////////////////
std::uint32_t SocketImpl::inaddrAny()
{
    return INADDR_ANY;
}


////////////////////////////////////////////////////////////
std::uint32_t SocketImpl::inaddrLoopback()
{
    return INADDR_LOOPBACK;
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::accept(SocketHandle handle, SockAddrIn& address, AddrLength& length)
{
    return ::accept(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), &length);
}


////////////////////////////////////////////////////////////
bool SocketImpl::listen(SocketHandle handle)
{
    return ::listen(handle, SOMAXCONN) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::getSockName(SocketHandle handle, SockAddrIn& address, AddrLength& length)
{
    return ::getsockname(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), &length) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::getPeerName(SocketHandle handle, SockAddrIn& address, AddrLength& length)
{
    return ::getpeername(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), &length) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::bind(SocketHandle handle, SockAddrIn& address)
{
    return ::bind(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), address.size()) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::connect(SocketHandle handle, SockAddrIn& address)
{
    return ::connect(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), address.size()) != -1;
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
unsigned long SocketImpl::ntohl(SockAddrIn addr)
{
    return ::ntohl(addr.m_impl->sin_addr.s_addr);
}


////////////////////////////////////////////////////////////
unsigned short SocketImpl::htons(unsigned short hostshort)
{
    return ::htons(hostshort);
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::htonl(unsigned long hostlong)
{
    return ::htonl(hostlong);
}


////////////////////////////////////////////////////////////
int SocketImpl::select(SocketHandle handle, long long timeoutUs)
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
    return ::select(static_cast<int>(handle + 1), nullptr, &selector, nullptr, &time);
}


////////////////////////////////////////////////////////////
base::Optional<std::uint32_t> SocketImpl::inetAddr(const char* data)
{
    const std::uint32_t ip = ::inet_addr(data);

    if (ip == INADDR_NONE)
        return base::nullOpt;

    return base::makeOptional<std::uint32_t>(ip);
}


////////////////////////////////////////////////////////////
const char* SocketImpl::addrToString(std::uint32_t addr)
{
    in_addr address{};
    address.s_addr = addr;

    return inet_ntoa(address);
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::tcpSocket()
{
    return ::socket(PF_INET, SOCK_STREAM, 0);
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::udpSocket()
{
    return ::socket(PF_INET, SOCK_DGRAM, 0);
}


////////////////////////////////////////////////////////////
bool SocketImpl::disableNagle(SocketHandle handle)
{
    int yes = 1;
    return setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&yes), sizeof(yes)) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::disableSigpipe([[maybe_unused]] SocketHandle handle)
{
#ifdef SFML_SYSTEM_MACOS
    int yes = 1;
    return setsockopt(handle, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&yes), sizeof(yes)) != -1;
#else
    return true;
#endif
}

////////////////////////////////////////////////////////////
bool SocketImpl::enableBroadcast(SocketHandle handle)
{
    int yes = 1;
    return static_cast<bool>(setsockopt(handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes)) != -1);
}


////////////////////////////////////////////////////////////
int SocketImpl::send(SocketHandle handle, const char* buf, int len, int flags)
{
    return ::send(handle, buf, len, flags);
}


////////////////////////////////////////////////////////////
int SocketImpl::sendTo(SocketHandle handle, const char* buf, int len, int flags, SockAddrIn& address)
{
    return ::sendto(handle, buf, len, flags, reinterpret_cast<sockaddr*>(&*address.m_impl), address.size());
}

////////////////////////////////////////////////////////////
int SocketImpl::recv(SocketHandle handle, char* buf, int len, int flags)
{
    return ::recv(handle, buf, len, flags);
}


////////////////////////////////////////////////////////////
int SocketImpl::recvFrom(SocketHandle handle, char* buf, int len, int flags, SockAddrIn& address, AddrLength& length)
{
    return ::recvfrom(handle, buf, len, flags, reinterpret_cast<sockaddr*>(&*address.m_impl), &length);
}


////////////////////////////////////////////////////////////
base::Optional<unsigned long> SocketImpl::convertToHostname(const char* address)
{
    addrinfo hints{}; // Zero-initialize
    hints.ai_family = AF_INET;

    addrinfo* result = nullptr;
    if (getaddrinfo(address, nullptr, &hints, &result) == 0 && result != nullptr)
    {
        sockaddr_in sin{};
        std::memcpy(&sin, result->ai_addr, sizeof(*result->ai_addr));

        const std::uint32_t ip = sin.sin_addr.s_addr;
        freeaddrinfo(result);

        return base::makeOptional<unsigned long>(ip);
    }

    return base::nullOpt;
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
