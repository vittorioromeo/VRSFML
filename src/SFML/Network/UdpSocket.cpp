// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/SocketImpl.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
UdpSocket::UdpSocket(bool isBlocking) : Socket(Type::Udp, isBlocking), m_buffer(MaxDatagramSize)
{
}


////////////////////////////////////////////////////////////
unsigned short UdpSocket::getLocalPort() const
{
    return getLocalPortImpl("UDP socket");
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::bind(unsigned short port, IpAddress address)
{
    // Close the socket if it is already bound
    if (getNativeHandle() != priv::SocketImpl::invalidSocket())
        (void)close(); // Intentionally discard

    // Create the internal socket if it doesn't exist
    if (!create())
        return Status::Error;

    // Check if the address is valid
    if (address == IpAddress::Broadcast)
        return Status::Error;

    // Bind the socket
    priv::SockAddrIn addr = priv::SocketImpl::createAddress(address.toInteger(), port);
    if (!priv::SocketImpl::bind(getNativeHandle(), addr))
    {
        priv::err() << "Failed to bind socket to port " << port;
        return Status::Error;
    }

    return Status::Done;
}


////////////////////////////////////////////////////////////
bool UdpSocket::unbind()
{
    return close();
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::send(const void* data, base::SizeT size, IpAddress remoteAddress, unsigned short remotePort)
{
    // Create the internal socket if it doesn't exist
    if (!create())
        return Status::Error;

    // Make sure that all the data will fit in one datagram
    if (size > MaxDatagramSize)
    {
        priv::err() << "Cannot send data over the network (the number of bytes to send is greater than "
                       "sf::UdpSocket::MaxDatagramSize)";

        return Status::Error;
    }

    // Build the target address
    priv::SockAddrIn address = priv::SocketImpl::createAddress(remoteAddress.toInteger(), remotePort);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
    // Send the data (unlike TCP, all the data is always sent in one call)
    const int sent = static_cast<int>(
        priv::SocketImpl::sendTo(getNativeHandle(),
                                 static_cast<const char*>(data),
                                 static_cast<priv::SocketImpl::Size>(size),
                                 0,
                                 address));
#pragma GCC diagnostic pop

    // Check for errors
    if (sent < 0)
        return priv::SocketImpl::getErrorStatus();

    return Status::Done;
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::receive(void*                      data,
                                  base::SizeT                size,
                                  base::SizeT&               received,
                                  base::Optional<IpAddress>& remoteAddress,
                                  unsigned short&            remotePort)
{
    // First clear the variables to fill
    received      = 0;
    remoteAddress = base::nullOpt;
    remotePort    = 0;

    // Check the destination buffer
    if (!data)
    {
        priv::err() << "Cannot receive data from the network (the destination buffer is invalid)";
        return Status::Error;
    }

    // Data that will be filled with the other computer's address
    priv::SockAddrIn address = priv::SocketImpl::createAddress(priv::SocketImpl::inaddrAny(), 0);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
    // Receive a chunk of bytes
    auto      addressSize  = address.size();
    const int sizeReceived = static_cast<int>(
        priv::SocketImpl::recvFrom(getNativeHandle(),
                                   static_cast<char*>(data),
                                   static_cast<priv::SocketImpl::Size>(size),
                                   0,
                                   address,
                                   addressSize));
#pragma GCC diagnostic pop

    // Check for errors
    if (sizeReceived < 0)
        return priv::SocketImpl::getErrorStatus();

    // Fill the sender information
    received = static_cast<base::SizeT>(sizeReceived);
    remoteAddress.emplace(priv::SocketImpl::getNtohl(address.sAddr()));
    remotePort = priv::SocketImpl::getNtohs(address.sinPort());

    return Status::Done;
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::send(Packet& packet, IpAddress remoteAddress, unsigned short remotePort)
{
    // UDP is a datagram-oriented protocol (as opposed to TCP which is a stream protocol).
    // Sending one datagram is almost safe: it may be lost but if it's received, then its data
    // is guaranteed to be ok. However, splitting a packet into multiple datagrams would be highly
    // unreliable, since datagrams may be reordered, dropped or mixed between different sources.
    // That's why SFML imposes a limit on packet size so that they can be sent in a single datagram.
    // This also removes the overhead associated to packets -- there's no size to send in addition
    // to the packet's data.

    // Get the data to send from the packet
    base::SizeT size = 0;
    const void* data = packet.onSend(size);

    // Send it
    return send(data, size, remoteAddress, remotePort);
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::receive(Packet& packet, base::Optional<IpAddress>& remoteAddress, unsigned short& remotePort)
{
    // See the detailed comment in send(Packet) above.

    // Receive the datagram
    base::SizeT  received = 0;
    const Status status   = receive(m_buffer.data(), m_buffer.size(), received, remoteAddress, remotePort);

    // If we received valid data, we can copy it to the user packet
    packet.clear();
    if ((status == Status::Done) && (received > 0))
        packet.onReceive(m_buffer.data(), received);

    return status;
}

} // namespace sf
