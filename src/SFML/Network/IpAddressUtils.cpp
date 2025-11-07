// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/IpAddressUtils.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/SocketImpl.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
base::Optional<IpAddress> IpAddressUtils::resolve(base::StringView address)
{
    using namespace base::literals;

    if (address.empty())
    {
        // Not generating en error message here as resolution failure is a valid outcome.
        return base::nullOpt;
    }

    if (address == "255.255.255.255"_sv)
    {
        // The broadcast address needs to be handled explicitly,
        // because it is also the value returned by inet_addr on error
        return base::makeOptional(IpAddress::Broadcast);
    }

    if (address == "0.0.0.0"_sv)
        return base::makeOptional(IpAddress::Any);

    // Try to convert the address as a byte representation ("xxx.xxx.xxx.xxx")
    if (const auto ip = priv::SocketImpl::inetAddr(address.data()); ip.hasValue())
        return base::makeOptional<IpAddress>(priv::SocketImpl::getNtohl(*ip));

    // Not a valid address, try to convert it as a host name
    const base::Optional converted = priv::SocketImpl::convertToHostname(address.data());

    if (!converted.hasValue())
    {
        // Not generating en error message here as resolution failure is a valid outcome.
        return base::nullOpt;
    }

    return base::makeOptional<IpAddress>(priv::SocketImpl::getNtohl(*converted));
}


////////////////////////////////////////////////////////////
base::String IpAddressUtils::toString(IpAddress ipAddress)
{
    return priv::SocketImpl::addrToString(priv::SocketImpl::getNtohl(ipAddress.m_address));
}

} // namespace sf
