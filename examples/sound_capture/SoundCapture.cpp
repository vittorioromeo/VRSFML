////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/AudioContext.hpp"
#include "Zancle/Audio/AudioSettings.hpp"
#include "Zancle/Audio/CaptureDevice.hpp"
#include "Zancle/Audio/CaptureDeviceHandle.hpp"
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/Sound.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"
#include "Zancle/Audio/SoundBufferRecorder.hpp"
#include "Zancle/Audio/SoundRecorder.hpp"

#include "Zancle/Scn/ScnStdin.hpp"
#include "Zancle/Scn/ScnString.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"

#include "Zancle/IO/Path.hpp"

#include "Zancle/Concurrency/Thread.hpp"

#include "Zancle/String/String.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the audio context
    auto audioContext = za::AudioContext::create().value();

    // Get the available capture devices
    auto deviceHandles = za::AudioContext::getAvailableCaptureDeviceHandles();

    // Check if any device can capture audio
    if (deviceHandles.empty())
    {
        za::printErrLn("Sorry, audio capture is not supported by your system");
        return 1;
    }

    // List the available capture device handles
    za::printLn("Available capture devices:\n");

    for (za::SizeT i = 0u; i < deviceHandles.size(); ++i)
        za::printLn("{}: {}", i, deviceHandles[i].getName());

    za::printLn("");

    // Choose the capture device
    za::SizeT deviceIndex = 0;

    if (deviceHandles.size() > 1)
    {
        deviceIndex = deviceHandles.size();
        za::print("Please choose the capture device to use [0-{}]: ", deviceHandles.size() - 1);

        do
        {
            (void)za::scnStdinInto(deviceIndex);
            za::scnStdinIgnoreLine();
        } while (deviceIndex >= deviceHandles.size());
    }

    // Choose the sample rate
    unsigned int sampleRate = 0;
    za::print("Please choose the sample rate for sound capture (44100 is CD quality): ");
    (void)za::scnStdinInto(sampleRate);
    za::scnStdinIgnoreLine();

    // Wait for user input...
    za::print("Press enter to start recording audio");
    za::scnStdinIgnoreLine();

    // Create the capture device
    za::CaptureDevice captureDevice(deviceHandles[deviceIndex]);

    // Here we'll use an integrated custom recorder, which saves the captured data into a sound buffer
    za::SoundBufferRecorder recorder;

    // Audio capture is done in a separate thread, so we can block the main thread while it is capturing
    if (!recorder.start(captureDevice, sampleRate))
    {
        za::printErrLn("Failed to start recorder");
        return 1;
    }

    za::print("Recording... press enter to stop");
    za::scnStdinIgnoreLine();

    if (!recorder.stop())
        za::printErrLn("Failed to stop sound buffer recorder");

    // Get the buffer containing the captured data
    const za::SoundBuffer& buffer = recorder.getBuffer();

    // Display captured sound information
    za::printLn("Sound information:{} {} seconds{} {} samples / seconds{} {} channels",
                '\n',
                buffer.getDuration().asSeconds(),
                '\n',
                buffer.getSampleRate(),
                '\n',
                buffer.getChannelCount());

    // Choose what to do with the recorded sound data
    za::print("What do you want to do with captured sound (p = play, s = save) ? ");
    const char choice = za::scnStdin<char>().valueOr('p');
    za::scnStdinIgnoreLine();

    if (choice == 's')
    {
        // Choose the filename
        za::String filename;
        za::print("Choose the file to create: ");
        (void)za::scnStdinReadLine(filename);

        // Save the buffer
        if (!buffer.saveToFile(filename))
            za::printErrLn("Could not save sound buffer to file");
    }
    else
    {
        // Create the default playback device
        za::PlaybackDevice playbackDevice{za::AudioContext::getDefaultPlaybackDeviceHandle().value()};

        // Create a sound instance and play it
        za::Sound sound(playbackDevice, buffer);
        sound.play();

        // Wait until finished
        while (sound.isPlaying())
        {
            // Display the playing position
            za::print("\rPlaying... {} sec        ", sound.getPlayingOffset().asSeconds());
            za::print("");

            // Leave some CPU time for other threads
            za::ThisThread::sleepFor(za::milliseconds(100));
        }
    }

    // Finished!
    za::printLn("\nDone!");

    // Wait until the user presses 'enter' key
    za::printLn("Press enter to exit...");
    za::scnStdinIgnoreLine();
}
