#include "Zancle/Audio/AudioContext.hpp"

#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/RectangleShape.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/Network/Ftp.hpp"
#include "Zancle/Network/Http.hpp"
#include "Zancle/Network/Packet.hpp"
#include "Zancle/Network/UdpSocket.hpp"

#include "Zancle/Window/VideoMode.hpp"
#include "Zancle/Window/Window.hpp"
#include "Zancle/Window/WindowContext.hpp"
#include "Zancle/Window/WindowSettings.hpp" // IWYU pragma: keep

#include "Zancle/String/Utf8String.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Geometry/Angle.hpp"
#include "Zancle/Geometry/Rect2.hpp"

#include "Zancle/Vocabulary/Optional.hpp"


// Instantiate some types from each module to test for linker issues. This program is not meant be ran.
int main()
{
    // Audio
    [[maybe_unused]] const auto audioContext = za::AudioContext::create();

    // Graphics
    [[maybe_unused]] auto             graphicsContext = za::GraphicsContext::create().value();
    [[maybe_unused]] const za::Color  color{};
    [[maybe_unused]] const za::Rect2i rect{};
    // [[maybe_unused]] const za::RenderWindow   renderWindow;
    [[maybe_unused]] const za::RectangleShape rectangleShape{{}};
    [[maybe_unused]] const za::Vertex         vertex{};

    // Network
    [[maybe_unused]] const za::Ftp                     ftp;
    [[maybe_unused]] const za::Http                    http;
    [[maybe_unused]] const za::Packet                  packet;
    [[maybe_unused]] const za::Optional<za::UdpSocket> udpSocket = za::UdpSocket::create(/* isBlocking */ true);

    // System
    [[maybe_unused]] const za::Angle      angle;
    [[maybe_unused]] const za::Utf8String string;
    [[maybe_unused]] const za::Time       time;

    // Window
    [[maybe_unused]] const za::VideoMode videoMode{.size = {0u, 0u}};
    [[maybe_unused]] const auto          window = za::Window::create({.size{0u, 0u}, .title = ""});
}
