#include <SFML/Copyright.hpp>


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/SocketImpl.hpp"

#include "SFML/System/Win32/WindowsHeader.hpp"

#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>


namespace
{
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
} globalInitializer;
} // namespace


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
NetworkShort SockAddrIn::sinPort() const
{
    return m_impl->sin_port;
}


////////////////////////////////////////////////////////////
NetworkLong SockAddrIn::sAddr() const
{
    return m_impl->sin_addr.s_addr;
}


////////////////////////////////////////////////////////////
AddrLength SockAddrIn::size() const
{
    return sizeof(sockaddr_in);
}


////////////////////////////////////////////////////////////
struct FDSet::Impl
{
    fd_set set{};
};


////////////////////////////////////////////////////////////
FDSet::FDSet() = default;


////////////////////////////////////////////////////////////
FDSet::~FDSet() = default;


////////////////////////////////////////////////////////////
FDSet::FDSet(const FDSet&) = default;


////////////////////////////////////////////////////////////
FDSet& FDSet::operator=(const FDSet&) = default;


////////////////////////////////////////////////////////////
void* FDSet::asPtr()
{
    return &m_impl->set;
}


////////////////////////////////////////////////////////////
const void* FDSet::asPtr() const
{
    return &m_impl->set;
}


////////////////////////////////////////////////////////////
SockAddrIn SocketImpl::createAddress(base::U32 address, unsigned short port)
{
    auto addr            = sockaddr_in();
    addr.sin_addr.s_addr = ::htonl(address);
    addr.sin_family      = AF_INET;
    addr.sin_port        = ::htons(port);

    return addr;
}


////////////////////////////////////////////////////////////
base::U32 SocketImpl::inaddrAny()
{
    return INADDR_ANY;
}


////////////////////////////////////////////////////////////
base::U32 SocketImpl::inaddrLoopback()
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
unsigned long SocketImpl::getNtohl(unsigned long netlong)
{
    return ::ntohl(netlong);
}


////////////////////////////////////////////////////////////
unsigned short SocketImpl::getNtohs(unsigned short netshort)
{
    return ::ntohs(netshort);
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::getNtohl(SockAddrIn addr)
{
    return ::ntohl(addr.m_impl->sin_addr.s_addr);
}


////////////////////////////////////////////////////////////
unsigned short SocketImpl::getHtons(NetworkShort hostshort)
{
    return ::htons(hostshort);
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::getHtonl(NetworkLong hostlong)
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
    time.tv_sec  = static_cast<long>(timeoutUs / 1'000'000);
    time.tv_usec = static_cast<int>(timeoutUs % 1'000'000);

    // Wait for something to write on our socket (which means that the connection request has returned)
    return ::select(static_cast<int>(handle + 1), nullptr, &selector, nullptr, &time);
}


////////////////////////////////////////////////////////////
base::Optional<base::U32> SocketImpl::inetAddr(const char* data)
{
    const base::U32 ip = ::inet_addr(data);
    return ip == INADDR_NONE ? base::nullOpt : base::makeOptional<base::U32>(ip);
}


////////////////////////////////////////////////////////////
const char* SocketImpl::addrToString(base::U32 addr)
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
NetworkSSizeT SocketImpl::send(SocketHandle handle, const char* buf, SocketImpl::Size len, int flags)
{
    return ::send(handle, buf, len, flags);
}


////////////////////////////////////////////////////////////
NetworkSSizeT SocketImpl::sendTo(SocketHandle handle, const char* buf, SocketImpl::Size len, int flags, SockAddrIn& address)
{
    return ::sendto(handle, buf, len, flags, reinterpret_cast<sockaddr*>(&*address.m_impl), address.size());
}


////////////////////////////////////////////////////////////
NetworkSSizeT SocketImpl::recv(SocketHandle handle, char* buf, SocketImpl::Size len, int flags)
{
    return ::recv(handle, buf, len, flags);
}


////////////////////////////////////////////////////////////
NetworkSSizeT SocketImpl::recvFrom(SocketHandle handle, char* buf, SocketImpl::Size len, int flags, SockAddrIn& address, AddrLength& length)
{
    return ::recvfrom(handle, buf, len, flags, reinterpret_cast<sockaddr*>(&*address.m_impl), &length);
}


////////////////////////////////////////////////////////////
base::Optional<NetworkLong> SocketImpl::convertToHostname(const char* address)
{
    addrinfo hints{}; // Zero-initialize
    hints.ai_family = AF_INET;

    addrinfo* result = nullptr;
    if (getaddrinfo(address, nullptr, &hints, &result) == 0 && result != nullptr)
    {
        sockaddr_in sin{};
        SFML_BASE_MEMCPY(&sin, result->ai_addr, sizeof(*result->ai_addr));

        const base::U32 ip = sin.sin_addr.s_addr;
        freeaddrinfo(result);

        return base::makeOptional<NetworkLong>(ip);
    }

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
bool SocketImpl::fdIsSet(SocketHandle handle, const FDSet& fdSet)
{
    return FD_ISSET(handle, static_cast<const fd_set*>(fdSet.asPtr()));
}


////////////////////////////////////////////////////////////
void SocketImpl::fdClear(SocketHandle handle, FDSet& fdSet)
{
    FD_CLR(handle, static_cast<fd_set*>(fdSet.asPtr()));
}


////////////////////////////////////////////////////////////
void SocketImpl::fdZero(FDSet& fdSet)
{
    FD_ZERO(static_cast<fd_set*>(fdSet.asPtr()));
}


////////////////////////////////////////////////////////////
int SocketImpl::getFDSetSize()
{
    return FD_SETSIZE;
}


////////////////////////////////////////////////////////////
void SocketImpl::fdSet(SocketHandle handle, FDSet& fdSet)
{
    FD_SET(handle, static_cast<fd_set*>(fdSet.asPtr()));
}


////////////////////////////////////////////////////////////
int SocketImpl::select(int nfds, FDSet* readfds, FDSet* writefds, FDSet* exceptfds, long long timeoutUs)
{
    // Setup the timeout
    timeval time{};
    time.tv_sec  = static_cast<long>(timeoutUs / 1'000'000);
    time.tv_usec = static_cast<int>(timeoutUs % 1'000'000);

    return ::select(nfds,
                    static_cast<fd_set*>(readfds->asPtr()),
                    static_cast<fd_set*>(writefds->asPtr()),
                    static_cast<fd_set*>(exceptfds->asPtr()),
                    timeoutUs == 0ll ? nullptr : &time);
}


////////////////////////////////////////////////////////////
void SocketImpl::close(SocketHandle sock)
{
    SFML_BASE_ASSERT(sock != invalidSocket());
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
} // namespace sf::priv
