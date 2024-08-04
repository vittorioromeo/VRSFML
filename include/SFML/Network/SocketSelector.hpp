#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Export.hpp>

#include <SFML/System/Time.hpp>

#include <SFML/Base/InPlacePImpl.hpp>


namespace sf
{
class Socket;

////////////////////////////////////////////////////////////
/// \brief Multiplexer that allows to read from multiple sockets
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API SocketSelector
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SocketSelector();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SocketSelector();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector(const SocketSelector&);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector& operator=(const SocketSelector&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector(SocketSelector&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector& operator=(SocketSelector&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Add a new socket to the selector
    ///
    /// This function keeps a weak reference to the socket,
    /// so you have to make sure that the socket is not destroyed
    /// while it is stored in the selector.
    /// This function does nothing if the socket is not valid.
    ///
    /// \param socket Reference to the socket to add
    ///
    /// \see remove, clear
    ///
    ////////////////////////////////////////////////////////////
    void add(Socket& socket);

    ////////////////////////////////////////////////////////////
    /// \brief Remove a socket from the selector
    ///
    /// This function doesn't destroy the socket, it simply
    /// removes the reference that the selector has to it.
    ///
    /// \param socket Reference to the socket to remove
    ///
    /// \see add, clear
    ///
    ////////////////////////////////////////////////////////////
    void remove(Socket& socket);

    ////////////////////////////////////////////////////////////
    /// \brief Remove all the sockets stored in the selector
    ///
    /// This function doesn't destroy any instance, it simply
    /// removes all the references that the selector has to
    /// external sockets.
    ///
    /// \see add, remove
    ///
    ////////////////////////////////////////////////////////////
    void clear();

    ////////////////////////////////////////////////////////////
    /// \brief Wait until one or more sockets are ready to receive
    ///
    /// This function returns as soon as at least one socket has
    /// some data available to be received. To know which sockets are
    /// ready, use the isReady function.
    /// If you use a timeout and no socket is ready before the timeout
    /// is over, the function returns false.
    ///
    /// \param timeout Maximum time to wait, (use Time::Zero for infinity)
    ///
    /// \return True if there are sockets ready, false otherwise
    ///
    /// \see isReady
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool wait(Time timeout = Time::Zero);

    ////////////////////////////////////////////////////////////
    /// \brief Test a socket to know if it is ready to receive data
    ///
    /// This function must be used after a call to Wait, to know
    /// which sockets are ready to receive data. If a socket is
    /// ready, a call to receive will never block because we know
    /// that there is data available to read.
    /// Note that if this function returns true for a TcpListener,
    /// this means that it is ready to accept a new connection.
    ///
    /// \param socket Socket to test
    ///
    /// \return True if the socket is ready to read, false otherwise
    ///
    /// \see isReady
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isReady(Socket& socket) const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 1792> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SocketSelector
/// \ingroup network
///
/// Socket selectors provide a way to wait until some data is
/// available on a set of sockets, instead of just one. This
/// is convenient when you have multiple sockets that may
/// possibly receive data, but you don't know which one will
/// be ready first. In particular, it avoids to use a thread
/// for each socket; with selectors, a single thread can handle
/// all the sockets.
///
/// All types of sockets can be used in a selector:
/// \li sf::TcpListener
/// \li sf::TcpSocket
/// \li sf::UdpSocket
///
/// A selector doesn't store its own copies of the sockets
/// (socket classes are not copyable anyway), it simply keeps
/// a reference to the original sockets that you pass to the
/// "add" function. Therefore, you can't use the selector as a
/// socket container, you must store them outside and make sure
/// that they are alive as long as they are used in the selector.
///
/// Using a selector is simple:
/// \li populate the selector with all the sockets that you want to observe
/// \li make it wait until there is data available on any of the sockets
/// \li test each socket to find out which ones are ready
///
/// Usage example:
/// \code
/// // Create a socket to listen to new connections
/// sf::TcpListener listener;
/// if (listener.listen(55001) != sf::Socket::Status::Done)
/// {
///     // Handle error...
/// }
///
/// // Create a list to store the future clients
/// std::vector<sf::TcpSocket> clients;
///
/// // Create a selector
/// sf::SocketSelector selector;
///
/// // Add the listener to the selector
/// selector.add(listener);
///
/// // Endless loop that waits for new connections
/// while (running)
/// {
///     // Make the selector wait for data on any socket
///     if (selector.wait())
///     {
///         // Test the listener
///         if (selector.isReady(listener))
///         {
///             // The listener is ready: there is a pending connection
///             sf::TcpSocket client;
///             if (listener.accept(client) == sf::Socket::Status::Done)
///             {
///                 // Add the new client to the selector so that we will
///                 // be notified when he sends something
///                 selector.add(client);
///
///                 // Add the new client to the clients list
///                 clients.push_back(std::move(client));
///             }
///             else
///             {
///                 // Handle error...
///             }
///         }
///         else
///         {
///             // The listener socket is not ready, test all other sockets (the clients)
///             for (sf::TcpSocket& client : clients)
///             {
///                 if (selector.isReady(client))
///                 {
///                     // The client has sent some data, we can receive it
///                     sf::Packet packet;
///                     if (client.receive(packet) == sf::Socket::Status::Done)
///                     {
///                         ...
///                     }
///                 }
///             }
///         }
///     }
/// }
/// \endcode
///
/// \see sf::Socket
///
////////////////////////////////////////////////////////////
