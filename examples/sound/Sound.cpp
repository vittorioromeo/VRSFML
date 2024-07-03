////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioDeviceHandle.hpp"

#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/MusicSource.hpp>
#include <SFML/Audio/MusicStream.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <iostream>


////////////////////////////////////////////////////////////
/// Play a sound
///
////////////////////////////////////////////////////////////
void playSound(sf::PlaybackDevice& playbackDevice)
{
    // Load a sound buffer from a wav file
    const auto buffer = sf::SoundBuffer::loadFromFile("resources/killdeer.wav").value();

    // Display sound information
    std::cout << "killdeer.wav:" << '\n'
              << " " << buffer.getDuration().asSeconds() << " seconds" << '\n'
              << " " << buffer.getSampleRate() << " samples / sec" << '\n'
              << " " << buffer.getChannelCount() << " channels" << '\n';

    // Create a sound instance and play it
    sf::Sound sound(playbackDevice, buffer);
    sound.play();

    // Loop while the sound is playing
    while (sound.getStatus() == sf::Sound::Status::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        " << std::flush;
    }

    std::cout << '\n' << std::endl;
}


////////////////////////////////////////////////////////////
/// Play a music
///
////////////////////////////////////////////////////////////
void playMusic(sf::PlaybackDevice& playbackDevice, const std::filesystem::path& filename)
{
    // Load an ogg music file
    auto music = sf::MusicSource::openFromFile("resources" / filename).value();

    // Display music information
    std::cout << filename << ":" << '\n'
              << " " << music.getDuration().asSeconds() << " seconds" << '\n'
              << " " << music.getSampleRate() << " samples / sec" << '\n'
              << " " << music.getChannelCount() << " channels" << '\n';

    // TODO
    auto musicStream = music.createStream(playbackDevice);

    // Play it
    musicStream.play();

    // Loop while the music is playing
    while (musicStream.getStatus() == sf::MusicStream::Status::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << musicStream.getPlayingOffset().asSeconds() << " sec        " << std::flush;
    }

    std::cout << '\n' << std::endl;
}


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // TODO
    auto audioContext        = sf::AudioContext::create().value();
    auto defaultDeviceHandle = audioContext.getDefaultDevice().value();
    auto playbackDevice      = sf::PlaybackDevice(audioContext, defaultDeviceHandle);

    // Play a sound
    playSound(playbackDevice);

    // Play music from an ogg file
    playMusic(playbackDevice, "doodle_pop.ogg");

    // Play music from a flac file
    playMusic(playbackDevice, "ding.flac");

    // Play music from a mp3 file
    playMusic(playbackDevice, "ding.mp3");

    // Wait until the user presses 'enter' key
    std::cout << "Press enter to exit..." << std::endl;
    std::cin.ignore(10000, '\n');
}
