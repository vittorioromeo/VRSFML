////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"


namespace
{
////////////////////////////////////////////////////////////
/// Play a sound
///
////////////////////////////////////////////////////////////
void playSound(sf::PlaybackDevice& playbackDevice)
{
    // Load a sound buffer from a wav file
    const auto buffer = sf::SoundBuffer::loadFromFile("resources/killdeer.wav").value();

    // Display sound information
    sf::cOut() << "killdeer.wav:" << '\n'
               << " " << buffer.getDuration().asSeconds() << " seconds" << '\n'
               << " " << buffer.getSampleRate() << " samples / sec" << '\n'
               << " " << buffer.getChannelCount() << " channels" << '\n';

    // Create a sound instance and play it
    sf::Sound sound(playbackDevice, buffer, sf::AudioSettings{});

    // Loop while the sound is playing
    while (sound.isPlaying())
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        sf::cOut() << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        " << sf::flush;
    }

    sf::cOut() << '\n' << sf::endL;
}


////////////////////////////////////////////////////////////
/// Play a music
///
////////////////////////////////////////////////////////////
void playMusic(sf::PlaybackDevice& playbackDevice, const sf::Path& filename)
{
    // Load an ogg music file
    auto musicReader = sf::MusicReader::openFromFile("resources" / filename).value();

    // Display music information
    sf::cOut() << filename << ":" << '\n'
               << " " << musicReader.getDuration().asSeconds() << " seconds" << '\n'
               << " " << musicReader.getSampleRate() << " samples / sec" << '\n'
               << " " << musicReader.getChannelCount() << " channels" << '\n';

    // Play it
    sf::Music music(playbackDevice, musicReader, sf::AudioSettings{});

    // Loop while the music is playing
    while (music.isPlaying())
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        sf::cOut() << "\rPlaying... " << music.getPlayingOffset().asSeconds() << " sec        " << sf::flush;
    }

    sf::cOut() << '\n' << sf::endL;
}

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create an audio context and get the default playback device
    auto               audioContext = sf::AudioContext::create().value();
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    // Play a sound
    playSound(playbackDevice);

    // Play music from an ogg file
    playMusic(playbackDevice, "doodle_pop.ogg");

    // Play music from a flac file
    playMusic(playbackDevice, "ding.flac");

    // Play music from a mp3 file
    playMusic(playbackDevice, "ding.mp3");

    // Wait until the user presses 'enter' key
    sf::cOut() << "Press enter to exit..." << sf::endL;
    sf::cIn().ignore(10'000, '\n');
}
