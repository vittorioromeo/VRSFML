#include "SFML/Audio/Sound.hpp"

#include "SFML/Audio/AudioContext.hpp"
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
        const sf::Sound sound(soundBuffer);
        CHECK(&sound.getBuffer() == &soundBuffer);
        CHECK(!sound.isLooping());
        CHECK(sound.getPlayingOffset() == sf::Time{});
        CHECK(sound.getStatus() == sf::Sound::Status::Stopped);
    }

    SECTION("Get buffer")
    {
        sf::Sound sound(soundBuffer);
        CHECK(&sound.getBuffer() == &soundBuffer);
    }

    SECTION("Set/get loop")
    {
        sf::Sound sound(soundBuffer);
        sound.setLooping(true);
        CHECK(sound.isLooping());
    }

    SECTION("Set/get playing offset")
    {
        sf::Sound sound(soundBuffer);
        sound.setPlayingOffset(sf::seconds(10)); // TODO P0: i think this must be stored in SoundSource as well?
        CHECK(sound.getPlayingOffset() == sf::seconds(10));
    }

    SECTION("Sound buffer move")
    {
        auto soundBufferA = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
        auto soundBufferB = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

        const sf::Sound sound(soundBufferA);
        CHECK(&sound.getBuffer() == &soundBufferA);
        CHECK(!sound.isLooping());
        CHECK(sound.getPlayingOffset() == sf::Time{});
        CHECK(sound.getStatus() == sf::Sound::Status::Stopped);

        soundBufferB = SFML_BASE_MOVE(soundBufferA); // TODO P0: this mistkae should be detected
        CHECK(&sound.getBuffer() == &soundBufferA);
    }

    SECTION("Multiple sounds and buffers -- copy")
    {
        auto soundBufferA = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
        auto soundBufferB = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

        const sf::Sound soundA(soundBufferA);
        const sf::Sound soundB(soundBufferB);

        CHECK(&soundA.getBuffer() == &soundBufferA);
        CHECK(&soundB.getBuffer() == &soundBufferB);

        soundBufferB = soundBufferA;

        CHECK(&soundA.getBuffer() == &soundBufferA);
        CHECK(&soundB.getBuffer() == &soundBufferB);
    }

    SECTION("Multiple sounds and buffers")
    {
        auto soundBufferA = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
        auto soundBufferB = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

        const sf::Sound soundA(soundBufferA);
        const sf::Sound soundB(soundBufferB);

        CHECK(&soundA.getBuffer() == &soundBufferA);
        CHECK(&soundB.getBuffer() == &soundBufferB);

        soundBufferB = SFML_BASE_MOVE(soundBufferA);

        CHECK(&soundA.getBuffer() == &soundBufferB);
        CHECK(&soundB.getBuffer() == &soundBufferB);
    }

#ifdef SFML_ENABLE_LIFETIME_TRACKING
    SECTION("Lifetime tracking")
    {
        SECTION("Return local from function")
        {
            const auto badFunction = []
            {
                const auto localSoundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
                return sf::Sound(localSoundBuffer);
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard;
            CHECK(!guard.fatalErrorTriggered());

            badFunction();

            CHECK(guard.fatalErrorTriggered());
        }

        SECTION("Move struct holding both dependee and dependant")
        {
            struct BadStruct
            {
                explicit BadStruct() :
                memberSoundBuffer{sf::SoundBuffer::loadFromFile("Audio/ding.flac").value()},
                memberSound{memberSoundBuffer}
                {
                }

                sf::SoundBuffer memberSoundBuffer;
                sf::Sound       memberSound;
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard;
            CHECK(!guard.fatalErrorTriggered());

            sf::base::Optional<BadStruct> badStruct0;
            badStruct0.emplace();
            CHECK(!guard.fatalErrorTriggered());

            badStruct0.reset();
            CHECK(!guard.fatalErrorTriggered());
        }

        SECTION("Dependee move assignment")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard;
            CHECK(!guard.fatalErrorTriggered());

            auto sb0 = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
            CHECK(!guard.fatalErrorTriggered());

            sf::Sound s0(sb0);
            CHECK(!guard.fatalErrorTriggered());

            sb0 = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
            CHECK(!guard.fatalErrorTriggered());
        }

        SECTION("Dependee destroyed before dependant")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard;
            CHECK(!guard.fatalErrorTriggered());

            auto optDependee = sf::SoundBuffer::loadFromFile("Audio/ding.flac");
            CHECK(optDependee.hasValue());
            CHECK(!guard.fatalErrorTriggered());

            sf::Sound s0(*optDependee);
            CHECK(!guard.fatalErrorTriggered());

            optDependee.reset();
            CHECK(guard.fatalErrorTriggered());
        }
    }
#endif
}
