////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicSource.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/SoundSource.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include <vector>


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

    std::vector<sf::PlaybackDevice> playbackDevices;
    for (const sf::PlaybackDeviceHandle& deviceHandle : playbackDeviceHandles)
    {
        sf::cOut() << "  - " << deviceHandle.getName() << '\n';
        playbackDevices.emplace_back(deviceHandle);
    }

    // Load resources
    auto soundBuffer  = sf::SoundBuffer::loadFromFile("resources/killdeer.wav").value();
    auto musicSource0 = sf::MusicSource::openFromFile("resources/doodle_pop.ogg").value();
    auto musicSource1 = sf::MusicSource::openFromFile("resources/ding.flac").value();
    auto musicSource2 = sf::MusicSource::openFromFile("resources/ding.mp3").value();

    // Create sound sources
    sf::Sound sound(soundBuffer);
    sf::Music music0(musicSource0);
    sf::Music music1(musicSource1);
    sf::Music music2(musicSource2);

    // Store all source sources together for convenience
    sf::SoundSource* const sources[]{&sound, &music0, &music1, &music2};

    // Play multiple sources simultaneously on separate playback devices
    for (std::size_t i = 0u; i < playbackDevices.size(); ++i)
        sources[i % 4]->play(playbackDevices[i]);

    // Keep program alive while sounds are playing and display spinning icon
    const char  messageIcons[]{'-', '\\', '|', '/'};
    std::size_t messageIconIndex = 0u;

    const auto anySourcePlaying = [&]
    {
        for (const sf::SoundSource* source : sources)
            if (source->getStatus() == sf::SoundSource::Status::Playing)
                return true;

        return false;
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
