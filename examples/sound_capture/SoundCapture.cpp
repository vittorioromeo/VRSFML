////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/CaptureDevice.hpp"
#include "SFML/Audio/CaptureDeviceHandle.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/SoundBufferRecorder.hpp"
#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/Thread.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Scn/ScnStdin.hpp"
#include "SFML/Base/Scn/ScnString.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the audio context
    auto audioContext = sf::AudioContext::create().value();

    // Get the available capture devices
    auto deviceHandles = sf::AudioContext::getAvailableCaptureDeviceHandles();

    // Check if any device can capture audio
    if (deviceHandles.empty())
    {
        sf::base::printErrLn("Sorry, audio capture is not supported by your system");
        return 1;
    }

    // List the available capture device handles
    sf::base::printLn("Available capture devices:\n");

    for (sf::base::SizeT i = 0u; i < deviceHandles.size(); ++i)
        sf::base::printLn("{}: {}", i, deviceHandles[i].getName());

    sf::base::printLn("");

    // Choose the capture device
    sf::base::SizeT deviceIndex = 0;

    if (deviceHandles.size() > 1)
    {
        deviceIndex = deviceHandles.size();
        sf::base::print("Please choose the capture device to use [0-{}]: ", deviceHandles.size() - 1);

        do
        {
            (void)sf::base::scnStdinInto(deviceIndex);
            sf::base::scnStdinIgnoreLine();
        } while (deviceIndex >= deviceHandles.size());
    }

    // Choose the sample rate
    unsigned int sampleRate = 0;
    sf::base::print("Please choose the sample rate for sound capture (44100 is CD quality): ");
    (void)sf::base::scnStdinInto(sampleRate);
    sf::base::scnStdinIgnoreLine();

    // Wait for user input...
    sf::base::print("Press enter to start recording audio");
    sf::base::scnStdinIgnoreLine();

    // Create the capture device
    sf::CaptureDevice captureDevice(deviceHandles[deviceIndex]);

    // Here we'll use an integrated custom recorder, which saves the captured data into a sound buffer
    sf::SoundBufferRecorder recorder;

    // Audio capture is done in a separate thread, so we can block the main thread while it is capturing
    if (!recorder.start(captureDevice, sampleRate))
    {
        sf::base::printErrLn("Failed to start recorder");
        return 1;
    }

    sf::base::print("Recording... press enter to stop");
    sf::base::scnStdinIgnoreLine();

    if (!recorder.stop())
        sf::base::printErrLn("Failed to stop sound buffer recorder");

    // Get the buffer containing the captured data
    const sf::SoundBuffer& buffer = recorder.getBuffer();

    // Display captured sound information
    sf::base::printLn("Sound information:{} {} seconds{} {} samples / seconds{} {} channels",
                      '\n',
                      buffer.getDuration().asSeconds(),
                      '\n',
                      buffer.getSampleRate(),
                      '\n',
                      buffer.getChannelCount());

    // Choose what to do with the recorded sound data
    sf::base::print("What do you want to do with captured sound (p = play, s = save) ? ");
    const char choice = sf::base::scnStdin<char>().valueOr('p');
    sf::base::scnStdinIgnoreLine();

    if (choice == 's')
    {
        // Choose the filename
        sf::base::String filename;
        sf::base::print("Choose the file to create: ");
        (void)sf::base::scnStdinReadLine(filename);

        // Save the buffer
        if (!buffer.saveToFile(filename))
            sf::base::printErrLn("Could not save sound buffer to file");
    }
    else
    {
        // Create the default playback device
        sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

        // Create a sound instance and play it
        sf::Sound sound(playbackDevice, buffer);
        sound.play();

        // Wait until finished
        while (sound.isPlaying())
        {
            // Display the playing position
            sf::base::print("\rPlaying... {} sec        ", sound.getPlayingOffset().asSeconds());
            sf::base::print("");

            // Leave some CPU time for other threads
            sf::ThisThread::sleepFor(sf::milliseconds(100));
        }
    }

    // Finished!
    sf::base::printLn("\nDone!");

    // Wait until the user presses 'enter' key
    sf::base::printLn("Press enter to exit...");
    sf::base::scnStdinIgnoreLine();
}
