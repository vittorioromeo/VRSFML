////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/CaptureDevice.hpp>
#include <SFML/Audio/CaptureDeviceHandle.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundBufferRecorder.hpp>
#include <SFML/Audio/SoundRecorder.hpp>

#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <iostream>

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
    auto deviceHandles = audioContext.getAvailableCaptureDeviceHandles();

    // Check if any device can capture audio
    if (deviceHandles.empty())
    {
        std::cerr << "Sorry, audio capture is not supported by your system" << std::endl;
        return EXIT_FAILURE;
    }

    // List the available capture device handles
    std::cout << "Available capture devices:\n\n";

    for (std::size_t i = 0u; i < deviceHandles.size(); ++i)
        std::cout << i << ": " << deviceHandles[i].getName() << '\n';

    std::cout << '\n';

    // Choose the capture device
    std::size_t deviceIndex = 0;

    if (deviceHandles.size() > 1)
    {
        deviceIndex = deviceHandles.size();
        std::cout << "Please choose the capture device to use [0-" << deviceHandles.size() - 1 << "]: ";

        do
        {
            std::cin >> deviceIndex;
            std::cin.ignore(10000, '\n');
        } while (deviceIndex >= deviceHandles.size());
    }

    // Choose the sample rate
    unsigned int sampleRate = 0;
    std::cout << "Please choose the sample rate for sound capture (44100 is CD quality): ";
    std::cin >> sampleRate;
    std::cin.ignore(10000, '\n');

    // Wait for user input...
    std::cout << "Press enter to start recording audio";
    std::cin.ignore(10000, '\n');

    // Here we'll use an integrated custom recorder, which saves the captured data into a sound buffer
    sf::SoundBufferRecorder recorder;

    // Create the capture device
    sf::CaptureDevice captureDevice(audioContext, deviceHandles[deviceIndex]);

    // Audio capture is done in a separate thread, so we can block the main thread while it is capturing
    if (!recorder.start(captureDevice, sampleRate))
    {
        std::cerr << "Failed to start recorder" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Recording... press enter to stop";
    std::cin.ignore(10000, '\n');

    if (!recorder.stop())
        std::cerr << "Failed to stop sound buffer recorder" << std::endl;

    // Get the buffer containing the captured data
    const sf::SoundBuffer& buffer = recorder.getBuffer();

    // Display captured sound information
    std::cout << "Sound information:" << '\n'
              << " " << buffer.getDuration().asSeconds() << " seconds" << '\n'
              << " " << buffer.getSampleRate() << " samples / seconds" << '\n'
              << " " << buffer.getChannelCount() << " channels" << '\n';

    // Choose what to do with the recorded sound data
    char choice = 0;
    std::cout << "What do you want to do with captured sound (p = play, s = save) ? ";
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice == 's')
    {
        // Choose the filename
        std::string filename;
        std::cout << "Choose the file to create: ";
        std::getline(std::cin, filename);

        // Save the buffer
        if (!buffer.saveToFile(filename))
            std::cerr << "Could not save sound buffer to file" << std::endl;
    }
    else
    {
        // Create the default playback device
        auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

        // Create a sound instance and play it
        sf::Sound sound(buffer);
        sound.play(playbackDevice);

        // Wait until finished
        while (sound.getStatus() == sf::Sound::Status::Playing)
        {
            // Display the playing position
            std::cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        ";
            std::cout << std::flush;

            // Leave some CPU time for other threads
            sf::sleep(sf::milliseconds(100));
        }
    }

    // Finished!
    std::cout << '\n' << "Done!\n";

    // Wait until the user presses 'enter' key
    std::cout << "Press enter to exit..." << std::endl;
    std::cin.ignore(10000, '\n');
}
