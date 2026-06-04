#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/Sound.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"
#include "ZancleBase/Algorithm/Find.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/InPlaceVector.hpp"
#include "ZancleBase/SizeT.hpp"

#include "ExampleUtils/LoadedSound.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] SoundManager
{
    ////////////////////////////////////////////////////////////
    enum : zb::SizeT
    {
        maxSounds = 256u
    };


    ////////////////////////////////////////////////////////////
    zb::InPlaceVector<za::Sound, maxSounds> soundsBeingPlayed;


    ////////////////////////////////////////////////////////////
    explicit SoundManager() = default;


    ////////////////////////////////////////////////////////////
    SoundManager(const SoundManager&) = delete;
    SoundManager(SoundManager&&)      = delete;


    ////////////////////////////////////////////////////////////
    void stopPlayingAll(const LoadedSound& ls)
    {
        for (za::Sound& sound : soundsBeingPlayed)
            if (sound.isPlaying() && &sound.getBuffer() == &ls.buffer)
                sound.stop();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::SizeT countPlayingPooled(const LoadedSound& ls) const
    {
        zb::SizeT acc = 0u;

        for (const za::Sound& sound : soundsBeingPlayed)
            if (sound.isPlaying() && &sound.getBuffer() == &ls.buffer)
                ++acc;

        return acc;
    }


    ////////////////////////////////////////////////////////////
    bool playPooled(za::PlaybackDevice& playbackDevice, const LoadedSound& ls, const zb::SizeT maxOverlap)
    {
        // TODO P2 (lib): improve in library

        if (countPlayingPooled(ls) >= maxOverlap)
            return false;

        auto* const it = zb::findIf( //
            soundsBeingPlayed.begin(),
            soundsBeingPlayed.end(),
            [](const za::Sound& sound) { return !sound.isPlaying(); });

        if (it != soundsBeingPlayed.end())
        {
            ZB_ASSERT(&it->getPlaybackDevice() == &playbackDevice);

            if (&it->getBuffer() == &ls.buffer)
            {
                it->applyAudioSettings(ls.settings);
                it->play();
                return true;
            }

            soundsBeingPlayed.reEmplaceByIterator(it, playbackDevice, ls.buffer, ls.settings).play();
            return true;
        }

        if (soundsBeingPlayed.size() >= maxSounds)
            return false;

        soundsBeingPlayed.emplaceBack(playbackDevice, ls.buffer, ls.settings).play();
        return true;
    }
};
