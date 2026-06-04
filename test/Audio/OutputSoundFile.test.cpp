#include "AudioUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Audio/OutputSoundFile.hpp"

#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/SoundChannel.hpp"

#include "Zancle/System/Path.hpp"

#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsDefaultConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Audio] za::OutputSoundFile")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_DEFAULT_CONSTRUCTIBLE(za::OutputSoundFile));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::OutputSoundFile));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(za::OutputSoundFile));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::OutputSoundFile));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::OutputSoundFile));
    }

    const za::Path stems[]{U"tmp", U"tmp-ń", U"tmp-🐌"};
    const za::Path extensions[]{U".wav", U".ogg", U".flac"};

    for (const auto& stem : stems)
    {
        for (const auto& extension : extensions)
        {
            const auto           filename = za::Path::getTempDirectory().value() / (stem + extension);
            const za::ChannelMap channelMap{za::SoundChannel::FrontLeft, za::SoundChannel::FrontRight};

            SECTION("openFromFile()")
            {
                {
                    auto outputSoundFile = za::OutputSoundFile::openFromFile(filename,
                                                                             44'100,
                                                                             static_cast<unsigned int>(channelMap.getSize()),
                                                                             channelMap);
                    CHECK(outputSoundFile.hasValue());
                    CHECK(za::Path(filename).exists());
                }

                CHECK(za::Path(filename).removeFromDisk());
            }
        }
    }
}
