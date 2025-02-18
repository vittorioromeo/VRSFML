#include "SFML/Audio/SoundFileFactory.hpp"

// Other 1st party headers
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/SoundChannel.hpp"
#include "SFML/Audio/SoundFileReader.hpp"
#include "SFML/Audio/SoundFileWriter.hpp"

#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <StringifyOptionalUtil.hpp>
#include <SystemUtil.hpp>

namespace
{

struct NoopSoundFileReader : sf::SoundFileReader
{
    static bool check(sf::InputStream&)
    {
        return false;
    }

    sf::base::Optional<Info> open(sf::InputStream&) override
    {
        return sf::base::nullOpt;
    }

    void seek(sf::base::U64) override
    {
    }

    sf::base::U64 read(sf::base::I16*, sf::base::U64) override
    {
        return 0;
    }
};

struct NoopSoundFileWriter : sf::SoundFileWriter
{
    static bool check(const sf::Path&)
    {
        return false;
    }

    bool open(const sf::Path&, unsigned int, unsigned int, const sf::ChannelMap&) override
    {
        return false;
    }

    void write(const sf::base::I16*, sf::base::U64) override
    {
    }
};

} // namespace

TEST_CASE("[Audio] sf::SoundFileFactory")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::SoundFileFactory));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::SoundFileFactory));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::SoundFileFactory));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::SoundFileFactory));
    }

    SECTION("isReaderRegistered()")
    {
        CHECK(!sf::SoundFileFactory::isReaderRegistered<NoopSoundFileReader>());

        sf::SoundFileFactory::registerReader<NoopSoundFileReader>();
        CHECK(sf::SoundFileFactory::isReaderRegistered<NoopSoundFileReader>());

        sf::SoundFileFactory::unregisterReader<NoopSoundFileReader>();
        CHECK(!sf::SoundFileFactory::isReaderRegistered<NoopSoundFileReader>());
    }

    SECTION("isWriterRegistered()")
    {
        CHECK(!sf::SoundFileFactory::isWriterRegistered<NoopSoundFileWriter>());

        sf::SoundFileFactory::registerWriter<NoopSoundFileWriter>();
        CHECK(sf::SoundFileFactory::isWriterRegistered<NoopSoundFileWriter>());

        sf::SoundFileFactory::unregisterWriter<NoopSoundFileWriter>();
        CHECK(!sf::SoundFileFactory::isWriterRegistered<NoopSoundFileWriter>());
    }

    SECTION("createReaderFromFilename()")
    {
        SECTION("Missing file")
        {
            CHECK(!sf::SoundFileFactory::createReaderFromFilename("does/not/exist.wav"));
        }

        SECTION("Valid file")
        {
            CHECK(sf::SoundFileFactory::createReaderFromFilename("Audio/ding.flac"));
            CHECK(sf::SoundFileFactory::createReaderFromFilename("Audio/ding.mp3"));
            CHECK(sf::SoundFileFactory::createReaderFromFilename("Audio/doodle_pop.ogg"));
            CHECK(sf::SoundFileFactory::createReaderFromFilename("Audio/killdeer.wav"));
        }
    }

    SECTION("createReaderFromStream()")
    {
        sf::base::Optional<sf::FileInputStream> stream;

        SECTION("flac")
        {
            stream = sf::FileInputStream::open("Audio/ding.flac");
        }

        SECTION("mp3")
        {
            stream = sf::FileInputStream::open("Audio/ding.mp3");
        }

        SECTION("ogg")
        {
            stream = sf::FileInputStream::open("Audio/doodle_pop.ogg");
        }

        SECTION("wav")
        {
            stream = sf::FileInputStream::open("Audio/killdeer.wav");
        }

        REQUIRE(stream.hasValue());
        CHECK(sf::SoundFileFactory::createReaderFromStream(*stream));
    }

    SECTION("createWriterFromFilename()")
    {
        SECTION("Invalid extension")
        {
            CHECK(!sf::SoundFileFactory::createWriterFromFilename("cannot/write/to.txt"));
        }

        SECTION("Valid extension")
        {
            CHECK(sf::SoundFileFactory::createWriterFromFilename("file.flac"));
            CHECK(!sf::SoundFileFactory::createWriterFromFilename("file.mp3")); // Mp3 writing not yet implemented
            CHECK(sf::SoundFileFactory::createWriterFromFilename("file.ogg"));
            CHECK(sf::SoundFileFactory::createWriterFromFilename("file.wav"));
        }
    }
}
