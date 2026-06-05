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

#include "Zancle/IO/IO.hpp"
#include "Zancle/IO/Path.hpp"
#include "Zancle/Concurrency/Thread.hpp"
#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"
#include "Zancle/Container/InPlaceVector.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the audio context
    auto audioContext = za::AudioContext::create().value();

    // For each hardware playback device, create a Zancle playback device
    const auto playbackDeviceHandles = za::AudioContext::getAvailablePlaybackDeviceHandles();
    if (playbackDeviceHandles.empty())
    {
        za::printErrLn("No playback devices found.");
        return 1;
    }

    za::printLn("Found {} playback devices:", playbackDeviceHandles.size());

    za::InPlaceVector<za::PlaybackDevice, 8> playbackDevices;
    for (const za::PlaybackDeviceHandle& deviceHandle : playbackDeviceHandles)
    {
        za::printLn("  - {}", deviceHandle.getName());
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

    za::Optional<za::Music> music0;
    za::Optional<za::Music> music1;
    za::Optional<za::Music> music2;

    if (playbackDevices.size() > 1u)
        music0.emplace(playbackDevices[1], musicSource0).play();

    if (playbackDevices.size() > 2u)
        music1.emplace(playbackDevices[2], musicSource1).play();

    if (playbackDevices.size() > 3u)
        music2.emplace(playbackDevices[3], musicSource2).play();

    // Keep program alive while sounds are playing and display spinning icon
    const char messageIcons[]{'-', '\\', '|', '/'};
    za::SizeT  messageIconIndex = 0u;

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
        za::print("\rPlaying... {}", messageIcons[messageIconIndex++ % 4]);
    }

    za::printLn("");
}
