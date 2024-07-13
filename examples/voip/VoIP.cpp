////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/CaptureDevice.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

#include <iostream>


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
    std::cout << "Do you want to be a server ('s') or a client ('c')? ";
    std::cin >> who;

    if (who == 's')
    {
        // Create an audio context and get the default playback device
        auto audioContext   = sf::AudioContext::create().value();
        auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

        // Run as a server
        void doServer(sf::PlaybackDevice&, unsigned short);
        doServer(playbackDevice, port);
    }
    else
    {
        // Create an audio context and get the default capture device
        auto audioContext  = sf::AudioContext::create().value();
        auto captureDevice = sf::CaptureDevice::createDefault(audioContext).value();

        // Run as a client
        void doClient(sf::CaptureDevice&, unsigned short);
        doClient(captureDevice, port);
    }

    // Wait until the user presses 'enter' key
    std::cout << "Press enter to exit..." << std::endl;
    std::cin.ignore(10000, '\n');
}
