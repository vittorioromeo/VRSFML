////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Client.hpp"
#include "Server.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/CaptureDevice.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Scn/ScnStdin.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose a random port for opening sockets (ports < 1024 are reserved)
    const unsigned short port = 2435;

    // Client or server ?
    sf::base::print("Do you want to be a server ('s') or a client ('c')? ");
    const char who = sf::base::scnStdin<char>().valueOr('c');

    if (who == 's')
    {
        // Create an audio context and get the default playback device
        auto               audioContext = sf::AudioContext::create().value();
        sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

        // Run as a server
        doServer(playbackDevice, port);
    }
    else
    {
        // Create an audio context and get the default capture device
        auto              audioContext = sf::AudioContext::create().value();
        sf::CaptureDevice captureDevice{sf::AudioContext::getDefaultCaptureDeviceHandle().value()};

        // Run as a client
        doClient(captureDevice, port);
    }

    // Wait until the user presses 'enter' key
    sf::base::printLn("Press enter to exit...");
    sf::base::scnStdinIgnoreLine();
}
