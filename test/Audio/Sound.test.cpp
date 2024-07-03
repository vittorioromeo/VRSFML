#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/Sound.hpp>

// Other 1st party headers
#include <SFML/Audio/SoundBuffer.hpp>

#include <SFML/System/Macros.hpp>
#include <SFML/System/Time.hpp>

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <SystemUtil.hpp>

#include <optional>
#include <type_traits>

TEST_CASE("[Audio] sf::Sound" * doctest::skip(skipAudioDeviceTests))
{
    auto audioContext        = sf::AudioContext::create().value();
    auto defaultDeviceHandle = audioContext.getDefaultDevice().value();
    auto playbackDevice      = sf::PlaybackDevice(audioContext, defaultDeviceHandle);

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_constructible_v<sf::Sound, sf::SoundBuffer&&>);
        STATIC_CHECK(!std::is_constructible_v<sf::Sound, const sf::SoundBuffer&&>);
        STATIC_CHECK(std::is_copy_constructible_v<sf::Sound>);
        STATIC_CHECK(std::is_copy_assignable_v<sf::Sound>);
        STATIC_CHECK(std::is_move_constructible_v<sf::Sound>);
        STATIC_CHECK(!std::is_nothrow_move_constructible_v<sf::Sound>);
        STATIC_CHECK(std::is_move_assignable_v<sf::Sound>);
        STATIC_CHECK(!std::is_nothrow_move_assignable_v<sf::Sound>);
        STATIC_CHECK(std::has_virtual_destructor_v<sf::Sound>);
    }

    const auto soundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();

    SECTION("Construction")
    {
        const sf::Sound sound(playbackDevice, soundBuffer);
        CHECK(&sound.getBuffer() == &soundBuffer);
        CHECK(!sound.getLoop());
        CHECK(sound.getPlayingOffset() == sf::Time::Zero);
        CHECK(sound.getStatus() == sf::Sound::Status::Stopped);
    }

    SECTION("Copy semantics")
    {
        const sf::Sound sound(playbackDevice, soundBuffer);

        SECTION("Construction")
        {
            const sf::Sound soundCopy(sound); // NOLINT(performance-unnecessary-copy-initialization)
            CHECK(&soundCopy.getBuffer() == &soundBuffer);
            CHECK(!soundCopy.getLoop());
            CHECK(soundCopy.getPlayingOffset() == sf::Time::Zero);
            CHECK(soundCopy.getStatus() == sf::Sound::Status::Stopped);
        }

        SECTION("Assignment")
        {
            const sf::SoundBuffer otherSoundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
            sf::Sound             soundCopy(playbackDevice, otherSoundBuffer);
            soundCopy = sound;
            CHECK(&soundCopy.getBuffer() == &soundBuffer);
            CHECK(!soundCopy.getLoop());
            CHECK(soundCopy.getPlayingOffset() == sf::Time::Zero);
            CHECK(soundCopy.getStatus() == sf::Sound::Status::Stopped);
        }
    }

    SECTION("Set/get buffer")
    {
        const sf::SoundBuffer otherSoundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
        sf::Sound             sound(playbackDevice, soundBuffer);
        sound.setBuffer(otherSoundBuffer);
        CHECK(&sound.getBuffer() == &otherSoundBuffer);
    }

    SECTION("Set/get loop")
    {
        sf::Sound sound(playbackDevice, soundBuffer);
        sound.setLoop(true);
        CHECK(sound.getLoop());
    }

    SECTION("Set/get playing offset")
    {
        sf::Sound sound(playbackDevice, soundBuffer);
        sound.setPlayingOffset(sf::seconds(10));
        CHECK(sound.getPlayingOffset() == sf::seconds(10));
    }

#ifdef SFML_ENABLE_LIFETIME_TRACKING
    SECTION("Lifetime tracking")
    {
        SECTION("Return local from function")
        {
            const auto badFunction = [&playbackDevice]
            {
                const auto localSoundBuffer = sf::SoundBuffer::loadFromFile("Audio/ding.flac").value();
                return sf::Sound(playbackDevice, localSoundBuffer);
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
                explicit BadStruct(sf::PlaybackDevice& playbackDevice) :
                memberSoundBuffer{sf::SoundBuffer::loadFromFile("Audio/ding.flac").value()},
                memberSound{playbackDevice, memberSoundBuffer}
                {
                }

                sf::SoundBuffer memberSoundBuffer;
                sf::Sound       memberSound;
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard;
            CHECK(!guard.fatalErrorTriggered());

            std::optional<BadStruct> badStruct0;
            badStruct0.emplace(playbackDevice);
            CHECK(!guard.fatalErrorTriggered());

            const BadStruct badStruct1 = SFML_MOVE(badStruct0.value());
            CHECK(!guard.fatalErrorTriggered());

            badStruct0.reset();
            CHECK(guard.fatalErrorTriggered());
        }
    }
#endif
}
