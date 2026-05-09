#include "ExampleUtils/LoadedSound.hpp"

#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Path.hpp"


////////////////////////////////////////////////////////////
LoadedSound::LoadedSound(const sf::Path& filename) :
    buffer(sf::SoundBuffer::loadFromFile(sf::Path{"resources"} / filename).value())
{
}


////////////////////////////////////////////////////////////
LoadedSound::LoadedSound(const char* const filename) : LoadedSound(sf::Path{filename})
{
}
