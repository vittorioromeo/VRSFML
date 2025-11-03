#pragma once

#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] LoadedSound
{
    ////////////////////////////////////////////////////////////
    sf::SoundBuffer   buffer;
    sf::AudioSettings settings;

    ////////////////////////////////////////////////////////////
    explicit LoadedSound(const sf::Path& filename) :
        buffer(sf::SoundBuffer::loadFromFile("resources/" / filename).value())
    {
    }
};
