#include "SFML/Audio/Sound.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

// Other 1st party headers
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[Audio] sf::Sound" * doctest::skip(skipAudioDeviceTests))
{
    auto               audioContext = sf::AudioContext::create().value();
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Sound, sf::SoundBuffer&&));
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Sound, const sf::SoundBuffer&&));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Sound));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Sound));
        STATIC_CHECK(!SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::Sound));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Sound));
        STATIC_CHECK(!SFML_BASE_IS_MOVE_ASSIGNABLE(sf::Sound));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Sound));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::Sound));
    }

    const auto soundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

    SECTION("Construction")
    {
        const sf::Sound sound(playbackDevice, soundBuffer, sf::AudioSettings{});
        CHECK(&sound.getBuffer() == &soundBuffer);
        CHECK(!sound.isLooping());
        CHECK(sound.getPlayingOffset() == sf::Time{});
        CHECK(!sound.isPlaying());
    }

    SECTION("Get buffer")
    {
        sf::Sound sound(playbackDevice, soundBuffer, sf::AudioSettings{});
        CHECK(&sound.getBuffer() == &soundBuffer);
    }

    SECTION("Set/get loop")
    {
        sf::Sound sound(playbackDevice, soundBuffer, sf::AudioSettings{});
        sound.setLooping(true);
        CHECK(sound.isLooping());
    }

    SECTION("Set/get playing offset")
    {
        sf::Sound sound(playbackDevice, soundBuffer, sf::AudioSettings{});
        sound.setPlayingOffset(sf::seconds(10)); // TODO P0: i think this must be stored in SoundSource as well?
        CHECK(sound.getPlayingOffset() == sf::seconds(10));
    }

    SECTION("Sound buffer move")
    {
        auto soundBufferA = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
        auto soundBufferB = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

        const sf::Sound sound(playbackDevice, soundBufferA, sf::AudioSettings{});
        CHECK(&sound.getBuffer() == &soundBufferA);
        CHECK(!sound.isLooping());
        CHECK(sound.getPlayingOffset() == sf::Time{});
        CHECK(!sound.isPlaying());

        soundBufferB = SFML_BASE_MOVE(soundBufferA); // TODO P0: this mistkae should be detected
        CHECK(&sound.getBuffer() == &soundBufferA);
    }

    SECTION("Multiple sounds and buffers -- copy")
    {
        auto soundBufferA = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
        auto soundBufferB = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

        const sf::Sound soundA(playbackDevice, soundBufferA, sf::AudioSettings{});
        const sf::Sound soundB(playbackDevice, soundBufferB, sf::AudioSettings{});

        CHECK(&soundA.getBuffer() == &soundBufferA);
        CHECK(&soundB.getBuffer() == &soundBufferB);

        // TODO P0: fix
        // soundBufferB = soundBufferA;

        CHECK(&soundA.getBuffer() == &soundBufferA);
        CHECK(&soundB.getBuffer() == &soundBufferB);
    }

    SECTION("Multiple sounds and buffers")
    {
        auto soundBufferA = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
        auto soundBufferB = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

        const sf::Sound soundA(playbackDevice, soundBufferA, sf::AudioSettings{});
        const sf::Sound soundB(playbackDevice, soundBufferB, sf::AudioSettings{});

        CHECK(&soundA.getBuffer() == &soundBufferA);
        CHECK(&soundB.getBuffer() == &soundBufferB);

        soundBufferB = SFML_BASE_MOVE(soundBufferA);

        CHECK(&soundA.getBuffer() == &soundBufferA);
        CHECK(&soundB.getBuffer() == &soundBufferB);
    }

#ifdef SFML_ENABLE_LIFETIME_TRACKING
    SECTION("Lifetime tracking")
    {
        SECTION("Return local from function")
        {
            const auto badFunction = [&playbackDevice]
            {
                const auto localSoundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
                return sf::Sound(playbackDevice, localSoundBuffer, sf::AudioSettings{});
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard{"SoundBuffer"};
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            badFunction();

            CHECK(guard.fatalErrorTriggered("SoundBuffer"));
        }

        SECTION("Move struct holding both dependee and dependant")
        {
            struct BadStruct
            {
                explicit BadStruct(sf::PlaybackDevice& thePlaybackDevice) :
                memberSoundBuffer{sf::SoundBuffer::loadFromFile("Audio/ding.flac").value()},
                memberSound{thePlaybackDevice, memberSoundBuffer, sf::AudioSettings{}}
                {
                }

                sf::SoundBuffer memberSoundBuffer;
                sf::Sound       memberSound;
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard{"SoundBuffer"};
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            sf::base::Optional<BadStruct> badStruct0;
            badStruct0.emplace(playbackDevice);
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            badStruct0.reset();
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));
        }

        SECTION("Dependee move assignment")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"SoundBuffer"};
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            auto sb0 = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            sf::Sound s0(playbackDevice, sb0, sf::AudioSettings{});
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            sb0 = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));
        }

        SECTION("Dependee destroyed before dependant")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"SoundBuffer"};
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            auto optDependee = sf::SoundBuffer::loadFromFile("Audio/ding.flac");
            CHECK(optDependee.hasValue());
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            sf::Sound s0(playbackDevice, *optDependee, sf::AudioSettings{});
            CHECK(!guard.fatalErrorTriggered("SoundBuffer"));

            optDependee.reset();
            CHECK(guard.fatalErrorTriggered("SoundBuffer"));
        }
    }
#endif
}
