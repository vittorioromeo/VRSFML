#pragma once

#include "ExampleUtils/LoadedSound.hpp"


////////////////////////////////////////////////////////////
struct Sounds
{
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
    LoadedSound sleep{"sleep.ogg"};
    LoadedSound glasshit{"glasshit.ogg"};
    LoadedSound glassbreak{"glassbreak.ogg"};
    LoadedSound bonk{"bonk.ogg"};
    LoadedSound napWake{"mew.ogg"};

    ////////////////////////////////////////////////////////////
    void setupSounds(bool volumeOnly, float volumeMult);

    ////////////////////////////////////////////////////////////
    explicit Sounds(float volumeMult);
};
