#include "AudioUtil.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Audio/OutputSoundFile.hpp"

#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/SoundChannel.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Audio] sf::OutputSoundFile")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::OutputSoundFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::OutputSoundFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::OutputSoundFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::OutputSoundFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::OutputSoundFile));
    }

    const sf::Path stems[]{U"tmp", U"tmp-ń", U"tmp-🐌"};
    const sf::Path extensions[]{U".wav", U".ogg", U".flac"};

    for (const auto& stem : stems)
    {
        for (const auto& extension : extensions)
        {
            const auto           filename = sf::Path::getTempDirectory().value() / (stem + extension);
            const sf::ChannelMap channelMap{sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight};

            SECTION("openFromFile()")
            {
                {
                    auto outputSoundFile = sf::OutputSoundFile::openFromFile(filename,
                                                                             44'100,
                                                                             static_cast<unsigned int>(channelMap.getSize()),
                                                                             channelMap);
                    CHECK(outputSoundFile.hasValue());
                    CHECK(sf::Path(filename).exists());
                }

                CHECK(sf::Path(filename).removeFromDisk());
            }
        }
    }
}
