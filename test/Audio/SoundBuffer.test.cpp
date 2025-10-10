#include "SFML/Audio/SoundBuffer.hpp"

// Other 1st party headers
#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <SystemUtil.hpp>

#include <string>


TEST_CASE("[Audio] sf::SoundBuffer" * doctest::skip(skipAudioDeviceTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundBuffer));
    }

    SECTION("Copy semantics")
    {
        const auto soundBuffer = sf::SoundBuffer::loadFromFile("ding.flac").value();

        SECTION("Construction")
        {
            const sf::SoundBuffer soundBufferCopy(soundBuffer); // NOLINT(performance-unnecessary-copy-initialization)
            CHECK(soundBufferCopy.getSamples() != nullptr);
            CHECK(soundBufferCopy.getSampleCount() == 87'798);
            CHECK(soundBufferCopy.getSampleRate() == 44'100);
            CHECK(soundBufferCopy.getChannelCount() == 1);
            CHECK(soundBufferCopy.getDuration() == sf::microseconds(1'990'884));
        }

        SECTION("Assignment")
        {
            sf::SoundBuffer soundBufferCopy = sf::SoundBuffer::loadFromFile("doodle_pop.ogg").value();
            soundBufferCopy                 = soundBuffer;
            CHECK(soundBufferCopy.getSamples() != nullptr);
            CHECK(soundBufferCopy.getSampleCount() == 87'798);
            CHECK(soundBufferCopy.getSampleRate() == 44'100);
            CHECK(soundBufferCopy.getChannelCount() == 1);
            CHECK(soundBufferCopy.getDuration() == sf::microseconds(1'990'884));
        }
    }

    SECTION("Move construction")
    {
        auto soundBuffer = sf::SoundBuffer::loadFromFile("ding.flac").value();

        const sf::SoundBuffer soundBufferMove(SFML_BASE_MOVE(soundBuffer));
        CHECK(soundBufferMove.getSamples() != nullptr);
        CHECK(soundBufferMove.getSampleCount() == 87'798);
        CHECK(soundBufferMove.getSampleRate() == 44'100);
        CHECK(soundBufferMove.getChannelCount() == 1);
        CHECK(soundBufferMove.getDuration() == sf::microseconds(1'990'884));
    }

    SECTION("Move assignmment")
    {
        auto soundBuffer     = sf::SoundBuffer::loadFromFile("ding.flac").value();
        auto soundBufferMove = sf::SoundBuffer::loadFromFile("ding.flac").value();

        soundBufferMove = SFML_BASE_MOVE(soundBuffer);
        CHECK(soundBufferMove.getSamples() != nullptr);
        CHECK(soundBufferMove.getSampleCount() == 87'798);
        CHECK(soundBufferMove.getSampleRate() == 44'100);
        CHECK(soundBufferMove.getChannelCount() == 1);
        CHECK(soundBufferMove.getDuration() == sf::microseconds(1'990'884));
    }

    SECTION("loadFromFile()")
    {
        SECTION("Invalid filename")
        {
            CHECK(!sf::SoundBuffer::loadFromFile("does/not/exist.wav").hasValue());
        }

        SECTION("Valid file")
        {
            const std::u32string filenameSuffixes[]{U"", U"-ń", U"-🐌"};

            for (const auto& filenameSuffix : filenameSuffixes)
            {
                const sf::Path filename = U"ding" + filenameSuffix + U".flac";

                INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                const auto soundBuffer = sf::SoundBuffer::loadFromFile("ding.flac").value();

                CHECK(soundBuffer.getSamples() != nullptr);
                CHECK(soundBuffer.getSampleCount() == 87'798);
                CHECK(soundBuffer.getSampleRate() == 44'100);
                CHECK(soundBuffer.getChannelCount() == 1);
                CHECK(soundBuffer.getDuration() == sf::microseconds(1'990'884));
            }
        }
    }

    SECTION("loadFromMemory()")
    {
        SECTION("Invalid memory")
        {
            constexpr unsigned char memory[5]{};
            CHECK(!sf::SoundBuffer::loadFromMemory(memory, 5).hasValue());
        }

        SECTION("Valid memory")
        {
            const auto memory      = loadIntoMemory("ding.flac");
            const auto soundBuffer = sf::SoundBuffer::loadFromMemory(memory.data(), memory.size()).value();
            CHECK(soundBuffer.getSamples() != nullptr);
            CHECK(soundBuffer.getSampleCount() == 87'798);
            CHECK(soundBuffer.getSampleRate() == 44'100);
            CHECK(soundBuffer.getChannelCount() == 1);
            CHECK(soundBuffer.getDuration() == sf::microseconds(1'990'884));
        }
    }

    SECTION("loadFromStream()")
    {
        auto       stream      = sf::FileInputStream::open("ding.flac").value();
        const auto soundBuffer = sf::SoundBuffer::loadFromStream(stream).value();
        CHECK(soundBuffer.getSamples() != nullptr);
        CHECK(soundBuffer.getSampleCount() == 87'798);
        CHECK(soundBuffer.getSampleRate() == 44'100);
        CHECK(soundBuffer.getChannelCount() == 1);
        CHECK(soundBuffer.getDuration() == sf::microseconds(1'990'884));
    }

    SECTION("saveToFile()")
    {
        const std::u32string stems[]{U"tmp", U"tmp-ń", U"tmp-🐌"};
        const std::u32string extensions[]{U".wav", U".ogg", U".flac"};

        for (const auto& stem : stems)
        {
            for (const auto& extension : extensions)
            {
                const auto filename = sf::Path::tempDirectoryPath() / sf::Path(stem + extension);

                INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                {
                    const auto soundBuffer = sf::SoundBuffer::loadFromFile("ding.flac").value();
                    REQUIRE(soundBuffer.saveToFile(filename));
                }

                const auto soundBuffer = sf::SoundBuffer::loadFromFile(filename).value();
                CHECK(soundBuffer.getSamples() != nullptr);
                CHECK(soundBuffer.getSampleCount() == 87'798);
                CHECK(soundBuffer.getSampleRate() == 44'100);
                CHECK(soundBuffer.getChannelCount() == 1);
                CHECK(soundBuffer.getDuration() == sf::microseconds(1'990'884));

                CHECK(filename.remove());
            }
        }
    }
}
