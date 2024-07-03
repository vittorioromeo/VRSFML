#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/MusicSource.hpp>
#include <SFML/Audio/MusicStream.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

// Other 1st party headers
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/Time.hpp>

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <SystemUtil.hpp>

#include <thread>
#include <type_traits>

TEST_CASE("[Audio] sf::MusicSource" * doctest::skip(skipAudioDeviceTests))
{
    auto audioContext        = sf::AudioContext::create().value();
    auto defaultDeviceHandle = audioContext.getDefaultDevice().value();
    auto playbackDevice      = sf::PlaybackDevice(audioContext, defaultDeviceHandle);

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_copy_constructible_v<sf::MusicSource>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::MusicSource>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::MusicSource>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::MusicSource>);
        STATIC_CHECK(!std::has_virtual_destructor_v<sf::MusicSource>);

        STATIC_CHECK(!std::is_copy_constructible_v<sf::MusicStream>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::MusicStream>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::MusicStream>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::MusicStream>);
        STATIC_CHECK(std::has_virtual_destructor_v<sf::MusicStream>);
    }

    SECTION("Span")
    {
        const sf::MusicStream::Span<float> span;
        CHECK(span.offset == 0);
        CHECK(span.length == 0);

        const sf::MusicStream::TimeSpan timeSpan;
        CHECK(timeSpan.offset == sf::Time::Zero);
        CHECK(timeSpan.length == sf::Time::Zero);
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid file")
        {
            CHECK(!sf::MusicSource::openFromFile("does/not/exist.wav"));
        }

        SECTION("Valid file")
        {
            auto music = sf::MusicSource::openFromFile("Audio/ding.mp3").value();
            CHECK(static_cast<const sf::MusicSource&>(music).getDuration() == sf::microseconds(1990884));
            CHECK(static_cast<const sf::MusicSource&>(music).getChannelCount() == 1);
            CHECK(static_cast<const sf::MusicSource&>(music).getSampleRate() == 44100);

            auto musicStream = music.createStream(playbackDevice);

            const auto [offset, length] = musicStream.getLoopPoints();
            CHECK(offset == sf::Time::Zero);
            CHECK(length == sf::microseconds(1990884));
            CHECK(musicStream.getStatus() == sf::MusicStream::Status::Stopped);
            CHECK(musicStream.getPlayingOffset() == sf::Time::Zero);
            CHECK(!musicStream.getLoop());
        }
    }

    SECTION("openFromMemory()")
    {
        std::vector<std::byte> memory(10, std::byte{0xCA});

        SECTION("Invalid buffer")
        {
            CHECK(!sf::MusicSource::openFromMemory(memory.data(), memory.size()));
        }

        SECTION("Valid buffer")
        {
            memory = loadIntoMemory("Audio/ding.flac");

            auto music = sf::MusicSource::openFromMemory(memory.data(), memory.size()).value();
            CHECK(static_cast<const sf::MusicSource&>(music).getDuration() == sf::microseconds(1990884));
            CHECK(static_cast<const sf::MusicSource&>(music).getChannelCount() == 1);
            CHECK(static_cast<const sf::MusicSource&>(music).getSampleRate() == 44100);

            auto musicStream = music.createStream(playbackDevice);

            const auto [offset, length] = musicStream.getLoopPoints();
            CHECK(offset == sf::Time::Zero);
            CHECK(length == sf::microseconds(1990884));
            CHECK(musicStream.getStatus() == sf::MusicStream::Status::Stopped);
            CHECK(musicStream.getPlayingOffset() == sf::Time::Zero);
            CHECK(!musicStream.getLoop());
        }
    }

    SECTION("openFromStream()")
    {
        auto stream = sf::FileInputStream::open("Audio/doodle_pop.ogg").value();
        auto music  = sf::MusicSource::openFromStream(stream).value();
        CHECK(static_cast<const sf::MusicSource&>(music).getDuration() == sf::microseconds(24002176));
        CHECK(static_cast<const sf::MusicSource&>(music).getChannelCount() == 2);
        CHECK(static_cast<const sf::MusicSource&>(music).getSampleRate() == 44100);

        auto musicStream = music.createStream(playbackDevice);

        const auto [offset, length] = musicStream.getLoopPoints();
        CHECK(offset == sf::Time::Zero);
        CHECK(length == sf::microseconds(24002176));
        CHECK(musicStream.getStatus() == sf::MusicStream::Status::Stopped);
        CHECK(musicStream.getPlayingOffset() == sf::Time::Zero);
        CHECK(!musicStream.getLoop());
    }

    SECTION("play/pause/stop")
    {
        auto music       = sf::MusicSource::openFromFile("Audio/ding.mp3").value();
        auto musicStream = music.createStream(playbackDevice);

        // Wait for background thread to start
        musicStream.play();
        while (musicStream.getStatus() == sf::MusicStream::Status::Stopped)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        CHECK(musicStream.getStatus() == sf::MusicStream::Status::Playing);

        // Wait for background thread to pause
        musicStream.pause();
        while (musicStream.getStatus() == sf::MusicStream::Status::Playing)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        CHECK(musicStream.getStatus() == sf::MusicStream::Status::Paused);

        // Wait for background thread to stop
        musicStream.stop();
        while (musicStream.getStatus() == sf::MusicStream::Status::Paused)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        CHECK(musicStream.getStatus() == sf::MusicStream::Status::Stopped);
    }

    SECTION("setLoopPoints()")
    {
        auto music       = sf::MusicSource::openFromFile("Audio/killdeer.wav").value();
        auto musicStream = music.createStream(playbackDevice);

        musicStream.setLoopPoints({sf::seconds(1), sf::seconds(2)});
        CHECK(musicStream.getChannelCount() == 1);
        CHECK(musicStream.getSampleRate() == 22050);

        const auto [offset, length] = musicStream.getLoopPoints();
        CHECK(offset == sf::seconds(1));
        CHECK(length == sf::seconds(2));
        CHECK(musicStream.getStatus() == sf::MusicStream::Status::Stopped);
        CHECK(musicStream.getPlayingOffset() == sf::Time::Zero);
        CHECK(!musicStream.getLoop());
    }
}
