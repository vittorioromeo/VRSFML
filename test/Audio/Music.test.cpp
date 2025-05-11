#include "SFML/Audio/Music.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

// Other 1st party headers
#include "SFML/Audio/MusicSource.hpp"

#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Builtins/Memset.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <SystemUtil.hpp>

#include <string>


TEST_CASE("[Audio] sf::Music" * doctest::skip(skipAudioDeviceTests))
{
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Music));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Music));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Music));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Music));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::Music));

        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Music));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Music));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Music));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Music));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::Music));
    }

    SECTION("Span")
    {
        const sf::Music::Span<float> span;
        CHECK(span.offset == 0);
        CHECK(span.length == 0);

        const sf::Music::TimeSpan timeSpan;
        CHECK(timeSpan.offset == sf::Time{});
        CHECK(timeSpan.length == sf::Time{});
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid file")
        {
            CHECK(!sf::MusicSource::openFromFile("does/not/exist.wav").hasValue());
        }

        SECTION("Valid file")
        {
            const std::u32string filenameSuffixes[] = {U"", U"-ń", U"-🐌"};

            for (const auto& filenameSuffix : filenameSuffixes)
            {
                const sf::Path filename = U"Audio/ding" + filenameSuffix + U".mp3";
                INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                auto musicSource = sf::MusicSource::openFromFile("Audio/ding.mp3").value();
                CHECK(musicSource.getDuration() == sf::microseconds(1'990'884));

                sf::Music music(musicSource);

                const auto [offset, length] = music.getLoopPoints();
                CHECK(offset == sf::Time{});
                CHECK(length == sf::microseconds(1'990'884));

                CHECK(music.getStatus() == sf::Music::Status::Stopped);
                CHECK(music.getPlayingOffset() == sf::Time{});
                CHECK(!music.isLooping());
            }
        }
    }

    SECTION("openFromMemory()")
    {
        sf::base::Vector<unsigned char> memory(10);
        SFML_BASE_MEMSET(memory.data(), 0xCA, 10);

        SECTION("Invalid buffer")
        {
            CHECK(!sf::MusicSource::openFromMemory(memory.data(), memory.size()).hasValue());
        }

        SECTION("Valid buffer")
        {
            memory = loadIntoMemory("Audio/ding.flac");

            auto musicSource = sf::MusicSource::openFromMemory(memory.data(), memory.size()).value();
            CHECK(static_cast<const sf::MusicSource&>(musicSource).getDuration() == sf::microseconds(1'990'884));
            CHECK(static_cast<const sf::MusicSource&>(musicSource).getChannelCount() == 1);
            CHECK(static_cast<const sf::MusicSource&>(musicSource).getSampleRate() == 44'100);

            sf::Music music(musicSource);

            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::Time{});
            CHECK(length == sf::microseconds(1'990'884));

            CHECK(music.getStatus() == sf::Music::Status::Stopped);
            CHECK(music.getPlayingOffset() == sf::Time{});
            CHECK(!music.isLooping());
        }
    }

    SECTION("openFromStream()")
    {
        auto stream      = sf::FileInputStream::open("Audio/doodle_pop.ogg").value();
        auto musicSource = sf::MusicSource::openFromStream(stream).value();
        CHECK(static_cast<const sf::MusicSource&>(musicSource).getDuration() == sf::microseconds(24'002'176));
        CHECK(static_cast<const sf::MusicSource&>(musicSource).getChannelCount() == 2);
        CHECK(static_cast<const sf::MusicSource&>(musicSource).getSampleRate() == 44'100);

        sf::Music music(musicSource);

        const auto [offset, length] = music.getLoopPoints();
        CHECK(offset == sf::Time{});
        CHECK(length == sf::microseconds(24'002'176));

        CHECK(music.getStatus() == sf::Music::Status::Stopped);
        CHECK(music.getPlayingOffset() == sf::Time{});
        CHECK(!music.isLooping());
    }

    SECTION("play/pause/stop")
    {
        auto musicSource = sf::MusicSource::openFromFile("Audio/ding.mp3").value();

        sf::Music music(musicSource);

        // Wait for background thread to start
        music.play(playbackDevice);
        while (music.getStatus() == sf::Music::Status::Stopped)
            sf::sleep(sf::milliseconds(10));
        CHECK(music.getStatus() == sf::Music::Status::Playing);

        // Wait for background thread to pause
        music.pause();
        while (music.getStatus() == sf::Music::Status::Playing)
            sf::sleep(sf::milliseconds(10));
        CHECK(music.getStatus() == sf::Music::Status::Paused);

        // Wait for background thread to stop
        music.stop();
        while (music.getStatus() == sf::Music::Status::Paused)
            sf::sleep(sf::milliseconds(10));
        CHECK(music.getStatus() == sf::Music::Status::Stopped);
    }

    SECTION("setLoopPoints()")
    {
        auto musicSource = sf::MusicSource::openFromFile("Audio/killdeer.wav").value();

        sf::Music music(musicSource);

        music.setLoopPoints({sf::seconds(1), sf::seconds(2)});
        CHECK(music.getChannelCount() == 1);
        CHECK(music.getSampleRate() == 22'050);

        SECTION("Within range")
        {
            music.setLoopPoints({sf::seconds(1), sf::seconds(2)});
            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::seconds(1));
            CHECK(length == sf::seconds(2));
        }

        SECTION("Duration too long")
        {
            music.setLoopPoints({sf::seconds(1), sf::seconds(1000)});
            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::seconds(1));
            CHECK(length == sf::microseconds(4'122'040));
        }

        SECTION("Offset too long")
        {
            musicSource = sf::MusicSource::openFromFile("Audio/killdeer.wav").value();

            music.setLoopPoints({sf::seconds(1000), sf::milliseconds(10)});
            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::seconds(0));
            CHECK(length == sf::microseconds(5'122'040));
        }

        CHECK(music.getChannelCount() == 1);
        CHECK(music.getSampleRate() == 22'050);
        CHECK(music.getStatus() == sf::Music::Status::Stopped);
        CHECK(music.getPlayingOffset() == sf::Time{});
        CHECK(!music.isLooping());
    }
}
