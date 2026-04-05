#pragma once

#include "ShowcaseExample.hpp"

#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
class ExampleAudio : public ShowcaseExample
{
private:
    ////////////////////////////////////////////////////////////
    bool m_useAlternativeMusicSource = false;

    ////////////////////////////////////////////////////////////
    sf::SoundBuffer m_sbByteMeow  = sf::SoundBuffer::loadFromFile("resources/bytemeow.ogg").value();
    sf::MusicReader m_msBGMWizard = sf::MusicReader::openFromFile("resources/bgmwizard.mp3").value();

    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::PlaybackDevice, 8> m_playbackDevices;

    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::Sound, 32> m_activeSounds;
    sf::base::Optional<sf::Music>          m_activeMusic;

    ////////////////////////////////////////////////////////////
    void refreshPlaybackDevices();

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleAudio();

    ////////////////////////////////////////////////////////////
    void update(float deltaTimeMs) override;

    ////////////////////////////////////////////////////////////
    void imgui() override;

    ////////////////////////////////////////////////////////////
    void draw() override;
};
