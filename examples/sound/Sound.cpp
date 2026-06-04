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

#include "Zancle/System/Fmt/FmtPath.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Thread.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"


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
    zb::printLn("killdeer.wav:{} {} seconds{} {} samples / sec{} {} channels",
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
        zb::print("\rPlaying... {} sec        ", sound.getPlayingOffset().asSeconds());
    }

    zb::printLn("");
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
    zb::printLn("{}:{} {} seconds{} {} samples / sec{} {} channels",
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
        zb::print("\rPlaying... {} sec        ", music.getPlayingOffset().asSeconds());
    }

    zb::printLn("");
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
    zb::printLn("Press enter to exit...");
    zb::scnStdinIgnoreLine();
}
