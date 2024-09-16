#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Export.hpp"

#include "SFML/Network/IpAddress.hpp"

#include "SFML/Base/FwdStdString.hpp" // used
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Encapsulate an IPv4 network address
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API IpAddressUtils
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
    /// \return Address on success, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<IpAddress> resolve(base::StringView address);

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
    [[nodiscard]] static std::string toString(IpAddress ipAddress);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::IpAddressUtils
/// \ingroup network
///
/// TODO P1: docs
///
/// \see sf::IpAddress
///
////////////////////////////////////////////////////////////
