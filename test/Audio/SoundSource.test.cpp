#include "SFML/Audio/SoundSource.hpp"

#include "SFML/Audio/PlaybackDevice.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

namespace
{
class SoundSource : public sf::SoundSource
{
    void play(sf::PlaybackDevice&) override
    {
    }

    void pause() override
    {
    }

    void stop() override
    {
    }

    [[nodiscard]] void* getSound() const override
    {
        return {};
    }

public:
    [[nodiscard]] Status getStatus() const override
    {
        return {};
    }
};
} // namespace

TEST_CASE("[Audio] sf::SoundSource" * doctest::skip(skipAudioDeviceTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundSource));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundSource));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundSource));
        STATIC_CHECK(!SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::SoundSource));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::SoundSource));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundSource));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::SoundSource));
    }

    SECTION("Construction")
    {
        const SoundSource soundSource;
        CHECK(soundSource.getPitch() == 0);
        CHECK(soundSource.getVolume() == 0);
        CHECK(soundSource.getPosition() == sf::Vector3f());
        CHECK(!soundSource.isRelativeToListener());
        CHECK(soundSource.getMinDistance() == 0);
        CHECK(soundSource.getAttenuation() == 0);
        CHECK(soundSource.getStatus() == sf::SoundSource::Status::Stopped);
    }

    SECTION("Copy semantics")
    {
        const SoundSource soundSource;

        SECTION("Construction")
        {
            const SoundSource soundSourceCopy(soundSource); // NOLINT(performance-unnecessary-copy-initialization)
            CHECK(soundSourceCopy.getPitch() == 0);
            CHECK(soundSourceCopy.getVolume() == 0);
            CHECK(soundSourceCopy.getPosition() == sf::Vector3f());
            CHECK(!soundSourceCopy.isRelativeToListener());
            CHECK(soundSourceCopy.getMinDistance() == 0);
            CHECK(soundSourceCopy.getAttenuation() == 0);
            CHECK(soundSourceCopy.getStatus() == sf::SoundSource::Status::Stopped);
        }

        SECTION("Assignment")
        {
            SoundSource soundSourceCopy;
            soundSourceCopy = soundSource;
            CHECK(soundSourceCopy.getPitch() == 0);
            CHECK(soundSourceCopy.getVolume() == 0);
            CHECK(soundSourceCopy.getPosition() == sf::Vector3f());
            CHECK(!soundSourceCopy.isRelativeToListener());
            CHECK(soundSourceCopy.getMinDistance() == 0);
            CHECK(soundSourceCopy.getAttenuation() == 0);
            CHECK(soundSourceCopy.getStatus() == sf::SoundSource::Status::Stopped);
        }
    }

    SECTION("Set/get pitch")
    {
        SoundSource soundSource;
        soundSource.setPitch(42);
        CHECK(soundSource.getPitch() == 0);
    }

    SECTION("Set/get volume")
    {
        SoundSource soundSource;
        soundSource.setVolume(0.5f);
        CHECK(soundSource.getVolume() == 0);
    }

    SECTION("Set/get position")
    {
        SoundSource soundSource;
        soundSource.setPosition({1, 2, 3});
        CHECK(soundSource.getPosition() == sf::Vector3f());
    }

    SECTION("Set/get relative to listener")
    {
        SoundSource soundSource;
        soundSource.setRelativeToListener(true);
        CHECK(!soundSource.isRelativeToListener());
    }

    SECTION("Set/get min distance")
    {
        SoundSource soundSource;
        soundSource.setMinDistance(12.34f);
        CHECK(soundSource.getMinDistance() == 0);
    }

    SECTION("Set/get attenuation")
    {
        SoundSource soundSource;
        soundSource.setAttenuation(10);
        CHECK(soundSource.getAttenuation() == 0);
    }
}
