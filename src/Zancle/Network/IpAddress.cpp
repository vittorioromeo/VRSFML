// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/IpAddress.hpp"

#include "Zancle/Network/Http.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"
#include "Zancle/Network/SocketHandle.hpp"
#include "Zancle/Network/SocketImpl.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/String/String.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/IntTypes.hpp"


namespace za
{
////////////////////////////////////////////////////////////
const IpAddress IpAddress::Any(0, 0, 0, 0);
const IpAddress IpAddress::LocalHost(127, 0, 0, 1);
const IpAddress IpAddress::Broadcast(255, 255, 255, 255);


////////////////////////////////////////////////////////////
IpAddress::IpAddress(za::U8 byte0, za::U8 byte1, za::U8 byte2, za::U8 byte3) :
    m_address(static_cast<za::U32>((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3))
{
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(za::U32 address) : m_address(address)
{
}


////////////////////////////////////////////////////////////
za::U32 IpAddress::toInteger() const
{
    return m_address;
}


////////////////////////////////////////////////////////////
za::Optional<IpAddress> IpAddress::getLocalAddress()
{
    // The method here is to connect a UDP socket to a public ip,
    // and get the local socket address with the getsockname function.
    // UDP connection will not send anything to the network, so this function won't cause any overhead.

    // Create the socket
    const SocketHandle sock = priv::SocketImpl::udpSocket();
    if (sock == priv::SocketImpl::invalidSocket())
    {
        priv::errMsg("Failed to retrieve local address (invalid socket)");
        return za::nullOpt;
    }

    // Connect the socket to a public ip (here 1.1.1.1) on any
    // port. This will give the local address of the network interface
    // used for default routing which is usually what we want.
    priv::SockAddrIn address = priv::SocketImpl::createAddress(0x01'01'01'01, 9);
    if (!priv::SocketImpl::connect(sock, address))
    {
        priv::SocketImpl::close(sock);

        priv::errMsg("Failed to retrieve local address (socket connection failure)");
        return za::nullOpt;
    }

    // Get the local address of the socket connection
    auto size = address.size();
    if (!priv::SocketImpl::getSockName(sock, address, size))
    {
        priv::SocketImpl::close(sock);

        priv::errMsg("Failed to retrieve local address (socket local address retrieval failure)");
        return za::nullOpt;
    }

    // Close the socket
    priv::SocketImpl::close(sock);

    // Finally build the IP address
    return za::makeOptional<IpAddress>(priv::SocketImpl::networkToHost(address.sAddr()));
}


////////////////////////////////////////////////////////////
za::Optional<IpAddress> IpAddress::getPublicAddress(Time timeout)
{
    // The trick here is more complicated, because the only way
    // to get our public IP address is to get it from a distant computer.
    // Here we get the web page from http://www.sfml-dev.org/ip-provider.php
    // and parse the result to extract our IP address
    // (not very hard: the web page contains only our IP address).

    Http server("www.sfml-dev.org");

    const Http::Request  request("/ip-provider.php", Http::Request::Method::Get);
    const Http::Response page = server.sendRequest(request, timeout);

    const Http::Response::Status status = page.getStatus();

    if (status == Http::Response::Status::Ok)
        return IpAddressUtils::resolve(page.getBody());

    priv::errMsg("Failed to retrieve public address from external IP resolution server (HTTP response status {})",
                 static_cast<int>(status));

    return za::nullOpt;
}


} // namespace za
