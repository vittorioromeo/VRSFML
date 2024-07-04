#include <SFML/Audio/Music.hpp>

#include <iostream>
#include <optional>

template <typename T>
class Singleton
{
public:
    static T& getInstance()
    {
        std::cout << "Singleton<T>::getInstance()\n";

        static T instance;
        return instance;
    }
};

struct MusicSingleton : Singleton<MusicSingleton>
{
    MusicSingleton()
    {
        std::cout << "MusicSingleton::MusicSingleton()\n";
    }

    std::optional<sf::Music> music;
};

int main()
{
    std::cout << "-- start of main --\n\n";

    MusicSingleton::getInstance();

    std::cout << "\n-- after first getInstance --\n\n";

    MusicSingleton::getInstance().music = sf::Music::openFromFile("resources/ding.flac").value();

    std::cout << "\n-- end of main-- \n\n";
}
