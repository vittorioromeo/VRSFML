#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

// Other 1st party headers
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <SystemUtil.hpp>

#include <type_traits>


TEST_CASE("[Audio] sf::Music" * doctest::skip(skipAudioDeviceTests))
{
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_copy_constructible_v<sf::Music>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::Music>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Music>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Music>);
        STATIC_CHECK(std::has_virtual_destructor_v<sf::Music>);

        STATIC_CHECK(!std::is_copy_constructible_v<sf::Music>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::Music>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Music>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Music>);
        STATIC_CHECK(std::has_virtual_destructor_v<sf::Music>);
    }

    SECTION("Span")
    {
        const sf::Music::Span<float> span;
        CHECK(span.offset == 0);
        CHECK(span.length == 0);

        const sf::Music::TimeSpan timeSpan;
        CHECK(timeSpan.offset == sf::Time::Zero);
        CHECK(timeSpan.length == sf::Time::Zero);
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid file")
        {
            CHECK(!sf::Music::openFromFile("does/not/exist.wav").hasValue());
        }

        SECTION("Valid file")
        {
            auto music = sf::Music::openFromFile("Audio/ding.mp3").value();
            CHECK(static_cast<const sf::Music&>(music).getDuration() == sf::microseconds(1990884));
            CHECK(static_cast<const sf::Music&>(music).getChannelCount() == 1);
            CHECK(static_cast<const sf::Music&>(music).getSampleRate() == 44100);

            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::Time::Zero);
            CHECK(length == sf::microseconds(1990884));
            CHECK(music.getStatus() == sf::Music::Status::Stopped);
            CHECK(music.getPlayingOffset() == sf::Time::Zero);
            CHECK(!music.isLooping());
        }
    }

    SECTION("openFromMemory()")
    {
        std::vector<std::byte> memory(10, std::byte{0xCA});

        SECTION("Invalid buffer")
        {
            CHECK(!sf::Music::openFromMemory(memory.data(), memory.size()).hasValue());
        }

        SECTION("Valid buffer")
        {
            memory = loadIntoMemory("Audio/ding.flac");

            auto music = sf::Music::openFromMemory(memory.data(), memory.size()).value();
            CHECK(static_cast<const sf::Music&>(music).getDuration() == sf::microseconds(1990884));
            CHECK(static_cast<const sf::Music&>(music).getChannelCount() == 1);
            CHECK(static_cast<const sf::Music&>(music).getSampleRate() == 44100);

            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::Time::Zero);
            CHECK(length == sf::microseconds(1990884));
            CHECK(music.getStatus() == sf::Music::Status::Stopped);
            CHECK(music.getPlayingOffset() == sf::Time::Zero);
            CHECK(!music.isLooping());
        }
    }

    SECTION("openFromStream()")
    {
        auto stream = sf::FileInputStream::open("Audio/doodle_pop.ogg").value();
        auto music  = sf::Music::openFromStream(stream).value();
        CHECK(static_cast<const sf::Music&>(music).getDuration() == sf::microseconds(24002176));
        CHECK(static_cast<const sf::Music&>(music).getChannelCount() == 2);
        CHECK(static_cast<const sf::Music&>(music).getSampleRate() == 44100);

        const auto [offset, length] = music.getLoopPoints();
        CHECK(offset == sf::Time::Zero);
        CHECK(length == sf::microseconds(24002176));
        CHECK(music.getStatus() == sf::Music::Status::Stopped);
        CHECK(music.getPlayingOffset() == sf::Time::Zero);
        CHECK(!music.isLooping());
    }

    SECTION("play/pause/stop")
    {
        auto music = sf::Music::openFromFile("Audio/ding.mp3").value();
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
        auto music = sf::Music::openFromFile("Audio/killdeer.wav").value();
        music.setLoopPoints({sf::seconds(1), sf::seconds(2)});
        CHECK(music.getChannelCount() == 1);
        CHECK(music.getSampleRate() == 22050);

        const auto [offset, length] = music.getLoopPoints();
        CHECK(offset == sf::seconds(1));
        CHECK(length == sf::seconds(2));
        CHECK(music.getStatus() == sf::Music::Status::Stopped);
        CHECK(music.getPlayingOffset() == sf::Time::Zero);
        CHECK(!music.isLooping());
    }
}
