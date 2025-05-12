////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Client.hpp"
#include "Server.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/CaptureDevice.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/System/IO.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose a random port for opening sockets (ports < 1024 are reserved)
    const unsigned short port = 2435;

    // Client or server ?
    char who = 0;
    sf::cOut() << "Do you want to be a server ('s') or a client ('c')? ";
    sf::cIn() >> who;

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
    sf::cOut() << "Press enter to exit..." << sf::endL;
    sf::cIn().ignore(10'000, '\n');
}
