#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Export.hpp>

#include <SFML/System/Time.hpp>

#include <SFML/Base/Optional.hpp>

#include <iosfwd>

#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class IpAddressUtils;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Encapsulate an IPv4 network address
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API IpAddress
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the address from 4 bytes
    ///
    /// Calling IpAddress(a, b, c, d) is equivalent to calling
    /// IpAddressUtils::resolve("a.b.c.d"), but safer as it doesn't
    /// have to parse a string to get the address components.
    ///
    /// \param byte0 First byte of the address
    /// \param byte1 Second byte of the address
    /// \param byte2 Third byte of the address
    /// \param byte3 Fourth byte of the address
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] IpAddress(std::uint8_t byte0, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3);

    ////////////////////////////////////////////////////////////
    /// \brief Construct the address from a 32-bits integer
    ///
    /// This constructor uses the internal representation of
    /// the address directly. It should be used for optimization
    /// purposes, and only if you got that representation from
    /// IpAddress::toInteger().
    ///
    /// \param address 4 bytes of the address packed into a 32-bits integer
    ///
    /// \see toInteger
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit IpAddress(std::uint32_t address);

    ////////////////////////////////////////////////////////////
    /// \brief Get an integer representation of the address
    ///
    /// The returned number is the internal representation of the
    /// address, and should be used for optimization purposes only
    /// (like sending the address through a socket).
    /// The integer produced by this function can then be converted
    /// back to a sf::IpAddress with the proper constructor.
    ///
    /// \return 32-bits unsigned integer representation of the address
    ///
    /// \see toString
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint32_t toInteger() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the computer's local address
    ///
    /// The local address is the address of the computer from the
    /// LAN point of view, i.e. something like 192.168.1.56. It is
    /// meaningful only for communications over the local network.
    /// Unlike getPublicAddress, this function is fast and may be
    /// used safely anywhere.
    ///
    /// \return Local IP address of the computer on success, `base::nullOpt` otherwise
    ///
    /// \see getPublicAddress
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<IpAddress> getLocalAddress();

    ////////////////////////////////////////////////////////////
    /// \brief Get the computer's public address
    ///
    /// The public address is the address of the computer from the
    /// internet point of view, i.e. something like 89.54.1.169.
    /// It is necessary for communications over the world wide web.
    /// The only way to get a public address is to ask it to a
    /// distant website; as a consequence, this function depends on
    /// both your network connection and the server, and may be
    /// very slow. You should use it as few as possible. Because
    /// this function depends on the network connection and on a distant
    /// server, you may use a time limit if you don't want your program
    /// to be possibly stuck waiting in case there is a problem; this
    /// limit is deactivated by default.
    ///
    /// \param timeout Maximum time to wait
    ///
    /// \return Public IP address of the computer on success, `base::nullOpt` otherwise
    ///
    /// \see getLocalAddress
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<IpAddress> getPublicAddress(Time timeout = Time::Zero);

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    // NOLINTBEGIN(readability-identifier-naming)
    static const IpAddress Any;       //!< Value representing any address (0.0.0.0)
    static const IpAddress LocalHost; //!< The "localhost" address (for connecting a computer to itself locally)
    static const IpAddress Broadcast; //!< The "broadcast" address (for sending UDP messages to everyone on a local network)
    // NOLINTEND(readability-identifier-naming)

private:
    friend IpAddressUtils;
    friend SFML_NETWORK_API bool operator<(IpAddress left, IpAddress right);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::uint32_t m_address; //!< Address stored as an unsigned 32 bit integer
};

////////////////////////////////////////////////////////////
/// \brief Overload of == operator to compare two IP addresses
///
/// \param left  Left operand (a IP address)
/// \param right Right operand (a IP address)
///
/// \return True if both addresses are equal
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_NETWORK_API bool operator==(IpAddress left, IpAddress right);

////////////////////////////////////////////////////////////
/// \brief Overload of != operator to compare two IP addresses
///
/// \param left  Left operand (a IP address)
/// \param right Right operand (a IP address)
///
/// \return True if both addresses are different
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_NETWORK_API bool operator!=(IpAddress left, IpAddress right);

////////////////////////////////////////////////////////////
/// \brief Overload of < operator to compare two IP addresses
///
/// \param left  Left operand (a IP address)
/// \param right Right operand (a IP address)
///
/// \return True if \a left is lesser than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_NETWORK_API bool operator<(IpAddress left, IpAddress right);

////////////////////////////////////////////////////////////
/// \brief Overload of > operator to compare two IP addresses
///
/// \param left  Left operand (a IP address)
/// \param right Right operand (a IP address)
///
/// \return True if \a left is greater than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_NETWORK_API bool operator>(IpAddress left, IpAddress right);

////////////////////////////////////////////////////////////
/// \brief Overload of <= operator to compare two IP addresses
///
/// \param left  Left operand (a IP address)
/// \param right Right operand (a IP address)
///
/// \return True if \a left is lesser or equal than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_NETWORK_API bool operator<=(IpAddress left, IpAddress right);

////////////////////////////////////////////////////////////
/// \brief Overload of >= operator to compare two IP addresses
///
/// \param left  Left operand (a IP address)
/// \param right Right operand (a IP address)
///
/// \return True if \a left is greater or equal than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_NETWORK_API bool operator>=(IpAddress left, IpAddress right);

////////////////////////////////////////////////////////////
/// \brief Overload of >> operator to extract an IP address from an input stream
///
/// \param stream  Input stream
/// \param address IP address to extract
///
/// \return Reference to the input stream
///
////////////////////////////////////////////////////////////
SFML_NETWORK_API std::istream& operator>>(std::istream& stream, base::Optional<IpAddress>& address);

////////////////////////////////////////////////////////////
/// \brief Overload of << operator to print an IP address to an output stream
///
/// \param stream  Output stream
/// \param address IP address to print
///
/// \return Reference to the output stream
///
////////////////////////////////////////////////////////////
SFML_NETWORK_API std::ostream& operator<<(std::ostream& stream, IpAddress address);

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::IpAddress
/// \ingroup network
///
/// sf::IpAddress is a utility class for manipulating network
/// addresses. It provides a set a implicit constructors and
/// conversion functions to easily build or transform an IP
/// address from/to various representations.
///
/// Usage example:
/// \code
/// auto a2 = sf::IpAddressUtils::resolve("127.0.0.1");      // the local host address
/// auto a3 = sf::IpAddress::Broadcast;                 // the broadcast address
/// sf::IpAddress a4(192, 168, 1, 56);                  // a local address
/// auto a5 = sf::IpAddressUtils::resolve("my_computer");    // a local address created from a network name
/// auto a6 = sf::IpAddressUtils::resolve("89.54.1.169");    // a distant address
/// auto a7 = sf::IpAddressUtils::resolve("www.google.com"); // a distant address created from a network name
/// auto a8 = sf::IpAddress::getLocalAddress();         // my address on the local network
/// auto a9 = sf::IpAddress::getPublicAddress();        // my address on the internet
/// \endcode
///
/// Note that sf::IpAddress currently doesn't support IPv6
/// nor other types of network addresses.
///
////////////////////////////////////////////////////////////
