#include "Zancle/Audio/SoundFileFactory.hpp"

// Other 1st party headers
#include "StringifyOptionalUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/SoundFileReader.hpp"
#include "Zancle/Audio/SoundFileWriter.hpp"
#include "Zancle/System/FileInputStream.hpp"
#include "Zancle/System/InputStream.hpp"
#include "Zancle/System/Path.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"


namespace
{

struct NoopSoundFileReader : za::SoundFileReader
{
    static bool check(za::InputStream&)
    {
        return false;
    }

    zb::Optional<Info> open(za::InputStream&) override
    {
        return zb::nullOpt;
    }

    void seek(zb::U64) override
    {
    }

    zb::U64 read(zb::I16*, zb::U64) override
    {
        return 0;
    }
};

struct NoopSoundFileWriter : za::SoundFileWriter
{
    static bool check(const za::Path&)
    {
        return false;
    }

    bool open(const za::Path&, unsigned int, unsigned int, const za::ChannelMap&) override
    {
        return false;
    }

    void write(const zb::I16*, zb::U64) override
    {
    }
};

} // namespace

TEST_CASE("[Audio] za::SoundFileFactory")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::SoundFileFactory));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(za::SoundFileFactory));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::SoundFileFactory));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::SoundFileFactory));
    }

    SECTION("isReaderRegistered()")
    {
        CHECK(!za::SoundFileFactory::isReaderRegistered<NoopSoundFileReader>());

        za::SoundFileFactory::registerReader<NoopSoundFileReader>();
        CHECK(za::SoundFileFactory::isReaderRegistered<NoopSoundFileReader>());

        za::SoundFileFactory::unregisterReader<NoopSoundFileReader>();
        CHECK(!za::SoundFileFactory::isReaderRegistered<NoopSoundFileReader>());
    }

    SECTION("isWriterRegistered()")
    {
        CHECK(!za::SoundFileFactory::isWriterRegistered<NoopSoundFileWriter>());

        za::SoundFileFactory::registerWriter<NoopSoundFileWriter>();
        CHECK(za::SoundFileFactory::isWriterRegistered<NoopSoundFileWriter>());

        za::SoundFileFactory::unregisterWriter<NoopSoundFileWriter>();
        CHECK(!za::SoundFileFactory::isWriterRegistered<NoopSoundFileWriter>());
    }

    SECTION("createReaderFromFilename()")
    {
        SECTION("Missing file")
        {
            CHECK((za::SoundFileFactory::createReaderFromFilename("does/not/exist.wav") == nullptr));
        }

        SECTION("Valid file")
        {
            CHECK((za::SoundFileFactory::createReaderFromFilename("ding.flac") != nullptr));
            CHECK((za::SoundFileFactory::createReaderFromFilename("ding.mp3") != nullptr));
            CHECK((za::SoundFileFactory::createReaderFromFilename("doodle_pop.ogg") != nullptr));
            CHECK((za::SoundFileFactory::createReaderFromFilename("killdeer.wav") != nullptr));
        }
    }

    SECTION("createReaderFromStream()")
    {
        zb::Optional<za::FileInputStream> stream;

        SECTION("flac")
        {
            stream = za::FileInputStream::open("ding.flac");
        }

        SECTION("mp3")
        {
            stream = za::FileInputStream::open("ding.mp3");
        }

        SECTION("ogg")
        {
            stream = za::FileInputStream::open("doodle_pop.ogg");
        }

        SECTION("wav")
        {
            stream = za::FileInputStream::open("killdeer.wav");
        }

        REQUIRE(stream.hasValue());
        CHECK((za::SoundFileFactory::createReaderFromStream(*stream) != nullptr));
    }

    SECTION("createWriterFromFilename()")
    {
        SECTION("Invalid extension")
        {
            CHECK((za::SoundFileFactory::createWriterFromFilename("cannot/write/to.txt") == nullptr));
        }

        SECTION("Valid extension")
        {
            CHECK((za::SoundFileFactory::createWriterFromFilename("file.flac") != nullptr));
            CHECK((za::SoundFileFactory::createWriterFromFilename("file.mp3") == nullptr)); // Mp3 writing not yet implemented
            CHECK((za::SoundFileFactory::createWriterFromFilename("file.ogg") != nullptr));
            CHECK((za::SoundFileFactory::createWriterFromFilename("file.wav") != nullptr));
        }
    }
}
