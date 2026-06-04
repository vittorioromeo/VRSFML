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
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Thread.hpp"
#include "Zancle/System/Time.hpp"
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"
#include "ZancleBase/Scn/ScnString.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"


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
        zb::printErrLn("Sorry, audio capture is not supported by your system");
        return 1;
    }

    // List the available capture device handles
    zb::printLn("Available capture devices:\n");

    for (zb::SizeT i = 0u; i < deviceHandles.size(); ++i)
        zb::printLn("{}: {}", i, deviceHandles[i].getName());

    zb::printLn("");

    // Choose the capture device
    zb::SizeT deviceIndex = 0;

    if (deviceHandles.size() > 1)
    {
        deviceIndex = deviceHandles.size();
        zb::print("Please choose the capture device to use [0-{}]: ", deviceHandles.size() - 1);

        do
        {
            (void)zb::scnStdinInto(deviceIndex);
            zb::scnStdinIgnoreLine();
        } while (deviceIndex >= deviceHandles.size());
    }

    // Choose the sample rate
    unsigned int sampleRate = 0;
    zb::print("Please choose the sample rate for sound capture (44100 is CD quality): ");
    (void)zb::scnStdinInto(sampleRate);
    zb::scnStdinIgnoreLine();

    // Wait for user input...
    zb::print("Press enter to start recording audio");
    zb::scnStdinIgnoreLine();

    // Create the capture device
    za::CaptureDevice captureDevice(deviceHandles[deviceIndex]);

    // Here we'll use an integrated custom recorder, which saves the captured data into a sound buffer
    za::SoundBufferRecorder recorder;

    // Audio capture is done in a separate thread, so we can block the main thread while it is capturing
    if (!recorder.start(captureDevice, sampleRate))
    {
        zb::printErrLn("Failed to start recorder");
        return 1;
    }

    zb::print("Recording... press enter to stop");
    zb::scnStdinIgnoreLine();

    if (!recorder.stop())
        zb::printErrLn("Failed to stop sound buffer recorder");

    // Get the buffer containing the captured data
    const za::SoundBuffer& buffer = recorder.getBuffer();

    // Display captured sound information
    zb::printLn("Sound information:{} {} seconds{} {} samples / seconds{} {} channels",
                '\n',
                buffer.getDuration().asSeconds(),
                '\n',
                buffer.getSampleRate(),
                '\n',
                buffer.getChannelCount());

    // Choose what to do with the recorded sound data
    zb::print("What do you want to do with captured sound (p = play, s = save) ? ");
    const char choice = zb::scnStdin<char>().valueOr('p');
    zb::scnStdinIgnoreLine();

    if (choice == 's')
    {
        // Choose the filename
        zb::String filename;
        zb::print("Choose the file to create: ");
        (void)zb::scnStdinReadLine(filename);

        // Save the buffer
        if (!buffer.saveToFile(filename))
            zb::printErrLn("Could not save sound buffer to file");
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
            zb::print("\rPlaying... {} sec        ", sound.getPlayingOffset().asSeconds());
            zb::print("");

            // Leave some CPU time for other threads
            za::ThisThread::sleepFor(za::milliseconds(100));
        }
    }

    // Finished!
    zb::printLn("\nDone!");

    // Wait until the user presses 'enter' key
    zb::printLn("Press enter to exit...");
    zb::scnStdinIgnoreLine();
}
