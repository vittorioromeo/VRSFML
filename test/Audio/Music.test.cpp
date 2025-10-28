#include "SFML/Audio/Music.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

// Other 1st party headers
#include "SFML/Audio/MusicReader.hpp"

#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Builtin/Memset.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <SystemUtil.hpp>

#include <string>


TEST_CASE("[Audio] sf::Music" * doctest::skip(skipAudioDeviceTests))
{
    auto               audioContext = sf::AudioContext::create().value();
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

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
            CHECK(!sf::MusicReader::openFromFile("does/not/exist.wav").hasValue());
        }

        SECTION("Valid file")
        {
            const std::u32string filenameSuffixes[] = {U"", U"-ń", U"-🐌"};

            for (const auto& filenameSuffix : filenameSuffixes)
            {
                const sf::Path filename = U"ding" + filenameSuffix + U".mp3";
                INFO("Filename: " << reinterpret_cast<const char*>(filename.to<std::u8string>().c_str()));

                auto musicReader = sf::MusicReader::openFromFile("ding.mp3").value();
                CHECK(musicReader.getDuration() == sf::microseconds(1'990'884));

                sf::Music music(playbackDevice, musicReader);

                const auto [offset, length] = music.getLoopPoints();
                CHECK(offset == sf::Time{});
                CHECK(length == sf::microseconds(1'990'884));

                CHECK(!music.isPlaying());
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
            CHECK(!sf::MusicReader::openFromMemory(memory.data(), memory.size()).hasValue());
        }

        SECTION("Valid buffer")
        {
            memory = loadIntoMemory("ding.flac");

            auto musicReader = sf::MusicReader::openFromMemory(memory.data(), memory.size()).value();
            CHECK(static_cast<const sf::MusicReader&>(musicReader).getDuration() == sf::microseconds(1'990'884));
            CHECK(static_cast<const sf::MusicReader&>(musicReader).getChannelCount() == 1);
            CHECK(static_cast<const sf::MusicReader&>(musicReader).getSampleRate() == 44'100);

            sf::Music music(playbackDevice, musicReader);

            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::Time{});
            CHECK(length == sf::microseconds(1'990'884));

            CHECK(!music.isPlaying());
            CHECK(music.getPlayingOffset() == sf::Time{});
            CHECK(!music.isLooping());
        }
    }

    SECTION("openFromStream()")
    {
        auto stream      = sf::FileInputStream::open("doodle_pop.ogg").value();
        auto musicReader = sf::MusicReader::openFromStream(stream).value();
        CHECK(static_cast<const sf::MusicReader&>(musicReader).getDuration() == sf::microseconds(24'002'176));
        CHECK(static_cast<const sf::MusicReader&>(musicReader).getChannelCount() == 2);
        CHECK(static_cast<const sf::MusicReader&>(musicReader).getSampleRate() == 44'100);

        sf::Music music(playbackDevice, musicReader);

        const auto [offset, length] = music.getLoopPoints();
        CHECK(offset == sf::Time{});
        CHECK(length == sf::microseconds(24'002'176));

        CHECK(!music.isPlaying());
        CHECK(music.getPlayingOffset() == sf::Time{});
        CHECK(!music.isLooping());
    }

    SECTION("play/pause/stop")
    {
        auto musicReader = sf::MusicReader::openFromFile("ding.mp3").value();

        sf::Music music(playbackDevice, musicReader);

        // Wait for background thread to start
        music.play();
        while (!music.isPlaying())
            sf::sleep(sf::milliseconds(10));
        CHECK(music.isPlaying());

        // Wait for background thread to pause
        music.pause();
        while (music.isPlaying())
            sf::sleep(sf::milliseconds(10));
        CHECK(!music.isPlaying());

        // Wait for background thread to stop
        music.stop();
        while (music.isPlaying())
            sf::sleep(sf::milliseconds(10));
        CHECK(!music.isPlaying());
    }

    SECTION("setLoopPoints()")
    {
        auto musicReader = sf::MusicReader::openFromFile("killdeer.wav").value();

        sf::Music music(playbackDevice, musicReader);

        music.setLoopPoints({sf::seconds(1), sf::seconds(2)});
        CHECK(musicReader.getChannelCount() == 1);
        CHECK(musicReader.getSampleRate() == 22'050);

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
            musicReader = sf::MusicReader::openFromFile("killdeer.wav").value();

            music.setLoopPoints({sf::seconds(1000), sf::milliseconds(10)});
            const auto [offset, length] = music.getLoopPoints();
            CHECK(offset == sf::seconds(0));
            CHECK(length == sf::microseconds(5'122'040));
        }

        CHECK(musicReader.getChannelCount() == 1);
        CHECK(musicReader.getSampleRate() == 22'050);
        CHECK(!music.isPlaying());
        CHECK(music.getPlayingOffset() == sf::Time{});
        CHECK(!music.isLooping());
    }

#if defined(SFML_ENABLE_LIFETIME_TRACKING) && !defined(SFML_SYSTEM_ADDRESS_SANITIZER_DETECTED)
    SECTION("Lifetime tracking")
    {
        SECTION("Return local from function")
        {
            const auto badFunction = [&playbackDevice]
            {
                auto localMusicSource = sf::MusicReader::openFromFile("ding.mp3").value();
                return sf::Music(playbackDevice, localMusicSource);
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard{"MusicReader"};
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            badFunction();

            CHECK(guard.fatalErrorTriggered("MusicReader"));
        }

        SECTION("Move struct holding both dependee and dependant")
        {
            struct BadStruct
            {
                explicit BadStruct(sf::PlaybackDevice& thePlaybackDevice) :
                    memberMusicSource{sf::MusicReader::openFromFile("ding.mp3").value()},
                    memberSound{thePlaybackDevice, memberMusicSource}
                {
                }

                sf::MusicReader memberMusicSource;
                sf::Music       memberSound;
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard{"MusicReader"};
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            sf::base::Optional<BadStruct> badStruct0;
            badStruct0.emplace(playbackDevice);
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            badStruct0.reset();
            CHECK(!guard.fatalErrorTriggered("MusicReader"));
        }

        SECTION("Dependee move assignment")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"MusicReader"};
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            auto sb0 = sf::MusicReader::openFromFile("ding.mp3").value();
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            sf::Music s0(playbackDevice, sb0);
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            sb0 = sf::MusicReader::openFromFile("ding.mp3").value();
            CHECK(!guard.fatalErrorTriggered("MusicReader"));
        }

        SECTION("Dependee destroyed before dependant")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"MusicReader"};
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            auto optDependee = sf::MusicReader::openFromFile("ding.mp3");
            CHECK(optDependee.hasValue());
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            sf::Music s0(playbackDevice, *optDependee);
            CHECK(!guard.fatalErrorTriggered("MusicReader"));

            optDependee.reset();
            CHECK(guard.fatalErrorTriggered("MusicReader"));
        }
    }
#endif
}
