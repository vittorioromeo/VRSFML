#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Http.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/IpAddressUtils.hpp>
#include <SFML/Network/SocketImpl.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Optional.hpp>

#include <istream>

#include <cstring>


namespace sf
{
////////////////////////////////////////////////////////////
const IpAddress IpAddress::Any(0, 0, 0, 0);
const IpAddress IpAddress::LocalHost(127, 0, 0, 1);
const IpAddress IpAddress::Broadcast(255, 255, 255, 255);


////////////////////////////////////////////////////////////
IpAddress::IpAddress(std::uint8_t byte0, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3) :
m_address(priv::SocketImpl::htonl(static_cast<std::uint32_t>((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3)))
{
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(std::uint32_t address) : m_address(priv::SocketImpl::htonl(address))
{
}


////////////////////////////////////////////////////////////
std::uint32_t IpAddress::toInteger() const
{
    return priv::SocketImpl::ntohl(m_address);
}


////////////////////////////////////////////////////////////
base::Optional<IpAddress> IpAddress::getLocalAddress()
{
    // The method here is to connect a UDP socket to anyone (here to localhost),
    // and get the local socket address with the getsockname function.
    // UDP connection will not send anything to the network, so this function won't cause any overhead.

    // Create the socket
    const SocketHandle sock = priv::SocketImpl::udpSocket();
    if (sock == priv::SocketImpl::invalidSocket())
    {
        priv::err() << "Failed to retrieve local address (invalid socket)";
        return base::nullOpt;
    }

    // Connect the socket to localhost on any port
    priv::SockAddrIn address = priv::SocketImpl::createAddress(priv::SocketImpl::ntohl(priv::SocketImpl::inaddrLoopback()), 9);
    if (!priv::SocketImpl::connect(sock, address))
    {
        priv::SocketImpl::close(sock);

        priv::err() << "Failed to retrieve local address (socket connection failure)";
        return base::nullOpt;
    }

    // Get the local address of the socket connection
    auto size = address.size();
    if (!priv::SocketImpl::getSockName(sock, address, size))
    {
        priv::SocketImpl::close(sock);

        priv::err() << "Failed to retrieve local address (socket local address retrieval failure)";
        return base::nullOpt;
    }

    // Close the socket
    priv::SocketImpl::close(sock);

    // Finally build the IP address
    return sf::base::makeOptional<IpAddress>(priv::SocketImpl::ntohl(address.sAddr()));
}


////////////////////////////////////////////////////////////
base::Optional<IpAddress> IpAddress::getPublicAddress(Time timeout)
{
    // The trick here is more complicated, because the only way
    // to get our public IP address is to get it from a distant computer.
    // Here we get the web page from http://www.sfml-dev.org/ip-provider.php
    // and parse the result to extract our IP address
    // (not very hard: the web page contains only our IP address).

    Http                 server("www.sfml-dev.org");
    const Http::Request  request("/ip-provider.php", Http::Request::Method::Get);
    const Http::Response page = server.sendRequest(request, timeout);

    const Http::Response::Status status = page.getStatus();

    if (status == Http::Response::Status::Ok)
        return IpAddressUtils::resolve(page.getBody());

    priv::err() << "Failed to retrieve public address from external IP resolution server (HTTP response status "
                << static_cast<int>(status) << ")";

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
bool operator==(IpAddress left, IpAddress right)
{
    return !(left < right) && !(right < left);
}


////////////////////////////////////////////////////////////
bool operator!=(IpAddress left, IpAddress right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
bool operator<(IpAddress left, IpAddress right)
{
    return left.m_address < right.m_address;
}


////////////////////////////////////////////////////////////
bool operator>(IpAddress left, IpAddress right)
{
    return right < left;
}


////////////////////////////////////////////////////////////
bool operator<=(IpAddress left, IpAddress right)
{
    return !(right < left);
}


////////////////////////////////////////////////////////////
bool operator>=(IpAddress left, IpAddress right)
{
    return !(left < right);
}


////////////////////////////////////////////////////////////
std::istream& operator>>(std::istream& stream, base::Optional<IpAddress>& address)
{
    std::string str;
    stream >> str;
    address = IpAddressUtils::resolve(str);

    return stream;
}


////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& stream, IpAddress address)
{
    return stream << IpAddressUtils::toString(address);
}

} // namespace sf
