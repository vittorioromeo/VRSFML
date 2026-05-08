#include "Sounds.hpp"

#include "SFML/Base/Assert.hpp"


////////////////////////////////////////////////////////////
void Sounds::setupSounds(const bool volumeOnly, const float volumeMult)
{
    SFML_BASE_ASSERT(volumeMult >= 0.f && volumeMult <= 1.f);

    const auto setupWorldSound = [&](auto& sound, const float attenuationMult = 1.f)
    {
        if (!volumeOnly)
        {
            sound.settings.attenuation           = 0.003f * attenuationMult;
            sound.settings.spatializationEnabled = true;
        }

        sound.settings.volume = volumeMult;
    };

    const auto setupUISound = [&](auto& sound)
    {
        if (!volumeOnly)
        {
            sound.settings.attenuation           = 0.f;
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
    setupWorldSound(sleep, /* attenuationMult */ 0.6f);
    setupWorldSound(bonk);
    setupWorldSound(glasshit);
    setupWorldSound(glassbreak);
    setupWorldSound(napWake);

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
    sleep.settings.volume     = volumeMult * 0.75f;
}


////////////////////////////////////////////////////////////
Sounds::Sounds(const float volumeMult)
{
    setupSounds(/*volumeOnly */ false, volumeMult);
}
