// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/IpAddressUtils.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/SocketImpl.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"


namespace za
{
////////////////////////////////////////////////////////////
za::Optional<IpAddress> IpAddressUtils::resolve(za::StringView address)
{
    if (address.empty())
    {
        // Not generating an error message here as resolution failure is a valid outcome.
        return za::nullOpt;
    }

    // Try to convert the address as a byte representation ("xxx.xxx.xxx.xxx").
    // `parseIpv4` uses `inet_pton`, which (unlike the legacy `inet_addr`) has no
    // ambiguity between "broadcast address" and "error", so we don't need to
    // special-case `"255.255.255.255"` or `"0.0.0.0"` here.
    if (const auto ip = priv::SocketImpl::parseIpv4(address.data()); ip.hasValue())
        return za::makeOptional<IpAddress>(priv::SocketImpl::networkToHost(*ip));

    // Not a valid address, try to convert it as a host name
    const za::Optional converted = priv::SocketImpl::convertToHostname(address.data());

    if (!converted.hasValue())
    {
        // Not generating an error message here as resolution failure is a valid outcome.
        return za::nullOpt;
    }

    return za::makeOptional<IpAddress>(priv::SocketImpl::networkToHost(*converted));
}


////////////////////////////////////////////////////////////
za::String IpAddressUtils::toString(IpAddress ipAddress)
{
    // `m_address` is the host-byte-order integer representation of the four
    // octets. `addrToString` wraps `inet_ntop`, which expects the address in
    // *network* byte order, so we convert host -> network here.
    const auto buf = priv::SocketImpl::addrToString(priv::SocketImpl::hostToNetwork(ipAddress.m_address));
    return za::String{buf.data};
}

} // namespace za
