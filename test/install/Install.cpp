#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Audio/AudioContext.hpp"

#include "SFML/Network/Ftp.hpp"
#include "SFML/Network/Http.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/Window.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/String.hpp"

// Instantiate some types from each module to test for linker issues. This program is not meant be ran.
int main()
{
    // Audio
    [[maybe_unused]] const auto audioContext = sf::AudioContext::create();

    // Graphics
    [[maybe_unused]] auto             graphicsContext = sf::GraphicsContext::create().value();
    [[maybe_unused]] const sf::Color  color{};
    [[maybe_unused]] const sf::Rect2i rect{};
    // [[maybe_unused]] const sf::RenderWindow   renderWindow;
    [[maybe_unused]] const sf::RectangleShape rectangleShape{{}};
    [[maybe_unused]] const sf::Vertex         vertex{};

    // Network
    [[maybe_unused]] const sf::Ftp       ftp;
    [[maybe_unused]] const sf::Http      http;
    [[maybe_unused]] const sf::Packet    packet;
    [[maybe_unused]] const sf::UdpSocket udpSocket(/* isBlocking */ true);

    // System
    [[maybe_unused]] const sf::Angle  angle;
    [[maybe_unused]] const sf::String string;
    [[maybe_unused]] const sf::Time   time;

    // Window
    [[maybe_unused]] const sf::VideoMode videoMode{{0u, 0u}};
    [[maybe_unused]] const sf::Window    window({.size{0u, 0u}, .title = ""});
}
