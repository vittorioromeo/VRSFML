////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundSource.hpp"

#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/MusicSource.hpp>
#include <SFML/Audio/MusicStream.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/PlaybackDeviceHandle.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <algorithm>
#include <iostream>


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Load audio resources
    auto resource0 = sf::SoundBuffer::loadFromFile("resources/killdeer.wav").value();
    auto resource1 = sf::MusicSource::openFromFile("resources/doodle_pop.ogg").value();
    auto resource2 = sf::MusicSource::openFromFile("resources/ding.flac").value();
    auto resource3 = sf::MusicSource::openFromFile("resources/ding.mp3").value();

    // Create sound sources
    sf::Sound source0(resource0);
    auto      source1 = resource1.createStream();
    auto      source2 = resource2.createStream();
    auto      source3 = resource3.createStream();

    // Store all source sources together for convenience
    sf::SoundSource* const sources[]{&source0, &source1, &source2, &source3};

    // Create the audio context
    auto audioContext = sf::AudioContext::create().value();

    // For each hardware playback device, create a SFML playback device
    std::vector<sf::PlaybackDevice> playbackDevices;
    for (const sf::PlaybackDeviceHandle& deviceHandle : audioContext.getAvailablePlaybackDeviceHandles())
        playbackDevices.emplace_back(audioContext, deviceHandle);

    // Play multiple sources simultaneously on separate playback devices
    for (std::size_t i = 0u; i < playbackDevices.size(); ++i)
        sources[i % 4]->play(playbackDevices[i]);

    // Keep program alive while sounds are playing
    const char  messageIcons[]{'-', '\\', '|', '/'};
    std::size_t messageIconIndex = 0u;

    while (std::any_of(sources,
                       sources + 4,
                       [](const sf::SoundSource* soundSource)
                       { return soundSource->getStatus() == sf::SoundSource::Status::Playing; }))
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display a message to show we're not frozen
        std::cout << "\rPlaying... " << messageIcons[messageIconIndex++ % 4] << std::flush;
    }

    std::cout << '\n' << std::endl;
}
