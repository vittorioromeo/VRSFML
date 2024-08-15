#include <SFML/Audio/SoundBuffer.hpp>

// Other 1st party headers
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/Time.hpp>

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[Audio] sf::SoundBuffer" * doctest::skip(skipAudioDeviceTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundBuffer));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::SoundBuffer));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundBuffer));
    }

    SECTION("Copy semantics")
    {
        const auto soundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

        SECTION("Construction")
        {
            const sf::SoundBuffer soundBufferCopy(soundBuffer); // NOLINT(performance-unnecessary-copy-initialization)
            CHECK(soundBufferCopy.getSamples() != nullptr);
            CHECK(soundBufferCopy.getSampleCount() == 87798);
            CHECK(soundBufferCopy.getSampleRate() == 44100);
            CHECK(soundBufferCopy.getChannelCount() == 1);
            CHECK(soundBufferCopy.getDuration() == sf::microseconds(1990884));
        }

        SECTION("Assignment")
        {
            sf::SoundBuffer soundBufferCopy = sf::SoundBuffer::loadFromFile("Audio/doodle_pop.ogg").value();
            soundBufferCopy                 = soundBuffer;
            CHECK(soundBufferCopy.getSamples() != nullptr);
            CHECK(soundBufferCopy.getSampleCount() == 87798);
            CHECK(soundBufferCopy.getSampleRate() == 44100);
            CHECK(soundBufferCopy.getChannelCount() == 1);
            CHECK(soundBufferCopy.getDuration() == sf::microseconds(1990884));
        }
    }

    SECTION("loadFromFile()")
    {
        SECTION("Invalid filename")
        {
            CHECK(!sf::SoundBuffer::loadFromFile("does/not/exist.wav").hasValue());
        }

        SECTION("Valid file")
        {
            const auto soundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
            CHECK(soundBuffer.getSamples() != nullptr);
            CHECK(soundBuffer.getSampleCount() == 87798);
            CHECK(soundBuffer.getSampleRate() == 44100);
            CHECK(soundBuffer.getChannelCount() == 1);
            CHECK(soundBuffer.getDuration() == sf::microseconds(1990884));
        }
    }

    SECTION("loadFromMemory()")
    {
        SECTION("Invalid memory")
        {
            constexpr std::byte memory[5]{};
            CHECK(!sf::SoundBuffer::loadFromMemory(memory, 5).hasValue());
        }

        SECTION("Valid memory")
        {
            const auto memory      = loadIntoMemory("Audio/ding.flac");
            const auto soundBuffer = sf::SoundBuffer::loadFromMemory(memory.data(), memory.size()).value();
            CHECK(soundBuffer.getSamples() != nullptr);
            CHECK(soundBuffer.getSampleCount() == 87798);
            CHECK(soundBuffer.getSampleRate() == 44100);
            CHECK(soundBuffer.getChannelCount() == 1);
            CHECK(soundBuffer.getDuration() == sf::microseconds(1990884));
        }
    }

    SECTION("loadFromStream()")
    {
        auto       stream      = sf::FileInputStream::open("Audio/ding.flac").value();
        const auto soundBuffer = sf::SoundBuffer::loadFromStream(stream).value();
        CHECK(soundBuffer.getSamples() != nullptr);
        CHECK(soundBuffer.getSampleCount() == 87798);
        CHECK(soundBuffer.getSampleRate() == 44100);
        CHECK(soundBuffer.getChannelCount() == 1);
        CHECK(soundBuffer.getDuration() == sf::microseconds(1990884));
    }

    SECTION("saveToFile()")
    {
        const auto filename = sf::Path::tempDirectoryPath() / "ding.flac";

        {
            const auto soundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
            REQUIRE(soundBuffer.saveToFile(filename));
        }

        const auto soundBuffer = sf::SoundBuffer::loadFromFile(filename).value();
        CHECK(soundBuffer.getSamples() != nullptr);
        CHECK(soundBuffer.getSampleCount() == 87798);
        CHECK(soundBuffer.getSampleRate() == 44100);
        CHECK(soundBuffer.getChannelCount() == 1);
        CHECK(soundBuffer.getDuration() == sf::microseconds(1990884));

        CHECK(filename.remove());
    }
}
