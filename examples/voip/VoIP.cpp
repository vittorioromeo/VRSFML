////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Client.hpp"
#include "Server.hpp"

#include "Zancle/Audio/AudioContext.hpp"
#include "Zancle/Audio/CaptureDevice.hpp"
#include "Zancle/Audio/PlaybackDevice.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"
#include "Zancle/Scn/ScnStdin.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose a random port for opening sockets (ports < 1024 are reserved)
    const unsigned short port = 2435;

    // Client or server ?
    za::print("Do you want to be a server ('s') or a client ('c')? ");
    const char who = za::scnStdin<char>().valueOr('c');

    if (who == 's')
    {
        // Create an audio context and get the default playback device
        auto               audioContext = za::AudioContext::create().value();
        za::PlaybackDevice playbackDevice{za::AudioContext::getDefaultPlaybackDeviceHandle().value()};

        // Run as a server
        doServer(playbackDevice, port);
    }
    else
    {
        // Create an audio context and get the default capture device
        auto              audioContext = za::AudioContext::create().value();
        za::CaptureDevice captureDevice{za::AudioContext::getDefaultCaptureDeviceHandle().value()};

        // Run as a client
        doClient(captureDevice, port);
    }

    // Wait until the user presses 'enter' key
    za::printLn("Press enter to exit...");
    za::scnStdinIgnoreLine();
}
