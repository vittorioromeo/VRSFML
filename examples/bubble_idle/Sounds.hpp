#pragma once

#include "SFML/Audio/AudioSample.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
struct Sounds
{
    ////////////////////////////////////////////////////////////
    struct LoadedSound
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

    ////////////////////////////////////////////////////////////
    LoadedSound pop{"pop.ogg"};
    LoadedSound reversePop{"reversePop.ogg"};
    LoadedSound shine{"shine.ogg"};
    LoadedSound shine2{"shine2.ogg"};
    LoadedSound shine3{"shine3.ogg"};
    LoadedSound click{"click2.ogg"};
    LoadedSound byteMeow{"bytemeow.ogg"};
    LoadedSound grab{"grab.ogg"};
    LoadedSound drop{"drop.ogg"};
    LoadedSound scratch{"scratch.ogg"};
    LoadedSound buy{"buy.ogg"};
    LoadedSound explosion{"explosion.ogg"};
    LoadedSound makeBomb{"makebomb.ogg"};
    LoadedSound hex{"hex.ogg"};
    LoadedSound byteSpeak{"bytespeak.ogg"};
    LoadedSound prestige{"prestige.ogg"};
    LoadedSound launch{"launch.ogg"};
    LoadedSound rocket{"rocket.ogg"};
    LoadedSound earthquake{"earthquake.ogg"};
    LoadedSound earthquakeFast{"earthquakefast.ogg"};
    LoadedSound woosh{"woosh.ogg"};
    LoadedSound cast0{"cast0.ogg"};
    LoadedSound notification{"notification.ogg"};
    LoadedSound failpop{"failpop.ogg"};
    LoadedSound failpopui{"failpop.ogg"};
    LoadedSound buffoff{"buffoff.ogg"};
    LoadedSound buffon{"buffon.ogg"};
    LoadedSound kaching{"kaching.ogg"};
    LoadedSound coin{"coin.ogg"};
    LoadedSound btnswitch{"btnswitch.ogg"};
    LoadedSound uitab{"uitab.ogg"};
    LoadedSound coindelay{"coindelay.ogg"};
    LoadedSound absorb{"absorb.ogg"};
    LoadedSound maintenance{"maintenance.ogg"};
    LoadedSound ritual{"ritual.ogg"};
    LoadedSound ritualend{"ritualend.ogg"};
    LoadedSound copyritual{"copyritual.ogg"};
    LoadedSound copyritualend{"copyritualend.ogg"};
    LoadedSound purrmeow{"purrmeow.ogg"};
    LoadedSound soulsteal{"soulsteal.ogg"};
    LoadedSound soulreturn{"soulreturn.ogg"};
    LoadedSound portalon{"portalon.ogg"};
    LoadedSound portaloff{"portaloff.ogg"};
    LoadedSound failcast{"failcast.ogg"};
    LoadedSound unlock{"unlock.ogg"};
    LoadedSound purchasable{"purchasable.ogg"};
    LoadedSound shimmer{"shimmer.ogg"};
    LoadedSound smokebomb{"smokebomb.ogg"};
    LoadedSound quack{"quack.ogg"};
    LoadedSound paper{"paper.ogg"};
    LoadedSound letterchime{"letterchime.ogg"};

    ////////////////////////////////////////////////////////////
    static inline constexpr sf::base::SizeT maxSounds = 256u;

    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::AudioSample, maxSounds> soundsBeingPlayed;

    ////////////////////////////////////////////////////////////
    void setupSounds(const bool volumeOnly, const float volumeMult)
    {
        SFML_BASE_ASSERT(volumeMult >= 0.f && volumeMult <= 1.f);

        const auto setupWorldSound = [&](auto& sound, const float attenuationMult = 1.f)
        {
            if (!volumeOnly)
            {
                sound.settings.rollOff               = 0.003f * attenuationMult;
                sound.settings.spatializationEnabled = true;
            }

            sound.settings.volume = volumeMult;
        };

        const auto setupUISound = [&](auto& sound)
        {
            if (!volumeOnly)
            {
                sound.settings.rollOff               = 0.f;
                sound.settings.spatializationEnabled = false;
            }

            sound.settings.volume = volumeMult;
        };

        setupWorldSound(pop);
        setupWorldSound(reversePop);
        setupWorldSound(shine);
        setupWorldSound(shine2);
        setupWorldSound(shine3);
        setupWorldSound(explosion);
        setupWorldSound(makeBomb);
        setupWorldSound(hex);
        setupWorldSound(launch);
        setupWorldSound(rocket);
        setupWorldSound(earthquake, /* attenuationMult */ 0.1f);
        setupWorldSound(earthquakeFast, /* attenuationMult */ 0.1f);
        setupWorldSound(woosh, /* attenuationMult */ 0.1f);
        setupWorldSound(cast0, /* attenuationMult */ 0.1f);
        setupWorldSound(failpop);
        setupWorldSound(kaching);
        setupWorldSound(coindelay);
        setupWorldSound(absorb);
        setupWorldSound(maintenance);
        setupWorldSound(ritual, /* attenuationMult */ 0.1f);
        setupWorldSound(ritualend, /* attenuationMult */ 0.1f);
        setupWorldSound(copyritual, /* attenuationMult */ 0.1f);
        setupWorldSound(copyritualend, /* attenuationMult */ 0.1f);
        setupWorldSound(purrmeow);
        setupWorldSound(soulsteal);
        setupWorldSound(soulreturn);
        setupWorldSound(portalon);
        setupWorldSound(portaloff);
        setupWorldSound(failcast, /* attenuationMult */ 0.1f);
        setupWorldSound(smokebomb);
        setupWorldSound(quack);

        setupUISound(click);
        setupUISound(byteMeow);
        setupUISound(grab);
        setupUISound(drop);
        setupUISound(scratch);
        setupUISound(buy);
        setupUISound(byteSpeak);
        setupUISound(prestige);
        setupUISound(notification);
        setupUISound(buffoff);
        setupUISound(buffon);
        setupUISound(coin);
        setupUISound(btnswitch);
        setupUISound(uitab);
        setupUISound(failpopui);
        setupUISound(unlock);
        setupUISound(purchasable);
        setupUISound(shimmer);
        setupUISound(paper);
        setupUISound(letterchime);

        scratch.settings.volume   = volumeMult * 0.35f;
        buy.settings.volume       = volumeMult * 0.75f;
        explosion.settings.volume = volumeMult * 0.75f;
        coin.settings.volume      = volumeMult * 0.50f;
        buffon.settings.volume    = volumeMult * 0.65f;
        buffoff.settings.volume   = volumeMult * 0.65f;
        shine3.settings.volume    = volumeMult * 0.25f;
    }

    ////////////////////////////////////////////////////////////
    explicit Sounds(const float volumeMult)
    {
        setupSounds(/*volumeOnly */ false, volumeMult);
    }

    ////////////////////////////////////////////////////////////
    Sounds(const Sounds&) = delete;
    Sounds(Sounds&&)      = delete;

    ////////////////////////////////////////////////////////////
    void stopPlayingAll(const LoadedSound& ls)
    {
        for (sf::AudioSample& sound : soundsBeingPlayed)
        {
            if (sound.isPlaying() && &sound.getBuffer() == &ls.buffer)
                sound.stop();
        }
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT countPlayingPooled(const LoadedSound& ls) const
    {
        sf::base::SizeT acc = 0u;

        for (const sf::AudioSample& sound : soundsBeingPlayed)
        {
            if (sound.isPlaying() && &sound.getBuffer() == &ls.buffer)
                ++acc;
        }

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
            [](const sf::AudioSample& sound) { return !sound.isPlaying(); });

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
