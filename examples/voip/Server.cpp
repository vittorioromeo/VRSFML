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


namespace
{
////////////////////////////////////////////////////////////
/// State shared between the audio thread (`onGetData`/`onSeek`)
/// and the network thread (`receiveLoop`). All heap-owned
/// members live here; because the `SoundStream` template
/// guarantees the state outlives the audio callback, there is
/// no destructor-order UAF hazard.
////////////////////////////////////////////////////////////
struct NetworkState
{
    mutable std::recursive_mutex    mutex;
    sf::base::Vector<sf::base::I16> samples;
    sf::base::SizeT                 offset{};
    bool                            hasFinished{};

    ////////////////////////////////////////////////////////////
    bool onGetData(sf::base::Vector<sf::base::I16>& outBuffer)
    {
        if ((offset >= samples.size()) && hasFinished)
            return false;

        // No new data has arrived since last update: wait until we get some
        while ((offset >= samples.size()) && !hasFinished)
            sf::sleep(sf::milliseconds(10));

        {
            const std::lock_guard lock(mutex);
            outBuffer.assignRange(samples.begin() + static_cast<sf::base::Vector<sf::base::I16>::difference_type>(offset),
                                  samples.end());
        }

        offset += outBuffer.size();
        return true;
    }

    ////////////////////////////////////////////////////////////
    void onSeek(sf::Time timeOffset)
    {
        offset = static_cast<sf::base::SizeT>(timeOffset.asMilliseconds()) * 44'100 * 1 / 1000;
    }
};


////////////////////////////////////////////////////////////
/// Customized sound stream for acquiring audio data
/// from the network
////////////////////////////////////////////////////////////
class NetworkAudioStream : public sf::SoundStream<NetworkState>
{
public:
    ////////////////////////////////////////////////////////////
    NetworkAudioStream(sf::PlaybackDevice& playbackDevice) :
        sf::SoundStream<NetworkState>{playbackDevice, sf::ChannelMap{sf::SoundChannel::Mono}, 44'100u}
    {
    }

    ////////////////////////////////////////////////////////////
    /// Run the server, stream audio data from the client
    ///
    ////////////////////////////////////////////////////////////
    void start(unsigned short port)
    {
        auto& s = state();

        if (!s.hasFinished)
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

            play();
            receiveLoop();
        }
        else
        {
            play();
        }
    }

private:
    ////////////////////////////////////////////////////////////
    /// Get audio data from the client until playback is stopped
    ////////////////////////////////////////////////////////////
    void receiveLoop()
    {
        auto& s = state();

        while (!s.hasFinished)
        {
            sf::Packet packet;
            if (m_client->receive(packet) != sf::Socket::Status::Done)
                break;

            sf::base::U8 id = 0;
            packet >> id;

            if (id == serverAudioData)
            {
                const sf::base::SizeT sampleCount = (packet.getDataSize() - 1) / sizeof(sf::base::I16);
                {
                    const std::lock_guard lock(s.mutex);
                    const auto*           begin = static_cast<const char*>(packet.getData()) + 1;
                    const auto*           end   = begin + sampleCount * sizeof(sf::base::I16);

                    for (const auto* it = begin; it != end; ++it)
                        s.samples.emplaceBack(*it);
                }
            }
            else if (id == serverEndOfStream)
            {
                sf::cOut() << "Audio data has been 100% received!" << sf::endL;
                s.hasFinished = true;
            }
            else
            {
                sf::cOut() << "Invalid packet received..." << sf::endL;
                s.hasFinished = true;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    // Member data (network-only; audio-thread state lives in `state()`)
    ////////////////////////////////////////////////////////////
    sf::base::Optional<sf::TcpListener> m_listener;
    sf::base::Optional<sf::TcpSocket>   m_client;
};

} // namespace


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
