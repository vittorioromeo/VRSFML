#pragma once

#include "ShowcaseExample.hpp"

#include "Zancle/Audio/Music.hpp"
#include "Zancle/Audio/MusicReader.hpp"
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/Sound.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"

#include "Zancle/System/Path.hpp"

#include "ZancleBase/InPlaceVector.hpp"
#include "ZancleBase/Optional.hpp"


////////////////////////////////////////////////////////////
class ExampleAudio : public ShowcaseExample
{
private:
    ////////////////////////////////////////////////////////////
    bool m_useAlternativeMusicSource = false;

    ////////////////////////////////////////////////////////////
    za::SoundBuffer m_sbByteMeow  = za::SoundBuffer::loadFromFile("resources/bytemeow.ogg").value();
    za::MusicReader m_msBGMWizard = za::MusicReader::openFromFile("resources/bgmwizard.mp3").value();

    ////////////////////////////////////////////////////////////
    zb::InPlaceVector<za::PlaybackDevice, 8> m_playbackDevices;

    ////////////////////////////////////////////////////////////
    zb::InPlaceVector<za::Sound, 32> m_activeSounds;
    zb::Optional<za::Music>          m_activeMusic;

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
