////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/Sound.hpp"

#include "Zancle/Audio/AudioContext.hpp"
#include "Zancle/Audio/AudioSettings.hpp"
#include "Zancle/Audio/Music.hpp"
#include "Zancle/Audio/MusicReader.hpp"
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/PlaybackDeviceHandle.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"

#include "Zancle/Err/FmtPath.hpp"

#include "Zancle/Scn/ScnStdin.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"

#include "Zancle/IO/Path.hpp"

#include "Zancle/Concurrency/Thread.hpp"

#include "Zancle/Chrono/Time.hpp"


namespace
{
////////////////////////////////////////////////////////////
/// Play a sound
///
////////////////////////////////////////////////////////////
void playSound(za::PlaybackDevice& playbackDevice)
{
    // Load a sound buffer from a wav file
    const auto buffer = za::SoundBuffer::loadFromFile("resources/killdeer.wav").value();

    // Display sound information
    za::printLn("killdeer.wav:{} {} seconds{} {} samples / sec{} {} channels",
                '\n',
                buffer.getDuration().asSeconds(),
                '\n',
                buffer.getSampleRate(),
                '\n',
                buffer.getChannelCount());

    // Create a sound instance and play it
    za::Sound sound(playbackDevice, buffer);
    sound.play();

    // Loop while the sound is playing
    while (sound.isPlaying())
    {
        // Leave some CPU time for other processes
        za::ThisThread::sleepFor(za::milliseconds(100));

        // Display the playing position
        za::print("\rPlaying... {} sec        ", sound.getPlayingOffset().asSeconds());
    }

    za::printLn("");
}


////////////////////////////////////////////////////////////
/// Play a music
///
////////////////////////////////////////////////////////////
void playMusic(za::PlaybackDevice& playbackDevice, const za::Path& filename)
{
    // Load an ogg music file
    auto musicReader = za::MusicReader::openFromFile("resources" / filename).value();

    // Display music information
    za::printLn("{}:{} {} seconds{} {} samples / sec{} {} channels",
                filename,
                '\n',
                musicReader.getDuration().asSeconds(),
                '\n',
                musicReader.getSampleRate(),
                '\n',
                musicReader.getChannelCount());

    // Play it
    za::Music music(playbackDevice, musicReader);
    music.play();

    // Loop while the music is playing
    while (music.isPlaying())
    {
        // Leave some CPU time for other processes
        za::ThisThread::sleepFor(za::milliseconds(100));

        // Display the playing position
        za::print("\rPlaying... {} sec        ", music.getPlayingOffset().asSeconds());
    }

    za::printLn("");
}

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create an audio context and get the default playback device
    auto               audioContext = za::AudioContext::create().value();
    za::PlaybackDevice playbackDevice{za::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    // Play a sound
    playSound(playbackDevice);

    // Play music from an ogg file
    playMusic(playbackDevice, "doodle_pop.ogg");

    // Play music from a flac file
    playMusic(playbackDevice, "ding.flac");

    // Play music from a mp3 file
    playMusic(playbackDevice, "ding.mp3");

    // Wait until the user presses 'enter' key
    za::printLn("Press enter to exit...");
    za::scnStdinIgnoreLine();
}
