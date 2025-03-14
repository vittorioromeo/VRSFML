#pragma once

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Algorithm.hpp"

#include <vector>


////////////////////////////////////////////////////////////
struct Sounds
{
    ////////////////////////////////////////////////////////////
    struct LoadedSound : private sf::SoundBuffer, public sf::Sound // TODO P2: eww
    {
        ////////////////////////////////////////////////////////////
        explicit LoadedSound(const sf::Path& filename) :
        sf::SoundBuffer(sf::SoundBuffer::loadFromFile("resources/" / filename).value()),
        sf::Sound(static_cast<const sf::SoundBuffer&>(*this))
        {
        }

        ////////////////////////////////////////////////////////////
        LoadedSound(const LoadedSound&) = delete;
        LoadedSound(LoadedSound&&)      = delete;

        ////////////////////////////////////////////////////////////
        const sf::Sound& asSound() const
        {
            return *this;
        }

        ////////////////////////////////////////////////////////////
        const sf::SoundBuffer& asBuffer() const
        {
            return *this;
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
    std::vector<sf::Sound> soundsBeingPlayed;

    ////////////////////////////////////////////////////////////
    explicit Sounds()
    {
        const auto setupWorldSound = [&](auto& sound, const float attenuationMult = 1.f)
        {
            sound.setAttenuation(0.003f * attenuationMult);
            sound.setSpatializationEnabled(true);
        };

        const auto setupUISound = [&](auto& sound)
        {
            sound.setAttenuation(0.f);
            sound.setSpatializationEnabled(false);
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

        scratch.setVolume(35.f);
        buy.setVolume(75.f);
        explosion.setVolume(75.f);
        coin.setVolume(50.f);
        buffon.setVolume(65.f);
        buffoff.setVolume(65.f);
        shine3.setVolume(25.f);
    }

    ////////////////////////////////////////////////////////////
    Sounds(const Sounds&) = delete;
    Sounds(Sounds&&)      = delete;

    ////////////////////////////////////////////////////////////
    void stopPlayingAll(const LoadedSound& ls)
    {
        for (sf::Sound& sound : soundsBeingPlayed)
            if (sound.getStatus() == sf::Sound::Status::Playing && &sound.getBuffer() == &ls.asBuffer())
                sound.stop();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT countPlayingPooled(const LoadedSound& ls) const
    {
        sf::base::SizeT acc = 0u;

        for (const sf::Sound& sound : soundsBeingPlayed)
            if (sound.getStatus() == sf::Sound::Status::Playing && &sound.getBuffer() == &ls.asBuffer())
                ++acc;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    bool playPooled(sf::PlaybackDevice& playbackDevice, const LoadedSound& ls, const sf::base::SizeT maxOverlap)
    {
        // TODO P2 (lib): improve in library

        if (countPlayingPooled(ls) >= maxOverlap)
            return false;

        const auto it = sf::base::findIf( //
            soundsBeingPlayed.begin(),
            soundsBeingPlayed.end(),
            [](const sf::Sound& sound) { return sound.getStatus() == sf::Sound::Status::Stopped; });

        if (it != soundsBeingPlayed.end())
        {
            *it = ls.asSound(); // assigment does not reallocate `m_impl`
            it->play(playbackDevice);

            return true;
        }

        // TODO P2 (lib): to sf base, also not needed
        // std::erase_if(soundsBeingPlayed,
        //               [](const sf::Sound& sound) { return sound.getStatus() == sf::Sound::Status::Stopped; });

        soundsBeingPlayed.emplace_back(ls.asSound()).play(playbackDevice);
        return true;
    }
};
