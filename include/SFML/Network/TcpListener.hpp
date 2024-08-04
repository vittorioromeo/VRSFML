#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Export.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Socket.hpp>


namespace sf
{
class TcpSocket;

////////////////////////////////////////////////////////////
/// \brief Socket that listens to new TCP connections
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API TcpListener : public Socket
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TcpListener();

    ////////////////////////////////////////////////////////////
    /// \brief Get the port to which the socket is bound locally
    ///
    /// If the socket is not listening to a port, this function
    /// returns 0.
    ///
    /// \return Port to which the socket is bound
    ///
    /// \see listen
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned short getLocalPort() const;

    ////////////////////////////////////////////////////////////
    /// \brief Start listening for incoming connection attempts
    ///
    /// This function makes the socket start listening on the
    /// specified port, waiting for incoming connection attempts.
    ///
    /// If the socket is already listening on a port when this
    /// function is called, it will stop listening on the old
    /// port before starting to listen on the new port.
    ///
    /// When providing sf::Socket::AnyPort as port, the listener
    /// will request an available port from the system.
    /// The chosen port can be retrieved by calling getLocalPort().
    ///
    /// \param port    Port to listen on for incoming connection attempts
    /// \param address Address of the interface to listen on
    ///
    /// \return Status code
    ///
    /// \see accept, close
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status listen(unsigned short port, IpAddress address = IpAddress::Any);

    ////////////////////////////////////////////////////////////
    /// \brief Stop listening and close the socket
    ///
    /// This function gracefully stops the listener. If the
    /// socket is not listening, this function has no effect.
    ///
    /// \see listen
    ///
    ////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////
    /// \brief Accept a new connection
    ///
    /// If the socket is in blocking mode, this function will
    /// not return until a connection is actually received.
    ///
    /// \param socket Socket that will hold the new connection
    ///
    /// \return Status code
    ///
    /// \see listen
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status accept(TcpSocket& socket);
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
/// When a new connection is received, you must call accept and
/// the listener returns a new instance of sf::TcpSocket that
/// is properly initialized and can be used to communicate with
/// the new client.
///
/// Listener sockets are specific to the TCP protocol,
/// UDP sockets are connectionless and can therefore communicate
/// directly. As a consequence, a listener socket will always
/// return the new connections as sf::TcpSocket instances.
///
/// A listener is automatically closed on destruction, like all
/// other types of socket. However if you want to stop listening
/// before the socket is destroyed, you can call its close()
/// function.
///
/// Usage example:
/// \code
/// // Create a listener socket and make it wait for new
/// // connections on port 55001
/// sf::TcpListener listener;
/// listener.listen(55001);
///
/// // Endless loop that waits for new connections
/// while (running)
/// {
///     sf::TcpSocket client;
///     if (listener.accept(client) == sf::Socket::Done)
///     {
///         // A new client just connected!
///         std::cout << "New connection received from " << client.getRemoteAddress().value() << '\n';
///         doSomethingWith(client);
///     }
/// }
/// \endcode
///
/// \see sf::TcpSocket, sf::Socket
///
////////////////////////////////////////////////////////////
