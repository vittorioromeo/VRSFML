#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Export.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/SocketHandle.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Socket that listens to new TCP connections
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API TcpListener : public Socket
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Result of `TcpListener::accept`
    ///
    /// When `status` is `Status::Done`, `socket` holds the newly
    /// accepted connection. Otherwise `socket` is empty.
    ///
    ////////////////////////////////////////////////////////////
    struct AcceptResult
    {
        Status                    status; //!< Operation status
        base::Optional<TcpSocket> socket; //!< Accepted connection (only when `status == Done`)
    };

    ////////////////////////////////////////////////////////////
    /// \brief Factory: create a socket that is already listening
    ///
    /// Atomically creates the underlying OS handle, binds it to
    /// `port`/`address` and starts listening for incoming
    /// connections. There is no "created but not listening"
    /// intermediate state.
    ///
    /// When providing `sf::Socket::AnyPort` as port, the system
    /// picks an available port; retrieve it via `getLocalPort()`.
    ///
    /// Binding to `sf::IpAddress::Broadcast` is rejected.
    ///
    /// \param port       Port to listen on for incoming connections
    /// \param isBlocking Desired blocking state
    /// \param address    Interface address to bind to
    ///
    /// \return `TcpListener` on success, `base::nullOpt` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<TcpListener> create(unsigned short port,
                                                            bool           isBlocking,
                                                            IpAddress      address = IpAddress::Any);

    ////////////////////////////////////////////////////////////
    /// \brief Get the port to which the socket is bound locally
    ///
    /// \return Port to which the socket is bound
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned short getLocalPort() const;

    ////////////////////////////////////////////////////////////
    /// \brief Accept a new connection
    ///
    /// If the socket is in blocking mode, this function will
    /// not return until a connection is actually received.
    ///
    /// \return An `AcceptResult` whose `status` indicates the
    ///         outcome and, on success, whose `socket` holds the
    ///         newly accepted connection.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] AcceptResult accept();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Private constructor used by the factory
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TcpListener(SocketHandle handle, bool isBlocking);
};


} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TcpListener
/// \ingroup network
///
/// A listener socket is a special type of socket that listens to
/// a given port and waits for connections on that port.
/// This is all it can do.
///
/// When a new connection is received, you must call `accept` and
/// the listener returns a new instance of `sf::TcpSocket` that
/// is properly initialized and can be used to communicate with
/// the new client.
///
/// Listener sockets are specific to the TCP protocol,
/// UDP sockets are connectionless and can therefore communicate
/// directly. As a consequence, a listener socket will always
/// return the new connections as `sf::TcpSocket` instances.
///
/// A listener is automatically closed on destruction.
///
/// Usage example:
/// \code
/// // Create a listener socket and make it wait for new
/// // connections on port 55001
/// auto listener = sf::TcpListener::create(55001, /* isBlocking */ true).value();
///
/// // Endless loop that waits for new connections
/// while (running)
/// {
///     if (auto result = listener.accept(); result.status == sf::Socket::Status::Done)
///     {
///         // A new client just connected!
///         std::cout << "New connection received from " << result.socket->getRemoteAddress().value() << '\n';
///         doSomethingWith(*result.socket);
///     }
/// }
/// \endcode
///
/// \see `sf::TcpSocket`, `sf::Socket`
///
////////////////////////////////////////////////////////////
