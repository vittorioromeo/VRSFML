#include "SFML/Audio/OutputSoundFile.hpp"

#include "SFML/Audio/ChannelMap.hpp"

#include "SFML/System/Path.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>

#include <string>


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

    const std::u32string stems[]{U"tmp", U"tmp-ń", U"tmp-🐌"};
    const std::u32string extensions[]{U".wav", U".ogg", U".flac"};

    for (const auto& stem : stems)
    {
        for (const auto& extension : extensions)
        {
            const auto           filename = sf::Path::tempDirectoryPath() / sf::Path(stem + extension);
            const sf::ChannelMap channelMap{sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight};

            INFO("Filename: " << std::string(reinterpret_cast<const char*>(filename.to<std::u8string>().c_str())));

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

                CHECK(sf::Path(filename).remove());
            }
        }
    }
}
