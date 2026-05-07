#pragma once

#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
class PlaybackDevice;
}


////////////////////////////////////////////////////////////
struct BGMBuffer
{
    sf::MusicReader musicReader;
    sf::Music       music;

    explicit BGMBuffer(sf::PlaybackDevice& playbackDevice, sf::MusicReader&& theMusicSource) :
        musicReader{SFML_BASE_MOVE(theMusicSource)},
        music{playbackDevice, musicReader}
    {
    }
};


////////////////////////////////////////////////////////////
struct MainBGMStorage
{
    sf::base::Array<sf::base::Optional<BGMBuffer>, 2u> bgmBuffers{sf::base::nullOpt, sf::base::nullOpt};
};
