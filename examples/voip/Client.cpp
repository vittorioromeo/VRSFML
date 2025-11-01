////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Client.hpp"

#include "SFML/Audio/CaptureDeviceHandle.hpp"
#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"

#include <string>


constexpr sf::base::U8 clientAudioData   = 1;
constexpr sf::base::U8 clientEndOfStream = 2;


////////////////////////////////////////////////////////////
/// Specialization of audio recorder for sending recorded audio
/// data through the network
////////////////////////////////////////////////////////////
class NetworkRecorder : public sf::SoundRecorder
{
public:
    ////////////////////////////////////////////////////////////
    /// Constructor
    ///
    /// \param host Remote host to which send the recording data
    /// \param port Port of the remote host
    ///
    ////////////////////////////////////////////////////////////
    explicit NetworkRecorder(sf::IpAddress host, unsigned short port) :
        m_host(host),
        m_port(port),
        m_socket(/* isBlocking */ true)
    {
    }

    ////////////////////////////////////////////////////////////
    /// Destructor
    ///
    /// \see SoundRecorder::~SoundRecorder()
    ///
    ////////////////////////////////////////////////////////////
    ~NetworkRecorder() override
    {
        if (!stop())
            sf::cErr() << "Failed to stop network recorder on destruction" << sf::endL;
    }

private:
    ////////////////////////////////////////////////////////////
    /// \see SoundRecorder::onStart
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onStart(sf::CaptureDevice&) override
    {
        if (m_socket.connect(m_host, m_port) == sf::Socket::Status::Done)
        {
            sf::cOut() << "Connected to server " << sf::IpAddressUtils::toString(m_host) << sf::endL;
            return true;
        }

        return false;
    }

    ////////////////////////////////////////////////////////////
    /// \see SoundRecorder::onProcessSamples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onProcessSamples(const sf::base::I16* samples, sf::base::SizeT sampleCount) override
    {
        // Pack the audio samples into a network packet
        sf::Packet packet;
        packet << clientAudioData;
        packet.append(samples, sampleCount * sizeof(sf::base::I16));

        // Send the audio packet to the server
        return m_socket.send(packet) == sf::Socket::Status::Done;
    }

    ////////////////////////////////////////////////////////////
    /// \see SoundRecorder::onStop
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onStop(sf::CaptureDevice&) override
    {
        // Send an "end-of-stream" packet
        sf::Packet packet;
        packet << clientEndOfStream;

        if (m_socket.send(packet) != sf::Socket::Status::Done)
        {
            sf::cErr() << "Failed to send end-of-stream packet" << sf::endL;
            return false;
        }

        // Close the socket
        [[maybe_unused]] const bool rc = m_socket.disconnect();
        SFML_BASE_ASSERT(rc);

        return true;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    sf::IpAddress  m_host;   ///< Address of the remote host
    unsigned short m_port;   ///< Remote port
    sf::TcpSocket  m_socket; ///< Socket used to communicate with the server
};


////////////////////////////////////////////////////////////
/// Create a client, connect it to a running server and
/// start sending it audio data
///
////////////////////////////////////////////////////////////
void doClient(sf::CaptureDevice& captureDevice, unsigned short port)
{
    // Ask for server address
    sf::base::Optional<sf::IpAddress> server;
    do
    {
        sf::cOut() << "Type address or name of the server to connect to: ";

        std::string addressStr;
        sf::cIn() >> addressStr;
        server = sf::IpAddressUtils::resolve(addressStr);
    } while (!server.hasValue());

    // Create an instance of our custom recorder
    NetworkRecorder recorder(server.value(), port);

    // Wait for user input...
    sf::cIn().ignore(10'000, '\n');
    sf::cOut() << "Press enter to start recording audio";
    sf::cIn().ignore(10'000, '\n');

    // Start capturing audio data
    if (!recorder.start(captureDevice, 44'100))
    {
        sf::cErr() << "Failed to start recorder" << sf::endL;
        return;
    }

    sf::cOut() << "Recording... press enter to stop";
    sf::cIn().ignore(10'000, '\n');

    if (!recorder.stop())
        sf::cErr() << "Failed to stop network recorder" << sf::endL;
}
