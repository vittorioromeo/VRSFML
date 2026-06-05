#pragma once

#include "Zancle/Audio/Music.hpp"
#include "Zancle/Audio/MusicReader.hpp"

#include "Zancle/Container/Array.hpp"
#include "Zancle/Base/Macros.hpp"
#include "Zancle/Vocabulary/Optional.hpp"


namespace za
{
class PlaybackDevice;
}


////////////////////////////////////////////////////////////
struct BGMBuffer
{
    za::MusicReader musicReader;
    za::Music       music;

    explicit BGMBuffer(za::PlaybackDevice& playbackDevice, za::MusicReader&& theMusicSource) :
        musicReader{ZA_MOVE(theMusicSource)},
        music{playbackDevice, musicReader}
    {
    }
};


////////////////////////////////////////////////////////////
struct MainBGMStorage
{
    za::Array<za::Optional<BGMBuffer>, 2u> bgmBuffers{za::nullOpt, za::nullOpt};
};
