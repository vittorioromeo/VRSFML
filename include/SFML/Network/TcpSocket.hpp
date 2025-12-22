#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Export.hpp"

#include "SFML/Network/Socket.hpp"

#include "SFML/System/Time.hpp"
#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class TcpListener;
class IpAddress;
class Packet;
} // namespace sf

namespace sf::base
{
class String;
} // namespace sf::base


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Specialized socket using the TCP protocol
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API TcpSocket : public Socket
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    using Byte = unsigned char;

    ////////////////////////////////////////////////////////////
    /// \brief TLS status codes that may be returned by TLS setup
    ///
    ////////////////////////////////////////////////////////////
    enum class TlsStatus
    {
        NotConnected,      //!< TCP connection not yet connected
        HandshakeStarted,  //!< TLS handshake has been started
        HandshakeComplete, //!< TLS handshake is complete, stream is encrypted
        Error              //!< An unexpected error happened
    };

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TcpSocket(bool isBlocking);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~TcpSocket();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    TcpSocket(const TcpSocket&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    TcpSocket& operator=(const TcpSocket&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    TcpSocket(TcpSocket&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    TcpSocket& operator=(TcpSocket&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the port to which the socket is bound locally
    ///
    /// If the socket is not connected, this function returns 0.
    ///
    /// \return Port to which the socket is bound
    ///
    /// \see `connect`, `getRemotePort`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned short getLocalPort() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the address of the connected peer
    ///
    /// If the socket is not connected, this function returns
    /// an unset optional.
    ///
    /// \return Address of the remote peer
    ///
    /// \see `getRemotePort`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<IpAddress> getRemoteAddress() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the port of the connected peer to which
    ///        the socket is connected
    ///
    /// If the socket is not connected, this function returns 0.
    ///
    /// \return Remote port to which the socket is connected
    ///
    /// \see `getRemoteAddress`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned short getRemotePort() const;

    ////////////////////////////////////////////////////////////
    /// \brief Connect the socket to a remote peer
    ///
    /// In blocking mode, this function may take a while, especially
    /// if the remote peer is not reachable. The last parameter allows
    /// you to stop trying to connect after a given timeout.
    /// If the socket is already connected, the connection is
    /// forcibly disconnected before attempting to connect again.
    ///
    /// \param remoteAddress Address of the remote peer
    /// \param remotePort    Port of the remote peer
    /// \param timeout       base::Optional maximum time to wait
    ///
    /// \return Status code
    ///
    /// \see `disconnect`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status connect(IpAddress remoteAddress, unsigned short remotePort, Time timeout = {});

    ////////////////////////////////////////////////////////////
    /// \brief Disconnect the socket from its remote peer
    ///
    /// This function gracefully closes the connection. If the
    /// socket is not connected, this function has no effect.
    ///
    /// \see `connect`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool disconnect();

    ////////////////////////////////////////////////////////////
    /// \brief Set up transport layer security as a client
    ///
    /// Once the TCP connection is connected, transport layer
    /// security can be set up.
    ///
    /// All the necessary cryptographic initialization will
    /// be performed when this function is called.
    ///
    /// If this function is called before the TCP connection is
    /// connected, it will return `TlsStatus::NotConnected` and
    /// must be called again once the TCP connection is connected.
    ///
    /// If this function started TLS setup but could not finish
    /// it within this call e.g. because this socket was set to
    /// non-blocking, it will return `TlsStatus::HandshakeStarted`
    /// and this function will have to be called repeatedly until
    /// `TlsStatus::HandshakeComplete` is returned. If this socket
    /// is blocking, `TlsStatus::HandshakeComplete` should be
    /// returned within the same function call if TLS setup was
    /// successful.
    ///
    /// If `TlsStatus::Error` is returned, something went wrong
    /// with TLS setup and the connection must be reconnected and
    /// TLS setup reattempted after it is connected again.
    ///
    /// If verification is enabled, this function verifies the peer
    /// using the system provided certificate store. If the peer
    /// does not have a certificate that was signed by a certificate
    /// authority i.e. a self-signed certificate, the entire certificate
    /// chain can be provided using the alternative overload.
    ///
    /// Servers that host multiple services under different names
    /// need to know which of those services we want to connect
    /// to in order to reply with the correct certificate chain.
    /// Server name indication (SNI) is used for this purpose. The
    /// hostname provided to this function is sent to the server
    /// if it supports SNI in order for it to return the corresponding
    /// certificate chain. The hostname is then used to verify the
    /// certificate chain that was returned by the server. If the
    /// server does not support SNI or only serves a single
    /// certificate chain, the hostname will only be used for
    /// verification.
    ///
    /// \param hostname   Hostname of the remote peer, used for verification
    /// \param verifyPeer `true` to enable peer verification, `false` to disable it
    ///
    /// \return TLS status code
    ///
    /// \see `setupTlsServer`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TlsStatus setupTlsClient(const sf::UnicodeString& hostname, bool verifyPeer = true);

    ////////////////////////////////////////////////////////////
    /// \brief Set up transport layer security as a client
    ///
    /// Once the TCP connection is connected, transport layer
    /// security can be set up.
    ///
    /// All the necessary cryptographic initialization will
    /// be performed when this function is called.
    ///
    /// If this function is called before the TCP connection is
    /// connected, it will return `TlsStatus::NotConnected` and
    /// must be called again once the TCP connection is connected.
    ///
    /// If this function started TLS setup but could not finish
    /// it within this call e.g. because this socket was set to
    /// non-blocking, it will return `TlsStatus::HandshakeStarted`
    /// and this function will have to be called repeatedly until
    /// `TlsStatus::HandshakeComplete` is returned. If this socket
    /// is blocking, `TlsStatus::HandshakeComplete` should be
    /// returned within the same function call if TLS setup was
    /// successful.
    ///
    /// If `TlsStatus::Error` is returned, something went wrong
    /// with TLS setup and the connection must be reconnected and
    /// TLS setup reattempted after it is connected again.
    ///
    /// Servers that host multiple services under different names
    /// need to know which of those services we want to connect
    /// to in order to reply with the correct certificate chain.
    /// Server name indication (SNI) is used for this purpose. The
    /// hostname provided to this function is sent to the server
    /// if it supports SNI in order for it to return the corresponding
    /// certificate chain. The hostname is then used to verify the
    /// certificate chain that was returned by the server. If the
    /// server does not support SNI or only serves a single
    /// certificate chain, the hostname will only be used for
    /// verification.
    ///
    /// When calling this overload, the certificate chain to verify
    /// the host with has to be provided. Verification is always
    /// enabled when calling this overload.
    ///
    /// The certificate data should be provided in PEM format.
    ///
    /// This overload is provided to prevent a const char* argument
    /// resulting in the `bool verifyPeer` overload being called
    /// instead of the `base::StringView` overload.
    ///
    /// \param hostname             Hostname of the remote peer, used for verification
    /// \param certificateChainData Null terminated string containing certificate chain data in PEM encoding
    ///
    /// \return TLS status code
    ///
    /// \see `setupTlsServer`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TlsStatus setupTlsClient(const sf::UnicodeString& hostname, const char* certificateChainData);

    ////////////////////////////////////////////////////////////
    /// \brief Set up transport layer security as a client
    ///
    /// Once the TCP connection is connected, transport layer
    /// security can be set up.
    ///
    /// All the necessary cryptographic initialization will
    /// be performed when this function is called.
    ///
    /// If this function is called before the TCP connection is
    /// connected, it will return `TlsStatus::NotConnected` and
    /// must be called again once the TCP connection is connected.
    ///
    /// If this function started TLS setup but could not finish
    /// it within this call e.g. because this socket was set to
    /// non-blocking, it will return `TlsStatus::HandshakeStarted`
    /// and this function will have to be called repeatedly until
    /// `TlsStatus::HandshakeComplete` is returned. If this socket
    /// is blocking, `TlsStatus::HandshakeComplete` should be
    /// returned within the same function call if TLS setup was
    /// successful.
    ///
    /// If `TlsStatus::Error` is returned, something went wrong
    /// with TLS setup and the connection must be reconnected and
    /// TLS setup reattempted after it is connected again.
    ///
    /// Servers that host multiple services under different names
    /// need to know which of those services we want to connect
    /// to in order to reply with the correct certificate chain.
    /// Server name indication (SNI) is used for this purpose. The
    /// hostname provided to this function is sent to the server
    /// if it supports SNI in order for it to return the corresponding
    /// certificate chain. The hostname is then used to verify the
    /// certificate chain that was returned by the server. If the
    /// server does not support SNI or only serves a single
    /// certificate chain, the hostname will only be used for
    /// verification.
    ///
    /// When calling this overload, the certificate chain to verify
    /// the host with has to be provided. Verification is always
    /// enabled when calling this overload.
    ///
    /// The certificate data can be provided in PEM or DER format.
    ///
    /// \param hostname             Hostname of the remote peer, used for verification
    /// \param certificateChainData Certificate chain data in PEM or DER encoding
    /// \param certificateChainSize Size of the certificate chain data
    ///
    /// \return TLS status code
    ///
    /// \see `setupTlsServer`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TlsStatus setupTlsClient(const sf::UnicodeString& hostname,
                                           const Byte*              certificateChainData,
                                           base::SizeT              certificateChainSize);

    ////////////////////////////////////////////////////////////
    /// \brief Set up transport layer security as a client
    ///
    /// Once the TCP connection is connected, transport layer
    /// security can be set up.
    ///
    /// All the necessary cryptographic initialization will
    /// be performed when this function is called.
    ///
    /// If this function is called before the TCP connection is
    /// connected, it will return `TlsStatus::NotConnected` and
    /// must be called again once the TCP connection is connected.
    ///
    /// If this function started TLS setup but could not finish
    /// it within this call e.g. because this socket was set to
    /// non-blocking, it will return `TlsStatus::HandshakeStarted`
    /// and this function will have to be called repeatedly until
    /// `TlsStatus::HandshakeComplete` is returned. If this socket
    /// is blocking, `TlsStatus::HandshakeComplete` should be
    /// returned within the same function call if TLS setup was
    /// successful.
    ///
    /// If `TlsStatus::Error` is returned, something went wrong
    /// with TLS setup and the connection must be reconnected and
    /// TLS setup reattempted after it is connected again.
    ///
    /// Servers that host multiple services under different names
    /// need to know which of those services we want to connect
    /// to in order to reply with the correct certificate chain.
    /// Server name indication (SNI) is used for this purpose. The
    /// hostname provided to this function is sent to the server
    /// if it supports SNI in order for it to return the corresponding
    /// certificate chain. The hostname is then used to verify the
    /// certificate chain that was returned by the server. If the
    /// server does not support SNI or only serves a single
    /// certificate chain, the hostname will only be used for
    /// verification.
    ///
    /// When calling this overload, the certificate chain to verify
    /// the host with has to be provided. Verification is always
    /// enabled when calling this overload.
    ///
    /// The certificate data should be provided in PEM format.
    ///
    /// \param hostname             Hostname of the remote peer, used for verification
    /// \param certificateChainData Certificate chain data in PEM encoding
    ///
    /// \return TLS status code
    ///
    /// \see `setupTlsServer`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TlsStatus setupTlsClient(const sf::UnicodeString& hostname, base::StringView certificateChainData);

    ////////////////////////////////////////////////////////////
    /// \brief Set up transport layer security as a server
    ///
    /// Once the TCP connection is connected, transport layer
    /// security can be set up.
    ///
    /// All the necessary cryptographic initialization will
    /// be performed when this function is called.
    ///
    /// If this function is called before the TCP connection is
    /// connected, it will return `TlsStatus::NotConnected` and
    /// must be called again once the TCP connection is connected.
    ///
    /// If this function started TLS setup but could not finish
    /// it within this call e.g. because this socket was set to
    /// non-blocking, it will return `TlsStatus::HandshakeStarted`
    /// and this function will have to be called repeatedly until
    /// `TlsStatus::HandshakeComplete` is returned. If this socket
    /// is blocking, `TlsStatus::HandshakeComplete` should be
    /// returned within the same function call if TLS setup was
    /// successful.
    ///
    /// If `TlsStatus::Error` is returned, something went wrong
    /// with TLS setup and the connection must be disconnected.
    /// The client must reconnect and reattempt TLS setup again.
    ///
    /// As a server, a certificate chain as well as a private key
    /// must be provided.
    ///
    /// The certificate and private key data can be provided in
    /// PEM or DER format.
    ///
    /// If the private key is secured by a password, the password
    /// must be provided.
    ///
    /// \param certificateChainData   Certificate chain data in PEM or DER encoding
    /// \param certificateChainSize   Size of the certificate chain data
    /// \param privateKeyData         Private key data in PEM or DER encoding
    /// \param privateKeySize         Size of the private key data
    /// \param privateKeyPasswordData Private key password data
    /// \param privateKeyPasswordSize Size of the private key password data
    ///
    /// \return TLS status code
    ///
    /// \see `setupTlsClient`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TlsStatus setupTlsServer(
        const Byte* certificateChainData,
        base::SizeT certificateChainSize,
        const Byte* privateKeyData,
        base::SizeT privateKeySize,
        const Byte* privateKeyPasswordData,
        base::SizeT privateKeyPasswordSize);

    ////////////////////////////////////////////////////////////
    /// \brief Set up transport layer security as a server
    ///
    /// Once the TCP connection is connected, transport layer
    /// security can be set up.
    ///
    /// All the necessary cryptographic initialization will
    /// be performed when this function is called.
    ///
    /// If this function is called before the TCP connection is
    /// connected, it will return `TlsStatus::NotConnected` and
    /// must be called again once the TCP connection is connected.
    ///
    /// If this function started TLS setup but could not finish
    /// it within this call e.g. because this socket was set to
    /// non-blocking, it will return `TlsStatus::HandshakeStarted`
    /// and this function will have to be called repeatedly until
    /// `TlsStatus::HandshakeComplete` is returned. If this socket
    /// is blocking, `TlsStatus::HandshakeComplete` should be
    /// returned within the same function call if TLS setup was
    /// successful.
    ///
    /// If `TlsStatus::Error` is returned, something went wrong
    /// with TLS setup and the connection must be disconnected.
    /// The client must reconnect and reattempt TLS setup again.
    ///
    /// As a server, a certificate chain as well as a private key
    /// must be provided.
    ///
    /// The certificate and private key data should be provided in
    /// PEM format.
    ///
    /// If the private key is secured by a password, the password
    /// must be provided.
    ///
    /// \param certificateChainData   Certificate chain data in PEM encoding
    /// \param privateKeyData         Private key data in PEM encoding
    /// \param privateKeyPasswordData Private key password if required
    ///
    /// \return TLS status code
    ///
    /// \see `setupTlsClient`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TlsStatus setupTlsServer(base::StringView certificateChainData,
                                           base::StringView privateKeyData,
                                           base::StringView privateKeyPasswordData = "");

    ////////////////////////////////////////////////////////////
    /// \brief Get the name of the TLS ciphersuite currently in use
    ///
    /// \return TLS ciphersuite currently in use or `std::nullopt` if TLS is not set up
    ///
    /// \see `setupTlsClient`, `setupTlsServer`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::String> getCurrentCiphersuiteName() const;

    ////////////////////////////////////////////////////////////
    /// \brief Send raw data to the remote peer
    ///
    /// To be able to handle partial sends over non-blocking
    /// sockets, use the `send(const void*, base::SizeT, base::SizeT&)`
    /// overload instead.
    /// This function will fail if the socket is not connected.
    ///
    /// \param data Pointer to the sequence of bytes to send
    /// \param size Number of bytes to send
    ///
    /// \return Status code
    ///
    /// \see `receive`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status send(const void* data, base::SizeT size);

    ////////////////////////////////////////////////////////////
    /// \brief Send raw data to the remote peer
    ///
    /// This function will fail if the socket is not connected.
    ///
    /// \param data Pointer to the sequence of bytes to send
    /// \param size Number of bytes to send
    /// \param sent The number of bytes sent will be written here
    ///
    /// \return Status code
    ///
    /// \see `receive`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status send(const void* data, base::SizeT size, base::SizeT& sent);

    ////////////////////////////////////////////////////////////
    /// \brief Receive raw data from the remote peer
    ///
    /// In blocking mode, this function will wait until some
    /// bytes are actually received.
    /// This function will fail if the socket is not connected.
    ///
    /// \param data     Pointer to the array to fill with the received bytes
    /// \param size     Maximum number of bytes that can be received
    /// \param received This variable is filled with the actual number of bytes received
    ///
    /// \return Status code
    ///
    /// \see `send`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status receive(void* data, base::SizeT size, base::SizeT& received);

    ////////////////////////////////////////////////////////////
    /// \brief Send a formatted packet of data to the remote peer
    ///
    /// In non-blocking mode, if this function returns `sf::Socket::Status::Partial`,
    /// you \em must retry sending the same unmodified packet before sending
    /// anything else in order to guarantee the packet arrives at the remote
    /// peer uncorrupted.
    /// This function will fail if the socket is not connected.
    ///
    /// \param packet Packet to send
    ///
    /// \return Status code
    ///
    /// \see `receive`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status send(Packet& packet);

    ////////////////////////////////////////////////////////////
    /// \brief Receive a formatted packet of data from the remote peer
    ///
    /// In blocking mode, this function will wait until the whole packet
    /// has been received.
    /// This function will fail if the socket is not connected.
    ///
    /// \param packet Packet to fill with the received data
    ///
    /// \return Status code
    ///
    /// \see `send`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status receive(Packet& packet);

private:
    friend class TcpListener;

    ////////////////////////////////////////////////////////////
    /// \brief Structure holding the data of a pending packet
    ///
    ////////////////////////////////////////////////////////////
    struct PendingPacket
    {
        base::U32          size{};         //!< Data of packet size
        base::SizeT        sizeReceived{}; //!< Number of size bytes received so far
        base::Vector<Byte> data;           //!< Data of the packet
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl; // TODO P1: review impl
    sf::base::UniquePtr<Impl> m_impl; //!< Implementation details

    PendingPacket      m_pendingPacket;     //!< Temporary data of the packet currently being received
    base::Vector<Byte> m_blockToSendBuffer; //!< Buffer used to prepare data being sent from the socket
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TcpSocket
/// \ingroup network
///
/// TCP is a connected protocol, which means that a TCP
/// socket can only communicate with the host it is connected
/// to. It can't send or receive anything if it is not connected.
///
/// The TCP protocol is reliable but adds a slight overhead.
/// It ensures that your data will always be received in order
/// and without errors (no data corrupted, lost or duplicated).
///
/// When a socket is connected to a remote host, you can
/// retrieve information about this host with the
/// `getRemoteAddress` and `getRemotePort` functions. You can
/// also get the local port to which the socket is bound
/// (which is automatically chosen when the socket is connected),
/// with the getLocalPort function.
///
/// Sending and receiving data can use either the low-level
/// or the high-level functions. The low-level functions
/// process a raw sequence of bytes, and cannot ensure that
/// one call to Send will exactly match one call to Receive
/// at the other end of the socket.
///
/// The high-level interface uses packets (see `sf::Packet`),
/// which are easier to use and provide more safety regarding
/// the data that is exchanged. You can look at the `sf::Packet`
/// class to get more details about how they work.
///
/// The socket is automatically disconnected when it is destroyed,
/// but if you want to explicitly close the connection while
/// the socket instance is still alive, you can call disconnect.
///
/// Usage example:
/// \code
/// // ----- The client -----
///
/// // Create a socket and connect it to 192.168.1.50 on port 55001
/// sf::TcpSocket socket;
/// socket.connect("192.168.1.50", 55001);
///
/// // Send a message to the connected host
/// std::string message = "Hi, I am a client";
/// socket.send(message.c_str(), message.size() + 1);
///
/// // Receive an answer from the server
/// char buffer[1024];
/// base::SizeT received = 0;
/// socket.receive(buffer, sizeof(buffer), received);
/// std::cout << "The server said: " << buffer << '\n';
///
/// // ----- The server -----
///
/// // Create a listener to wait for incoming connections on port 55001
/// sf::TcpListener listener;
/// listener.listen(55001);
///
/// // Wait for a connection
/// sf::TcpSocket socket;
/// listener.accept(socket);
/// std::cout << "New client connected: " << socket.getRemoteAddress().value() << '\n';
///
/// // Receive a message from the client
/// char buffer[1024];
/// base::SizeT received = 0;
/// socket.receive(buffer, sizeof(buffer), received);
/// std::cout << "The client said: " << buffer << '\n';
///
/// // Send an answer
/// std::string message = "Welcome, client";
/// socket.send(message.c_str(), message.size() + 1);
/// \endcode
///
/// \see `sf::Socket`, `sf::UdpSocket`, `sf::Packet`
///
////////////////////////////////////////////////////////////
