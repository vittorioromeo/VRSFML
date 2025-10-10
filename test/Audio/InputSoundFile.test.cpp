#include "SFML/Audio/InputSoundFile.hpp"

// Other 1st party headers
#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Array.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <StringifyArrayUtil.hpp>
#include <SystemUtil.hpp>

#include <string>


TEST_CASE("[Audio] sf::InputSoundFile")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::InputSoundFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::InputSoundFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::InputSoundFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::InputSoundFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::InputSoundFile));
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid file")
        {
            CHECK(!sf::InputSoundFile::openFromFile("does/not/exist.wav").hasValue());
        }

        SECTION("Valid file")
        {
            const std::u32string filenameSuffixes[] = {U"", U"-ń", U"-🐌"};
            for (const auto& filenameSuffix : filenameSuffixes)
            {

                SECTION("flac")
                {
                    const sf::Path filename = U"ding" + filenameSuffix + U".flac";
                    INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                    const auto inputSoundFile = sf::InputSoundFile::openFromFile(filename).value();
                    CHECK(inputSoundFile.getSampleCount() == 87'798);
                    CHECK(inputSoundFile.getChannelCount() == 1);
                    CHECK(inputSoundFile.getSampleRate() == 44'100);
                    CHECK(inputSoundFile.getDuration() == sf::microseconds(1'990'884));
                    CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
                }

                SECTION("mp3")
                {
                    const sf::Path filename = U"ding" + filenameSuffix + U".mp3";
                    INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                    const auto inputSoundFile = sf::InputSoundFile::openFromFile(filename).value();
                    CHECK(inputSoundFile.getSampleCount() == 87'798);
                    CHECK(inputSoundFile.getChannelCount() == 1);
                    CHECK(inputSoundFile.getSampleRate() == 44'100);
                    CHECK(inputSoundFile.getDuration() == sf::microseconds(1'990'884));
                    CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
                }

                SECTION("ogg")
                {
                    const sf::Path filename = U"doodle_pop" + filenameSuffix + U".ogg";
                    INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                    const auto inputSoundFile = sf::InputSoundFile::openFromFile(filename).value();
                    CHECK(inputSoundFile.getSampleCount() == 2'116'992);
                    CHECK(inputSoundFile.getChannelCount() == 2);
                    CHECK(inputSoundFile.getSampleRate() == 44'100);
                    CHECK(inputSoundFile.getDuration() == sf::microseconds(24'002'176));
                    CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
                }

                SECTION("wav")
                {
                    const sf::Path filename = U"killdeer" + filenameSuffix + U".wav";
                    INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                    const auto inputSoundFile = sf::InputSoundFile::openFromFile(filename).value();
                    CHECK(inputSoundFile.getSampleCount() == 112'941);
                    CHECK(inputSoundFile.getChannelCount() == 1);
                    CHECK(inputSoundFile.getSampleRate() == 22'050);
                    CHECK(inputSoundFile.getDuration() == sf::microseconds(5'122'040));
                    CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
                }
            }
        }
    }

    SECTION("openFromMemory()")
    {
        const auto memory         = loadIntoMemory("killdeer.wav");
        const auto inputSoundFile = sf::InputSoundFile::openFromMemory(memory.data(), memory.size()).value();
        CHECK(inputSoundFile.getSampleCount() == 112'941);
        CHECK(inputSoundFile.getChannelCount() == 1);
        CHECK(inputSoundFile.getSampleRate() == 22'050);
        CHECK(inputSoundFile.getDuration() == sf::microseconds(5'122'040));
        CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
    }

    SECTION("openFromStream()")
    {
        SECTION("flac")
        {
            auto       stream         = sf::FileInputStream::open("ding.flac").value();
            const auto inputSoundFile = sf::InputSoundFile::openFromStream(stream).value();
            CHECK(inputSoundFile.getSampleCount() == 87'798);
            CHECK(inputSoundFile.getChannelCount() == 1);
            CHECK(inputSoundFile.getSampleRate() == 44'100);
            CHECK(inputSoundFile.getDuration() == sf::microseconds(1'990'884));
            CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
        }

        SECTION("mp3")
        {
            auto       stream         = sf::FileInputStream::open("ding.mp3").value();
            const auto inputSoundFile = sf::InputSoundFile::openFromStream(stream).value();
            CHECK(inputSoundFile.getSampleCount() == 87'798);
            CHECK(inputSoundFile.getChannelCount() == 1);
            CHECK(inputSoundFile.getSampleRate() == 44'100);
            CHECK(inputSoundFile.getDuration() == sf::microseconds(1'990'884));
            CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
        }

        SECTION("ogg")
        {
            auto       stream         = sf::FileInputStream::open("doodle_pop.ogg").value();
            const auto inputSoundFile = sf::InputSoundFile::openFromStream(stream).value();
            CHECK(inputSoundFile.getSampleCount() == 2'116'992);
            CHECK(inputSoundFile.getChannelCount() == 2);
            CHECK(inputSoundFile.getSampleRate() == 44'100);
            CHECK(inputSoundFile.getDuration() == sf::microseconds(24'002'176));
            CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
        }

        SECTION("wav")
        {
            auto       stream         = sf::FileInputStream::open("killdeer.wav").value();
            const auto inputSoundFile = sf::InputSoundFile::openFromStream(stream).value();
            CHECK(inputSoundFile.getSampleCount() == 112'941);
            CHECK(inputSoundFile.getChannelCount() == 1);
            CHECK(inputSoundFile.getSampleRate() == 22'050);
            CHECK(inputSoundFile.getDuration() == sf::microseconds(5'122'040));
            CHECK(inputSoundFile.getTimeOffset(/* sampleOffset */ 0u) == sf::Time{});
        }
    }

    SECTION("seek(sf::base::U64)")
    {
        SECTION("flac")
        {
            auto       inputSoundFile = sf::InputSoundFile::openFromFile("ding.flac").value();
            const auto sampleOffset   = inputSoundFile.seek(1000);
            CHECK(inputSoundFile.getTimeOffset(sampleOffset) == sf::microseconds(22'675));
            CHECK(sampleOffset == 1000);
        }

        SECTION("mp3")
        {
            auto       inputSoundFile = sf::InputSoundFile::openFromFile("ding.mp3").value();
            const auto sampleOffset   = inputSoundFile.seek(1000);
            CHECK(inputSoundFile.getTimeOffset(sampleOffset) == sf::microseconds(22'675));
            CHECK(sampleOffset == 1000);
        }

        SECTION("ogg")
        {
            auto       inputSoundFile = sf::InputSoundFile::openFromFile("doodle_pop.ogg").value();
            const auto sampleOffset   = inputSoundFile.seek(1000);
            CHECK(inputSoundFile.getTimeOffset(sampleOffset) == sf::microseconds(11'337));
            CHECK(sampleOffset == 1000);
        }

        SECTION("wav")
        {
            auto       inputSoundFile = sf::InputSoundFile::openFromFile("killdeer.wav").value();
            const auto sampleOffset   = inputSoundFile.seek(1000);
            CHECK(inputSoundFile.getTimeOffset(sampleOffset) == sf::microseconds(45'351));
            CHECK(sampleOffset == 1000);
        }
    }

    SECTION("seek(Time)")
    {
        auto       inputSoundFile = sf::InputSoundFile::openFromFile("ding.flac").value();
        const auto sampleOffset   = inputSoundFile.seek(sf::milliseconds(100));
        CHECK(inputSoundFile.getSampleCount() == 87'798);
        CHECK(inputSoundFile.getChannelCount() == 1);
        CHECK(inputSoundFile.getSampleRate() == 44'100);
        CHECK(inputSoundFile.getDuration() == sf::microseconds(1'990'884));
        CHECK(inputSoundFile.getTimeOffset(sampleOffset) == sf::milliseconds(100));
        CHECK(sampleOffset == 4410);
    }

    SECTION("read()")
    {
        auto inputSoundFile = sf::InputSoundFile::openFromFile("ding.flac").value();

        SECTION("Null address")
        {
            CHECK(inputSoundFile.read(nullptr, 10) == 0);
        }

        sf::base::Array<sf::base::I16, 4> samples{};

        SECTION("Zero count")
        {
            CHECK(inputSoundFile.read(samples.data(), 0) == 0);
        }

        SECTION("Successful read")
        {
            SECTION("flac")
            {
                inputSoundFile = sf::InputSoundFile::openFromFile("ding.flac").value();
                CHECK(inputSoundFile.read(samples.data(), samples.size()) == 4);
                CHECK(samples == sf::base::Array<sf::base::I16, 4>{0, 1, -1, 4});
                CHECK(inputSoundFile.read(samples.data(), samples.size()) == 4);
                CHECK(samples == sf::base::Array<sf::base::I16, 4>{1, 4, 9, 6});
            }

            SECTION("mp3")
            {
                inputSoundFile = sf::InputSoundFile::openFromFile("ding.mp3").value();
                CHECK(inputSoundFile.read(samples.data(), samples.size()) == 4);
                CHECK(samples == sf::base::Array<sf::base::I16, 4>{0, -2, 0, 2});
                CHECK(inputSoundFile.read(samples.data(), samples.size()) == 4);
                CHECK(samples == sf::base::Array<sf::base::I16, 4>{1, 4, 6, 8});
            }

            SECTION("ogg")
            {
                inputSoundFile = sf::InputSoundFile::openFromFile("doodle_pop.ogg").value();
                CHECK(inputSoundFile.read(samples.data(), samples.size()) == 4);
                CHECK(samples == sf::base::Array<sf::base::I16, 4>{-827, -985, -1168, -1319});
                CHECK(inputSoundFile.read(samples.data(), samples.size()) == 4);
                CHECK(samples == sf::base::Array<sf::base::I16, 4>{-1738, -1883, -2358, -2497});
            }

            SECTION("wav")
            {
                // Cannot be tested since reading from a .wav file triggers UB
            }
        }
    }
}
