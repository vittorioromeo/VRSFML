#pragma once

#include "LoadedSound.hpp"

#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/InPlaceVector.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] SoundManager
{
    ////////////////////////////////////////////////////////////
    static inline constexpr sf::base::SizeT maxSounds = 256u;

    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::Sound, maxSounds> soundsBeingPlayed;

    ////////////////////////////////////////////////////////////
    explicit SoundManager() = default;

    ////////////////////////////////////////////////////////////
    SoundManager(const SoundManager&) = delete;
    SoundManager(SoundManager&&)      = delete;

    ////////////////////////////////////////////////////////////
    void stopPlayingAll(const LoadedSound& ls)
    {
        for (sf::Sound& sound : soundsBeingPlayed)
            if (sound.isPlaying() && &sound.getBuffer() == &ls.buffer)
                sound.stop();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] sf::base::SizeT countPlayingPooled(const LoadedSound& ls) const
    {
        sf::base::SizeT acc = 0u;

        for (const sf::Sound& sound : soundsBeingPlayed)
            if (sound.isPlaying() && &sound.getBuffer() == &ls.buffer)
                ++acc;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    bool playPooled(sf::PlaybackDevice& playbackDevice, const LoadedSound& ls, const sf::base::SizeT maxOverlap)
    {
        // TODO P2 (lib): improve in library

        if (countPlayingPooled(ls) >= maxOverlap)
            return false;

        auto* const it = sf::base::findIf( //
            soundsBeingPlayed.begin(),
            soundsBeingPlayed.end(),
            [](const sf::Sound& sound) { return !sound.isPlaying(); });

        if (it != soundsBeingPlayed.end())
        {
            SFML_BASE_ASSERT(&it->getPlaybackDevice() == &playbackDevice);

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
