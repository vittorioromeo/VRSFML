////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the audio context
    auto audioContext = sf::AudioContext::create().value();

    // For each hardware playback device, create a SFML playback device
    const auto playbackDeviceHandles = sf::AudioContext::getAvailablePlaybackDeviceHandles();
    if (playbackDeviceHandles.empty())
    {
        sf::cErr() << "No playback devices found.\n";
        return 1;
    }

    sf::cOut() << "Found " << playbackDeviceHandles.size() << " playback devices:\n";

    sf::base::InPlaceVector<sf::PlaybackDevice, 8> playbackDevices;
    for (const sf::PlaybackDeviceHandle& deviceHandle : playbackDeviceHandles)
    {
        sf::cOut() << "  - " << deviceHandle.getName() << '\n';
        playbackDevices.emplaceBack(deviceHandle);
    }

    // Load resources
    auto soundBuffer  = sf::SoundBuffer::loadFromFile("resources/killdeer.wav").value();
    auto musicSource0 = sf::MusicReader::openFromFile("resources/doodle_pop.ogg").value();
    auto musicSource1 = sf::MusicReader::openFromFile("resources/ding.flac").value();
    auto musicSource2 = sf::MusicReader::openFromFile("resources/ding.mp3").value();

    // Create sound sources
    sf::Sound                     sound(playbackDevices[0], soundBuffer);
    sf::base::Optional<sf::Music> music0;
    sf::base::Optional<sf::Music> music1;
    sf::base::Optional<sf::Music> music2;

    if (playbackDevices.size() > 1u)
        music0.emplace(playbackDevices[1], musicSource0);

    if (playbackDevices.size() > 2u)
        music1.emplace(playbackDevices[2], musicSource1);

    if (playbackDevices.size() > 3u)
        music2.emplace(playbackDevices[3], musicSource2);

    // Keep program alive while sounds are playing and display spinning icon
    const char      messageIcons[]{'-', '\\', '|', '/'};
    sf::base::SizeT messageIconIndex = 0u;

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
        sf::sleep(sf::milliseconds(100));

        // Display a message to show we're not frozen
        sf::cOut() << "\rPlaying... " << messageIcons[messageIconIndex++ % 4] << sf::flush;
    }

    sf::cOut() << '\n' << sf::endL;
}
