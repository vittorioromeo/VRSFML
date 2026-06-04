////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Server.hpp"

#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/SoundChannel.hpp"
#include "Zancle/Audio/SoundStream.hpp"

#include "Zancle/Network/IpAddressUtils.hpp"
#include "Zancle/Network/Packet.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/TcpListener.hpp"
#include "Zancle/Network/TcpSocket.hpp"

#include "Zancle/System/Thread.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp" // IWYU pragma: keep
#include "ZancleBase/Vector.hpp"

#include <mutex>


constexpr zb::U8 serverAudioData   = 1;
constexpr zb::U8 serverEndOfStream = 2;


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
    mutable std::recursive_mutex mutex;
    zb::Vector<zb::I16>          samples;
    zb::SizeT                    offset{};
    bool                         hasFinished{};

    ////////////////////////////////////////////////////////////
    bool onGetData(zb::Vector<zb::I16>& outBuffer)
    {
        if ((offset >= samples.size()) && hasFinished)
            return false;

        // No new data has arrived since last update: wait until we get some
        while ((offset >= samples.size()) && !hasFinished)
            za::ThisThread::sleepFor(za::milliseconds(10));

        {
            const std::lock_guard lock(mutex);
            outBuffer.assignRange(samples.begin() + static_cast<zb::Vector<zb::I16>::difference_type>(offset),
                                  samples.end());
        }

        offset += outBuffer.size();
        return true;
    }

    ////////////////////////////////////////////////////////////
    void onSeek(za::Time timeOffset)
    {
        offset = static_cast<zb::SizeT>(timeOffset.asMilliseconds()) * 44'100 * 1 / 1000;
    }
};


////////////////////////////////////////////////////////////
/// Customized sound stream for acquiring audio data
/// from the network
////////////////////////////////////////////////////////////
class NetworkAudioStream : public za::SoundStream<NetworkState>
{
public:
    ////////////////////////////////////////////////////////////
    NetworkAudioStream(za::PlaybackDevice& playbackDevice) :
        za::SoundStream<NetworkState>{playbackDevice, za::ChannelMap{za::SoundChannel::Mono}, 44'100u}
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
            m_listener = za::TcpListener::create(port, /* isBlocking */ true);
            if (!m_listener.hasValue())
                return;
            zb::printLn("Server is listening to port {}, waiting for connections... ", port);

            // Wait for a connection
            auto acceptResult = m_listener->accept();
            if (acceptResult.status != za::Socket::Status::Done)
                return;
            m_client = ZB_MOVE(acceptResult.socket);
            zb::printLn("Client connected: {}", za::IpAddressUtils::toString(m_client->getRemoteAddress().value()));

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
            za::Packet packet;
            if (m_client->receive(packet) != za::Socket::Status::Done)
                break;

            zb::U8 id = 0;
            packet >> id;

            if (id == serverAudioData)
            {
                const zb::SizeT sampleCount = (packet.getDataSize() - 1) / sizeof(zb::I16);
                {
                    const std::lock_guard lock(s.mutex);
                    const auto*           begin = static_cast<const char*>(packet.getData()) + 1;
                    const auto*           end   = begin + sampleCount * sizeof(zb::I16);

                    for (const auto* it = begin; it != end; ++it)
                        s.samples.emplaceBack(*it);
                }
            }
            else if (id == serverEndOfStream)
            {
                zb::printLn("Audio data has been 100% received!");
                s.hasFinished = true;
            }
            else
            {
                zb::printLn("Invalid packet received...");
                s.hasFinished = true;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    // Member data (network-only; audio-thread state lives in `state()`)
    ////////////////////////////////////////////////////////////
    zb::Optional<za::TcpListener> m_listener;
    zb::Optional<za::TcpSocket>   m_client;
};

} // namespace


////////////////////////////////////////////////////////////
/// Launch a server and wait for incoming audio data from
/// a connected client
///
////////////////////////////////////////////////////////////
void doServer(za::PlaybackDevice& playbackDevice, unsigned short port)
{
    // Build an audio stream to play sound data as it is received through the network
    NetworkAudioStream audioStream(playbackDevice);
    audioStream.start(port);

    // Loop until the sound playback is finished
    while (audioStream.isPlaying())
    {
        // Leave some CPU time for other threads
        za::ThisThread::sleepFor(za::milliseconds(100));
    }

    zb::scnStdinIgnoreLine();

    // Wait until the user presses 'enter' key
    zb::printLn("Press enter to replay the sound...");
    zb::scnStdinIgnoreLine();

    // Replay the sound (just to make sure replaying the received data is OK)
    audioStream.play();

    // Loop until the sound playback is finished
    while (audioStream.isPlaying())
    {
        // Leave some CPU time for other threads
        za::ThisThread::sleepFor(za::milliseconds(100));
    }
}
