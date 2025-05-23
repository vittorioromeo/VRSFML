#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/SocketHandle.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"

#if defined(SFML_SYSTEM_WINDOWS)

// TODO P1: cleanup

#else

    #include "SFML/Base/SizeT.hpp"

    #include <sys/socket.h>
    #include <sys/types.h>

#endif

#ifdef htons
    #undef htons
#endif

#ifdef htonl
    #undef htonl
#endif


struct sockaddr_in;

/*
// NOLINTBEGIN
typedef struct in_addr
{
    union
    {
        struct
        {
            unsigned char s_b1, s_b2, s_b3, s_b4;
        } S_un_b;
        struct
        {
            unsigned short s_w1, s_w2;
        } S_un_w;
        unsigned long S_addr;
    } S_un;
} IN_ADDR, *PIN_ADDR, *LPIN_ADDR;

struct sockaddr_in
{
    short          sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char           sin_zero[8];
};
// NOLINTEND
*/

namespace sf::priv
{
////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)
using AddrLength    = int;
using NetworkLong   = unsigned long;
using NetworkShort  = unsigned short;
using NetworkSSizeT = int;
#else
using AddrLength    = socklen_t;
using NetworkLong   = base::U32;
using NetworkShort  = base::U16;
using NetworkSSizeT = ssize_t;
#endif

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class SockAddrIn
{
public:
    SockAddrIn();
    ~SockAddrIn();

    SockAddrIn(const SockAddrIn&);
    SockAddrIn(const sockaddr_in&);

    [[nodiscard]] NetworkShort sinPort() const;
    [[nodiscard]] NetworkLong  sAddr() const;

    [[nodiscard]] AddrLength size() const;

    base::InPlacePImpl<sockaddr_in, 64> m_impl;
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class FDSet
{
public:
    FDSet();
    ~FDSet();

    FDSet(const FDSet&);
    FDSet& operator=(const FDSet&);

    [[nodiscard]] void*       asPtr();
    [[nodiscard]] const void* asPtr() const;

private:
    struct Impl;
    base::InPlacePImpl<Impl, 768> m_impl;
};

////////////////////////////////////////////////////////////
/// \brief Helper class implementing all the non-portable
///        socket stuff
///
////////////////////////////////////////////////////////////
class SocketImpl
{
public:
////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)
    using Size = int;
#else
    using Size = base::SizeT;
#endif

    ////////////////////////////////////////////////////////////
    /// \brief Create an internal sockaddr_in address
    ///
    /// \param address Target address
    /// \param port    Target port
    ///
    /// \return sockaddr_in ready to be used by socket functions
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SockAddrIn createAddress(base::U32 address, unsigned short port);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::U32 inaddrAny();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::U32 inaddrLoopback();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle accept(SocketHandle handle, SockAddrIn& address, AddrLength& length);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool listen(SocketHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool getSockName(SocketHandle handle, SockAddrIn& address, AddrLength& length);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool getPeerName(SocketHandle handle, SockAddrIn& address, AddrLength& length);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool bind(SocketHandle handle, SockAddrIn& address);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool connect(SocketHandle handle, SockAddrIn& address);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkLong getNtohl(NetworkLong netlong);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkShort getNtohs(NetworkShort netshort);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkLong getNtohl(SockAddrIn addr);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkShort getHtons(NetworkShort hostshort);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkLong getHtonl(NetworkLong hostlong);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static int select(SocketHandle handle, long long timeoutUs);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<base::U32> inetAddr(const char* data);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static const char* addrToString(base::U32 addr);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle tcpSocket();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle udpSocket();

    ////////////////////////////////////////////////////////////
    /// \brief Return the value of the invalid socket
    ///
    /// \return Special value of the invalid socket
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle invalidSocket();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool disableNagle(SocketHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool disableSigpipe(SocketHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool enableBroadcast(SocketHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkSSizeT send(SocketHandle handle, const char* buf, SocketImpl::Size len, int flags);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkSSizeT sendTo(SocketHandle handle, const char* buf, SocketImpl::Size len, int flags, SockAddrIn& address);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkSSizeT recv(SocketHandle handle, char* buf, SocketImpl::Size len, int flags);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static NetworkSSizeT recvFrom(
        SocketHandle     handle,
        char*            buf,
        SocketImpl::Size len,
        int              flags,
        SockAddrIn&      address,
        AddrLength&      length);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<NetworkLong> convertToHostname(const char* address);

    ////////////////////////////////////////////////////////////
    /// \brief Close and destroy a socket
    ///
    /// \param sock Handle of the socket to close
    ///
    ////////////////////////////////////////////////////////////
    static void close(SocketHandle sock);

    ////////////////////////////////////////////////////////////
    /// \brief Set a socket as blocking or non-blocking
    ///
    /// \param sock  Handle of the socket
    /// \param block New blocking state of the socket
    ///
    ////////////////////////////////////////////////////////////
    static void setBlocking(SocketHandle sock, bool block);

    ////////////////////////////////////////////////////////////
    /// Get the last socket error status
    ///
    /// \return Status corresponding to the last socket error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Socket::Status getErrorStatus();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool fdIsSet(SocketHandle handle, const FDSet& fdSet);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    static void fdClear(SocketHandle handle, FDSet& fdSet);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    static void fdZero(FDSet& fdSet);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static int getFDSetSize();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    static void fdSet(SocketHandle handle, FDSet& fdSet);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static int select(int nfds, FDSet* readfds, FDSet* writefds, FDSet* exceptfds, long long timeoutUs);
};

} // namespace sf::priv
