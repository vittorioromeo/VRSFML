////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>

#include <stdexcept>

////////////////////////////////////////////////////////////
/// Helper function
////////////////////////////////////////////////////////////
sf::Text makeText()
{
    sf::Font font;
    if (!font.loadFromFile("resources/tuffy.ttf"))
        throw std::runtime_error("Could not find font :(");

    return sf::Text(font, "Definitely not a bug!");
}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    sf::Text text = makeText();
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "Boom");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(text);
        window.display();
    }

    return EXIT_SUCCESS;
}
