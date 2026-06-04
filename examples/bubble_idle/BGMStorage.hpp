#pragma once

#include "Zancle/Audio/Music.hpp"
#include "Zancle/Audio/MusicReader.hpp"
#include "ZancleBase/Array.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"


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
        musicReader{ZB_MOVE(theMusicSource)},
        music{playbackDevice, musicReader}
    {
    }
};


////////////////////////////////////////////////////////////
struct MainBGMStorage
{
    zb::Array<zb::Optional<BGMBuffer>, 2u> bgmBuffers{zb::nullOpt, zb::nullOpt};
};
