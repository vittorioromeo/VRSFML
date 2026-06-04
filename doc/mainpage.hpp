////////////////////////////////////////////////////////////
/// \mainpage Zancle Documentation
///
/// \section welcome Welcome
/// Welcome to the official Zancle documentation. Here you will find a detailed
/// view of all the Zancle classes and functions. <br/>
/// If you are looking for tutorials, you can visit the official website
/// at <a href="https://www.zancle.org/">www.zancle.org</a>.
///
/// \section example Short example
/// Here is a short example, to show you how simple it is to use Zancle:
///
/// \code
///
/// #include <Zancle/Audio/Music.hpp>
///
/// #include <Zancle/Graphics/RenderWindow.hpp>
/// #include <Zancle/Graphics/Text.hpp>
/// #include <Zancle/Graphics/Texture.hpp>
/// #include <Zancle/Graphics/Sprite.hpp>
///
/// #include <Zancle/Window/Event.hpp>
/// #include <Zancle/Window/VideoMode.hpp>
///
/// #include <ZancleBase/Optional.hpp>
///
/// int main()
/// {
///     // Create the main window
///     auto window = za::RenderWindow::create(za::VideoMode({800, 600}), "Zancle window").value();
///
///     // Load a sprite to display
///     const auto texture = za::Texture::loadFromFile("cute_image.jpg").value();
///     za::Sprite sprite(texture.getRect());
///
///     // Create a graphical text to display
///     const auto font = za::Font::openFromFile("arial.ttf").value();
///     za::Text text(font, "Hello Zancle", 50);
///
///     // Load a music to play
///     auto music = za::Music::openFromFile("nice_music.ogg").value();
///
///     // Play the music
///     music.play();
///
///     // Start the game loop
///     while (true)
///     {
///         // Process events
///         while (const zb::Optional event = window.pollEvent())
///         {
///             // Close window: exit
///             if (event->is<za::Event::Closed>())
///                 return 0;
///         }
///
///         // Clear screen
///         window.clear();
///
///         // Draw the sprite
///         window.draw(sprite, texture);
///
///         // Draw the string
///         window.draw(text);
///
///         // Update the window
///         window.display();
///     }
/// }
/// \endcode
////////////////////////////////////////////////////////////
