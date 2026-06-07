#include "AudioUtil.hpp"
#include "TemporaryFile.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Audio/OutputSoundFile.hpp"

#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/SoundChannel.hpp"

#include "Zancle/Fmt/FmtToString.hpp"

#include "Zancle/IO/Path.hpp"

#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsDefaultConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Audio] za::OutputSoundFile")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZA_IS_DEFAULT_CONSTRUCTIBLE(za::OutputSoundFile));
        STATIC_CHECK(!ZA_IS_COPY_CONSTRUCTIBLE(za::OutputSoundFile));
        STATIC_CHECK(!ZA_IS_COPY_ASSIGNABLE(za::OutputSoundFile));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::OutputSoundFile));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::OutputSoundFile));
    }

    // Disambiguate temp-file names across parallel test processes
    const auto     pidSuffix = za::Path(za::fmtToString("-{}", za::testing::getProcessUniqueId()));
    const za::Path stems[]{U"tmp" + pidSuffix, U"tmp-ń" + pidSuffix, U"tmp-🐌" + pidSuffix};
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
