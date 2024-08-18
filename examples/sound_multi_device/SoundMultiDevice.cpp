////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioContextUtils.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/SoundSource.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include <iostream>


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create sound sources
    auto      soundBuffer = sf::SoundBuffer::loadFromFile("resources/killdeer.wav").value();
    sf::Sound sound(soundBuffer);

    auto music0 = sf::Music::openFromFile("resources/doodle_pop.ogg").value();
    auto music1 = sf::Music::openFromFile("resources/ding.flac").value();
    auto music2 = sf::Music::openFromFile("resources/ding.mp3").value();

    // Store all source sources together for convenience
    sf::SoundSource* const sources[]{&sound, &music0, &music1, &music2};

    // Create the audio context
    auto audioContext = sf::AudioContext::create().value();

    // For each hardware playback device, create a SFML playback device
    std::vector<sf::PlaybackDevice> playbackDevices;
    for (const sf::PlaybackDeviceHandle& deviceHandle :
         sf::AudioContextUtils::getAvailablePlaybackDeviceHandles(audioContext))
        playbackDevices.emplace_back(audioContext, deviceHandle);

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
        std::cout << "\rPlaying... " << messageIcons[messageIconIndex++ % 4] << std::flush;
    }

    std::cout << '\n' << std::endl;
}
