////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/CaptureDevice.hpp"
#include "SFML/Audio/CaptureDeviceHandle.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/SoundBufferRecorder.hpp"
#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include <string>

#include <cstdlib>


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
        sf::cErr() << "Sorry, audio capture is not supported by your system" << sf::endL;
        return EXIT_FAILURE;
    }

    // List the available capture device handles
    sf::cOut() << "Available capture devices:\n\n";

    for (sf::base::SizeT i = 0u; i < deviceHandles.size(); ++i)
        sf::cOut() << i << ": " << deviceHandles[i].getName() << '\n';

    sf::cOut() << '\n';

    // Choose the capture device
    sf::base::SizeT deviceIndex = 0;

    if (deviceHandles.size() > 1)
    {
        deviceIndex = deviceHandles.size();
        sf::cOut() << "Please choose the capture device to use [0-" << deviceHandles.size() - 1 << "]: ";

        do
        {
            sf::cIn() >> deviceIndex;
            sf::cIn().ignore(10'000, '\n');
        } while (deviceIndex >= deviceHandles.size());
    }

    // Choose the sample rate
    unsigned int sampleRate = 0;
    sf::cOut() << "Please choose the sample rate for sound capture (44100 is CD quality): ";
    sf::cIn() >> sampleRate;
    sf::cIn().ignore(10'000, '\n');

    // Wait for user input...
    sf::cOut() << "Press enter to start recording audio";
    sf::cIn().ignore(10'000, '\n');

    // Here we'll use an integrated custom recorder, which saves the captured data into a sound buffer
    sf::SoundBufferRecorder recorder;

    // Create the capture device
    sf::CaptureDevice captureDevice(deviceHandles[deviceIndex]);

    // Audio capture is done in a separate thread, so we can block the main thread while it is capturing
    if (!recorder.start(captureDevice, sampleRate))
    {
        sf::cErr() << "Failed to start recorder" << sf::endL;
        return EXIT_FAILURE;
    }

    sf::cOut() << "Recording... press enter to stop";
    sf::cIn().ignore(10'000, '\n');

    if (!recorder.stop())
        sf::cErr() << "Failed to stop sound buffer recorder" << sf::endL;

    // Get the buffer containing the captured data
    const sf::SoundBuffer& buffer = recorder.getBuffer();

    // Display captured sound information
    sf::cOut() << "Sound information:" << '\n'
               << " " << buffer.getDuration().asSeconds() << " seconds" << '\n'
               << " " << buffer.getSampleRate() << " samples / seconds" << '\n'
               << " " << buffer.getChannelCount() << " channels" << '\n';

    // Choose what to do with the recorded sound data
    char choice = 0;
    sf::cOut() << "What do you want to do with captured sound (p = play, s = save) ? ";
    sf::cIn() >> choice;
    sf::cIn().ignore(10'000, '\n');

    if (choice == 's')
    {
        // Choose the filename
        std::string filename;
        sf::cOut() << "Choose the file to create: ";
        sf::getLine(sf::cIn(), filename);

        // Save the buffer
        if (!buffer.saveToFile(filename))
            sf::cErr() << "Could not save sound buffer to file" << sf::endL;
    }
    else
    {
        // Create the default playback device
        auto playbackDevice = sf::PlaybackDevice::createDefault().value();

        // Create a sound instance and play it
        sf::Sound sound(buffer);
        sound.play(playbackDevice);

        // Wait until finished
        while (sound.getStatus() == sf::Sound::Status::Playing)
        {
            // Display the playing position
            sf::cOut() << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        ";
            sf::cOut() << sf::flush;

            // Leave some CPU time for other threads
            sf::sleep(sf::milliseconds(100));
        }
    }

    // Finished!
    sf::cOut() << '\n' << "Done!\n";

    // Wait until the user presses 'enter' key
    sf::cOut() << "Press enter to exit..." << sf::endL;
    sf::cIn().ignore(10'000, '\n');
}
