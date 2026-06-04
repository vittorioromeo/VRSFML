////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/AudioContext.hpp"
#include "Zancle/Audio/AudioSettings.hpp"
#include "Zancle/Audio/Music.hpp"
#include "Zancle/Audio/MusicReader.hpp"
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/PlaybackDeviceHandle.hpp"
#include "Zancle/Audio/Sound.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"
#include "Zancle/System/IO.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Thread.hpp"
#include "Zancle/System/Time.hpp"
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/InPlaceVector.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the audio context
    auto audioContext = za::AudioContext::create().value();

    // For each hardware playback device, create a SFML playback device
    const auto playbackDeviceHandles = za::AudioContext::getAvailablePlaybackDeviceHandles();
    if (playbackDeviceHandles.empty())
    {
        zb::printErrLn("No playback devices found.");
        return 1;
    }

    zb::printLn("Found {} playback devices:", playbackDeviceHandles.size());

    zb::InPlaceVector<za::PlaybackDevice, 8> playbackDevices;
    for (const za::PlaybackDeviceHandle& deviceHandle : playbackDeviceHandles)
    {
        zb::printLn("  - {}", deviceHandle.getName());
        playbackDevices.emplaceBack(deviceHandle);
    }

    // Load resources
    auto soundBuffer  = za::SoundBuffer::loadFromFile("resources/killdeer.wav").value();
    auto musicSource0 = za::MusicReader::openFromFile("resources/doodle_pop.ogg").value();
    auto musicSource1 = za::MusicReader::openFromFile("resources/ding.flac").value();
    auto musicSource2 = za::MusicReader::openFromFile("resources/ding.mp3").value();

    // Create sound sources
    za::Sound sound(playbackDevices[0], soundBuffer);
    sound.play();

    zb::Optional<za::Music> music0;
    zb::Optional<za::Music> music1;
    zb::Optional<za::Music> music2;

    if (playbackDevices.size() > 1u)
        music0.emplace(playbackDevices[1], musicSource0).play();

    if (playbackDevices.size() > 2u)
        music1.emplace(playbackDevices[2], musicSource1).play();

    if (playbackDevices.size() > 3u)
        music2.emplace(playbackDevices[3], musicSource2).play();

    // Keep program alive while sounds are playing and display spinning icon
    const char messageIcons[]{'-', '\\', '|', '/'};
    zb::SizeT  messageIconIndex = 0u;

    const auto anySourcePlaying = [&]
    {
        return sound.isPlaying() ||                          //
               (music0.hasValue() && music0->isPlaying()) || //
               (music1.hasValue() && music1->isPlaying()) || //
               (music2.hasValue() && music2->isPlaying());
    };

    while (anySourcePlaying())
    {
        // Leave some CPU time for other processes
        za::ThisThread::sleepFor(za::milliseconds(100));

        // Display a message to show we're not frozen
        zb::print("\rPlaying... {}", messageIcons[messageIconIndex++ % 4]);
    }

    zb::printLn("");
}
