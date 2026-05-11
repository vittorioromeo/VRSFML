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
#include "SFML/Window/WindowSettings.hpp" // IWYU pragma: keep

#include "SFML/System/Angle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Utf8String.hpp"

#include "SFML/Base/Optional.hpp"


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
    [[maybe_unused]] const sf::Ftp                           ftp;
    [[maybe_unused]] const sf::Http                          http;
    [[maybe_unused]] const sf::Packet                        packet;
    [[maybe_unused]] const sf::base::Optional<sf::UdpSocket> udpSocket = sf::UdpSocket::create(/* isBlocking */ true);

    // System
    [[maybe_unused]] const sf::Angle      angle;
    [[maybe_unused]] const sf::Utf8String string;
    [[maybe_unused]] const sf::Time       time;

    // Window
    [[maybe_unused]] const sf::VideoMode videoMode{.size = {0u, 0u}};
    [[maybe_unused]] const auto          window = sf::Window::create({.size{0u, 0u}, .title = ""});
}
