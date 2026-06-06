#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Export.hpp"

#include "Zancle/Network/IpAddress.hpp"

#include "Zancle/String/StringView.hpp"

#include "Zancle/Vocabulary/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class String;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Encapsulate an IPv4 network address
///
////////////////////////////////////////////////////////////
class ZA_NETWORK_API IpAddressUtils
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the address from a null-terminated string view
    ///
    /// Here \a address can be either a decimal address
    /// (ex: "192.168.1.56") or a network name (ex: "localhost").
    ///
    /// \param address IP address or network name
    ///
    /// \return Address on success, `za::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::Optional<IpAddress> resolve(za::StringView address);

    ////////////////////////////////////////////////////////////
    /// \brief Get a string representation of the address
    ///
    /// The returned string is the decimal representation of the
    /// IP address (like "192.168.1.56"), even if it was constructed
    /// from a host name.
    ///
    /// \return String representation of the address
    ///
    /// \see toInteger
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::String toString(IpAddress ipAddress);
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::IpAddressUtils
/// \ingroup network
///
/// TODO P1: docs
///
/// \see za::IpAddress
///
////////////////////////////////////////////////////////////
