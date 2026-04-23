#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Export.hpp"

#include "SFML/Network/SocketHandle.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Base class for all the socket types
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API Socket
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Status codes that may be returned by socket functions
    ///
    ////////////////////////////////////////////////////////////
    enum class Status : unsigned char
    {
        Done,         //!< The socket has sent / received the data
        NotReady,     //!< The socket is not ready to send / receive data yet
        Partial,      //!< The socket sent a part of the data
        Disconnected, //!< The TCP socket has been disconnected
        Error         //!< An unexpected error happened
    };

    ////////////////////////////////////////////////////////////
    /// \brief Some special values used by sockets
    ///
    ////////////////////////////////////////////////////////////
    enum : unsigned short
    {
        AnyPort = 0u //!< Special value that tells the system to pick any available port
    };

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Socket();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Socket(const Socket&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Socket& operator=(const Socket&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Socket(Socket&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Socket& operator=(Socket&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Set the blocking state of the socket
    ///
    /// In blocking mode, calls will not return until they have
    /// completed their task. For example, a call to `receive` in
    /// blocking mode won't return until some data was actually
    /// received.
    /// In non-blocking mode, calls will always return immediately,
    /// using the return code to signal whether there was data
    /// available or not.
    ///
    /// \param blocking `true` to set the socket as blocking, `false` for non-blocking
    ///
    /// \see `isBlocking`
    ///
    ////////////////////////////////////////////////////////////
    void setBlocking(bool blocking);

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether the socket is in blocking or non-blocking mode
    ///
    /// \return `true` if the socket is blocking, `false` otherwise
    ///
    /// \see `setBlocking`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isBlocking() const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Types of protocols that the socket can use
    ///
    ////////////////////////////////////////////////////////////
    enum class Type : unsigned char
    {
        Tcp, //!< TCP protocol
        Udp  //!< UDP protocol
    };

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an already-valid OS handle
    ///
    /// Derived classes must obtain a valid handle via the factory
    /// helpers below before constructing. A `Socket` always owns
    /// a valid OS handle until it is moved-from or explicitly
    /// closed by a derived class (e.g. `TcpSocket::disconnect`).
    ///
    /// \param type       Type of the socket (TCP or UDP)
    /// \param handle     Valid OS-level socket handle
    /// \param isBlocking Current blocking state of the handle
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Socket(Type type, SocketHandle handle, bool isBlocking);

    ////////////////////////////////////////////////////////////
    /// \brief Return the internal handle of the socket
    ///
    /// This function can only be accessed by derived classes.
    ///
    /// \return The internal (OS-specific) handle of the socket
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SocketHandle getNativeHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Apply TCP-specific configuration to an open handle
    ///
    /// Sets the blocking mode and applies TCP options (TCP_NODELAY,
    /// SO_NOSIGPIPE on macOS). Used by both `createTcpHandle` and
    /// `TcpListener::accept` to keep the two configuration paths in
    /// sync.
    ///
    /// \param handle     Valid OS-level TCP socket handle
    /// \param isBlocking Desired blocking state
    ///
    ////////////////////////////////////////////////////////////
    static void configureTcpHandle(SocketHandle handle, bool isBlocking);

    ////////////////////////////////////////////////////////////
    /// \brief Create a fully-configured TCP socket handle
    ///
    /// Equivalent to opening a TCP socket and calling
    /// `configureTcpHandle` on it.
    ///
    /// \param isBlocking Desired blocking state
    ///
    /// \return Valid handle on success, `invalidSocket` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle createTcpHandle(bool isBlocking);

    ////////////////////////////////////////////////////////////
    /// \brief Create a fully-configured UDP socket handle
    ///
    /// Sets the blocking mode and enables broadcast.
    ///
    /// \param isBlocking Desired blocking state
    ///
    /// \return Valid handle on success, `invalidSocket` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SocketHandle createUdpHandle(bool isBlocking);

    ////////////////////////////////////////////////////////////
    /// \brief Close the OS handle and invalidate it
    ///
    /// After this call the socket is "dead": any further use is
    /// a programming error. The destructor is safe.
    ///
    ////////////////////////////////////////////////////////////
    void closeHandle();

    ////////////////////////////////////////////////////////////
    /// \brief Get the port to which the socket is bound locally
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned short getLocalPortImpl(const char* socketTypeStr) const;

private:
    friend class SocketSelector;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Type         m_type;       //!< Type of the socket (TCP or UDP)
    SocketHandle m_socket;     //!< Socket descriptor
    bool         m_isBlocking; //!< Current blocking mode of the socket
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Socket
/// \ingroup network
///
/// This class mainly defines internal stuff to be used by
/// derived classes.
///
/// The only public features that it defines, and which
/// is therefore common to all the socket classes, is the
/// blocking state. All sockets can be set as blocking or
/// non-blocking.
///
/// In blocking mode, socket functions will hang until
/// the operation completes, which means that the entire
/// program (well, in fact the current thread if you use
/// multiple ones) will be stuck waiting for your socket
/// operation to complete.
///
/// In non-blocking mode, all the socket functions will
/// return immediately. If the socket is not ready to complete
/// the requested operation, the function simply returns
/// the proper status code (`Socket::Status::NotReady`).
///
/// The default mode, which is blocking, is the one that is
/// generally used, in combination with threads or selectors.
/// The non-blocking mode is rather used in real-time
/// applications that run an endless loop that can poll
/// the socket often enough, and cannot afford blocking
/// this loop.
///
/// \see `sf::TcpListener`, `sf::TcpSocket`, `sf::UdpSocket`
///
////////////////////////////////////////////////////////////
