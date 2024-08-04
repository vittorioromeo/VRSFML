#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/IpAddressUtils.hpp>
#include <SFML/Network/SocketImpl.hpp>

#include <SFML/Base/Optional.hpp>

#include <string>

#include <cstring>


namespace sf
{
////////////////////////////////////////////////////////////
base::Optional<IpAddress> IpAddressUtils::resolve(std::string_view address)
{
    using namespace std::string_view_literals;

    if (address.empty())
    {
        // Not generating en error message here as resolution failure is a valid outcome.
        return base::nullOpt;
    }

    if (address == "255.255.255.255"sv)
    {
        // The broadcast address needs to be handled explicitly,
        // because it is also the value returned by inet_addr on error
        return sf::base::makeOptional(IpAddress::Broadcast);
    }

    if (address == "0.0.0.0"sv)
        return sf::base::makeOptional(IpAddress::Any);

    // Try to convert the address as a byte representation ("xxx.xxx.xxx.xxx")
    if (const auto ip = priv::SocketImpl::inetAddr(address.data()); ip.hasValue())
        return sf::base::makeOptional<IpAddress>(priv::SocketImpl::ntohl(*ip));

    // Not a valid address, try to convert it as a host name
    const base::Optional converted = priv::SocketImpl::convertToHostname(address.data());

    if (!converted.hasValue())
    {
        // Not generating en error message here as resolution failure is a valid outcome.
        return base::nullOpt;
    }

    return base::makeOptional<IpAddress>(priv::SocketImpl::ntohl(*converted));
}


////////////////////////////////////////////////////////////
std::string IpAddressUtils::toString(IpAddress ipAddress)
{
    return priv::SocketImpl::addrToString(ipAddress.m_address);
}

} // namespace sf
