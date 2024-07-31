////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/CaptureDeviceHandle.hpp>
#include <SFML/Audio/SoundRecorder.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <SFML/Base/Optional.hpp>

#include <iostream>

#include <cstddef>
#include <cstdint>


const std::uint8_t clientAudioData   = 1;
const std::uint8_t clientEndOfStream = 2;


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
    explicit NetworkRecorder(sf::IpAddress host, unsigned short port) : m_host(host), m_port(port)
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
            std::cerr << "Failed to stop network recorder on destruction" << std::endl;
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
            std::cout << "Connected to server " << m_host << std::endl;
            return true;
        }

        return false;
    }

    ////////////////////////////////////////////////////////////
    /// \see SoundRecorder::onProcessSamples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onProcessSamples(const std::int16_t* samples, std::size_t sampleCount) override
    {
        // Pack the audio samples into a network packet
        sf::Packet packet;
        packet << clientAudioData;
        packet.append(samples, sampleCount * sizeof(std::int16_t));

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
            std::cerr << "Failed to send end-of-stream packet" << std::endl;
            return false;
        }

        // Close the socket
        m_socket.disconnect();
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
/// start sending him audio data
///
////////////////////////////////////////////////////////////
void doClient(sf::CaptureDevice& captureDevice, unsigned short port)
{
    // Ask for server address
    sf::base::Optional<sf::IpAddress> server;
    do
    {
        std::cout << "Type address or name of the server to connect to: ";
        std::cin >> server;
    } while (!server.hasValue());

    // Create an instance of our custom recorder
    NetworkRecorder recorder(server.value(), port);

    // Wait for user input...
    std::cin.ignore(10000, '\n');
    std::cout << "Press enter to start recording audio";
    std::cin.ignore(10000, '\n');

    // Start capturing audio data
    if (!recorder.start(captureDevice, 44100))
    {
        std::cerr << "Failed to start recorder" << std::endl;
        return;
    }

    std::cout << "Recording... press enter to stop";
    std::cin.ignore(10000, '\n');

    if (!recorder.stop())
        std::cerr << "Failed to stop network recorder" << std::endl;
}
