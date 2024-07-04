////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/CaptureDevice.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

#include <iostream>


////////////////////////////////////////////////////////////
// Function prototypes
// (I'm too lazy to put them into separate headers...)
////////////////////////////////////////////////////////////
void doClient(sf::CaptureDevice& captureDevice, unsigned short port);
void doServer(sf::PlaybackDevice& playbackDevice, unsigned short port);


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create an audio context and get the default playback and capture devices
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();
    auto captureDevice  = sf::CaptureDevice::createDefault(audioContext).value();

    // Choose a random port for opening sockets (ports < 1024 are reserved)
    const unsigned short port = 2435;

    // Client or server ?
    char who = 0;
    std::cout << "Do you want to be a server ('s') or a client ('c')? ";
    std::cin >> who;

    if (who == 's')
    {
        // Run as a server
        doServer(playbackDevice, port);
    }
    else
    {
        // Run as a client
        doClient(captureDevice, port);
    }

    // Wait until the user presses 'enter' key
    std::cout << "Press enter to exit..." << std::endl;
    std::cin.ignore(10000, '\n');
}
