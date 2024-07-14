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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/SocketHandle.hpp>

#if defined(SFML_SYSTEM_WINDOWS)


#else

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstddef>

#endif

#include <cstdint>

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

namespace sf::priv
{
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
    using AddrLength = int;
    using Size       = int;
#else
    using AddrLength = socklen_t;
    using Size       = std::size_t;
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
    [[nodiscard]] static sockaddr_in createAddress(std::uint32_t address, unsigned short port);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle accept(SocketHandle handle, sockaddr_in& address, AddrLength& length);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool listen(SocketHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool getSockName(SocketHandle handle, sockaddr_in& address, AddrLength& length);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool getPeerName(SocketHandle handle, sockaddr_in& address, AddrLength& length);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool bind(SocketHandle handle, sockaddr_in& address);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool connect(SocketHandle handle, sockaddr_in& address);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned long ntohl(unsigned long netlong);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned short ntohs(unsigned short netshort);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned long ntohl(sockaddr_in addr);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool select(SocketHandle handle, long long timeoutUs);

    ////////////////////////////////////////////////////////////
    /// \brief Return the value of the invalid socket
    ///
    /// \return Special value of the invalid socket
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle invalidSocket();

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
};

} // namespace sf::priv
