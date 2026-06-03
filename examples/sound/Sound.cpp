////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Sound.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Fmt/FmtPath.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Thread.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Scn/ScnStdin.hpp"


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
    sf::base::printLn("killdeer.wav:{} {} seconds{} {} samples / sec{} {} channels",
                      '\n',
                      buffer.getDuration().asSeconds(),
                      '\n',
                      buffer.getSampleRate(),
                      '\n',
                      buffer.getChannelCount());

    // Create a sound instance and play it
    sf::Sound sound(playbackDevice, buffer);
    sound.play();

    // Loop while the sound is playing
    while (sound.isPlaying())
    {
        // Leave some CPU time for other processes
        sf::ThisThread::sleepFor(sf::milliseconds(100));

        // Display the playing position
        sf::base::print("\rPlaying... {} sec        ", sound.getPlayingOffset().asSeconds());
    }

    sf::base::printLn("");
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
    sf::base::printLn("{}:{} {} seconds{} {} samples / sec{} {} channels",
                      filename,
                      '\n',
                      musicReader.getDuration().asSeconds(),
                      '\n',
                      musicReader.getSampleRate(),
                      '\n',
                      musicReader.getChannelCount());

    // Play it
    sf::Music music(playbackDevice, musicReader);
    music.play();

    // Loop while the music is playing
    while (music.isPlaying())
    {
        // Leave some CPU time for other processes
        sf::ThisThread::sleepFor(sf::milliseconds(100));

        // Display the playing position
        sf::base::print("\rPlaying... {} sec        ", music.getPlayingOffset().asSeconds());
    }

    sf::base::printLn("");
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
    sf::base::printLn("Press enter to exit...");
    sf::base::scnStdinIgnoreLine();
}
