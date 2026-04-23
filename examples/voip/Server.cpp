////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Server.hpp"

#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundChannel.hpp"
#include "SFML/Audio/SoundStream.hpp"

#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp" // IWYU pragma: keep
#include "SFML/Base/Vector.hpp"

#include <mutex>


constexpr sf::base::U8 serverAudioData   = 1;
constexpr sf::base::U8 serverEndOfStream = 2;


////////////////////////////////////////////////////////////
/// Customized sound stream for acquiring audio data
/// from the network
////////////////////////////////////////////////////////////
class NetworkAudioStream : public sf::SoundStream
{
public:
    ////////////////////////////////////////////////////////////
    /// Default constructor
    ///
    ////////////////////////////////////////////////////////////
    NetworkAudioStream(sf::PlaybackDevice& playbackDevice) :
        sf::SoundStream{playbackDevice, sf::ChannelMap{sf::SoundChannel::Mono}, 44'100u}
    {
    }

    ////////////////////////////////////////////////////////////
    ~NetworkAudioStream() override
    {
        // Drain the audio thread before `m_samples`/`m_mutex`/`m_offset`/
        // `m_hasFinished` are destroyed: `onGetData` reads all of them, and
        // member destruction order would otherwise free them before
        // `SoundStream::Impl::~Impl` calls `uninitSound`.
        detachFromEngine();
    }

    ////////////////////////////////////////////////////////////
    /// Run the server, stream audio data from the client
    ///
    ////////////////////////////////////////////////////////////
    void start(unsigned short port)
    {
        if (!m_hasFinished)
        {
            // Create a server socket already listening on `port`
            m_listener = sf::TcpListener::create(port, /* isBlocking */ true);
            if (!m_listener.hasValue())
                return;
            sf::cOut() << "Server is listening to port " << port << ", waiting for connections... " << sf::endL;

            // Wait for a connection
            auto acceptResult = m_listener->accept();
            if (acceptResult.status != sf::Socket::Status::Done)
                return;
            m_client = SFML_BASE_MOVE(acceptResult.socket);
            sf::cOut() << "Client connected: " << sf::IpAddressUtils::toString(m_client->getRemoteAddress().value())
                       << sf::endL;

            // Start playback
            play();

            // Start receiving audio data
            receiveLoop();
        }
        else
        {
            // Start playback
            play();
        }
    }

private:
    ////////////////////////////////////////////////////////////
    /// /see SoundStream::OnGetData
    ///
    ////////////////////////////////////////////////////////////
    bool onGetData(sf::base::Vector<sf::base::I16>& outBuffer) override
    {
        // We have reached the end of the buffer and all audio data have been played: we can stop playback
        if ((m_offset >= m_samples.size()) && m_hasFinished)
            return false;

        // No new data has arrived since last update: wait until we get some
        while ((m_offset >= m_samples.size()) && !m_hasFinished)
            sf::sleep(sf::milliseconds(10));

        // Copy samples into the base-owned output buffer to avoid synchronization problems
        // (don't forget that we run in two separate threads)
        {
            const std::lock_guard lock(m_mutex);
            outBuffer.assignRange(m_samples.begin() + static_cast<sf::base::Vector<sf::base::I16>::difference_type>(m_offset),
                                  m_samples.end());
        }

        // Update the playing offset
        m_offset += outBuffer.size();

        return true;
    }

    ////////////////////////////////////////////////////////////
    /// /see SoundStream::OnSeek
    ///
    ////////////////////////////////////////////////////////////
    void onSeek(sf::Time timeOffset) override
    {
        m_offset = static_cast<sf::base::SizeT>(timeOffset.asMilliseconds()) * 44'100 * 1 / 1000;
    }

    ////////////////////////////////////////////////////////////
    /// Get audio data from the client until playback is stopped
    ///
    ////////////////////////////////////////////////////////////
    void receiveLoop()
    {
        while (!m_hasFinished)
        {
            // Get waiting audio data from the network
            sf::Packet packet;
            if (m_client->receive(packet) != sf::Socket::Status::Done)
                break;

            // Extract the message ID
            sf::base::U8 id = 0;
            packet >> id;

            if (id == serverAudioData)
            {
                // Extract audio samples from the packet, and append it to our samples buffer
                const sf::base::SizeT sampleCount = (packet.getDataSize() - 1) / sizeof(sf::base::I16);

                // Don't forget that the other thread can access the sample array at any time
                // (so we protect any operation on it with the mutex)
                {
                    const std::lock_guard lock(m_mutex);
                    const auto*           begin = static_cast<const char*>(packet.getData()) + 1;
                    const auto*           end   = begin + sampleCount * sizeof(sf::base::I16);

                    for (const auto* it = begin; it != end; ++it)
                        m_samples.emplaceBack(*it);
                }
            }
            else if (id == serverEndOfStream)
            {
                // End of stream reached: we stop receiving audio data
                sf::cOut() << "Audio data has been 100% received!" << sf::endL;
                m_hasFinished = true;
            }
            else
            {
                // Something's wrong...
                sf::cOut() << "Invalid packet received..." << sf::endL;
                m_hasFinished = true;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    sf::base::Optional<sf::TcpListener> m_listener;
    sf::base::Optional<sf::TcpSocket>   m_client;
    std::recursive_mutex                m_mutex;
    sf::base::Vector<sf::base::I16>     m_samples;
    sf::base::SizeT                     m_offset{};
    bool                                m_hasFinished{};
};


////////////////////////////////////////////////////////////
/// Launch a server and wait for incoming audio data from
/// a connected client
///
////////////////////////////////////////////////////////////
void doServer(sf::PlaybackDevice& playbackDevice, unsigned short port)
{
    // Build an audio stream to play sound data as it is received through the network
    NetworkAudioStream audioStream(playbackDevice);
    audioStream.start(port);

    // Loop until the sound playback is finished
    while (audioStream.isPlaying())
    {
        // Leave some CPU time for other threads
        sf::sleep(sf::milliseconds(100));
    }

    sf::cIn().ignore(10'000, '\n');

    // Wait until the user presses 'enter' key
    sf::cOut() << "Press enter to replay the sound..." << sf::endL;
    sf::cIn().ignore(10'000, '\n');

    // Replay the sound (just to make sure replaying the received data is OK)
    audioStream.play();

    // Loop until the sound playback is finished
    while (audioStream.isPlaying())
    {
        // Leave some CPU time for other threads
        sf::sleep(sf::milliseconds(100));
    }
}
