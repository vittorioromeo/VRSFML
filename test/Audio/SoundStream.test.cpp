#include "SFML/Audio/SoundStream.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundChannel.hpp"

#include "SFML/System/Time.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <SystemUtil.hpp>


namespace
{
class TestSoundStream : public sf::SoundStream
{
public:
    explicit TestSoundStream(sf::PlaybackDevice& playbackDevice) :
    sf::SoundStream{playbackDevice, sf::ChannelMap{sf::SoundChannel::Mono}, 0u}
    {
    }

protected:
    [[nodiscard]] bool onGetData(Chunk& /* data */) override
    {
        return true;
    }

    void onSeek(sf::Time /* timeOffset */) override
    {
    }
};

} // namespace

TEST_CASE("[Audio] sf::SoundStream" * doctest::skip(skipAudioDeviceTests))
{
    auto               audioContext = sf::AudioContext::create().value();
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundStream));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundStream));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundStream));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::SoundStream));
    }

    SECTION("Chunk")
    {
        const sf::SoundStream::Chunk chunk;
        CHECK(chunk.samples == nullptr);
        CHECK(chunk.sampleCount == 0);
    }

    SECTION("Construction")
    {
        const TestSoundStream testSoundStream(playbackDevice);
        CHECK(!testSoundStream.isPlaying());
        CHECK(testSoundStream.getPlayingOffset() == sf::Time{});
        CHECK(!testSoundStream.isLooping());
    }

    SECTION("Set/get playing offset")
    {
        TestSoundStream testSoundStream(playbackDevice);
        testSoundStream.setPlayingOffset(sf::milliseconds(100));
        CHECK(testSoundStream.getPlayingOffset() == sf::milliseconds(0));
    }

    SECTION("Set/get loop")
    {
        TestSoundStream testSoundStream(playbackDevice);
        testSoundStream.setLooping(true);
        CHECK(testSoundStream.isLooping());
    }

    SECTION("Set/get pitch")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setPitch(42);
        CHECK(soundStream.getPitch() == 42);
    }

    SECTION("Set/get pan")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setPan(1.f);
        CHECK(soundStream.getPan() == 1.f);
        soundStream.setPan(2.f);
        CHECK(soundStream.getPan() == 2.f);
        soundStream.setPan(-2.f);
        CHECK(soundStream.getPan() == -2.f);
    }

    SECTION("Set/get volume")
    {
        TestSoundStream soundStream(playbackDevice);

        soundStream.setVolume(0.f);
        CHECK(soundStream.getVolume() == 0.f);

        soundStream.setVolume(0.5f);
        CHECK(soundStream.getVolume() == 0.5f);

        soundStream.setVolume(1.f);
        CHECK(soundStream.getVolume() == 1.f);
    }

    SECTION("Set/get spatialization enabled")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setSpatializationEnabled(true);
        CHECK(soundStream.isSpatializationEnabled());
    }

    SECTION("Set/get position")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setPosition({1, 2, 3});
        CHECK(soundStream.getPosition() == sf::Vec3f(1, 2, 3));
    }

    SECTION("Set/get direction")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setDirection({4, 5, 6});
        CHECK(soundStream.getDirection() == sf::Vec3f(4, 5, 6));
    }

    SECTION("Set/get cone")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setCone({sf::radians(1), sf::radians(2), 3});
        CHECK(soundStream.getCone().innerAngle == sf::radians(1));
        CHECK(soundStream.getCone().outerAngle == sf::radians(2));
        CHECK(soundStream.getCone().outerGain == 3);
    }

    SECTION("Set/get velocity")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setVelocity({7, 8, 9});
        CHECK(soundStream.getVelocity() == sf::Vec3f(7, 8, 9));
    }

    SECTION("Set/get doppler factor")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setDopplerFactor(1);
        CHECK(soundStream.getDopplerFactor() == 1);
    }

    SECTION("Set/get directional attenuation factor")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setDirectionalAttenuationFactor(1);
        CHECK(soundStream.getDirectionalAttenuationFactor() == 1);
    }

    SECTION("Set/get relative to listener")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setRelativeToListener(true);
        CHECK(soundStream.isRelativeToListener());
    }

    SECTION("Set/get min distance")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setMinDistance(12.34f);
        CHECK(soundStream.getMinDistance() == 12.34f);
    }

    SECTION("Set/get max distance")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setMaxDistance(12.34f);
        CHECK(soundStream.getMaxDistance() == 12.34f);
    }

    SECTION("Set/get min gain")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setMinGain(12.34f);
        CHECK(soundStream.getMinGain() == 12.34f);
    }

    SECTION("Set/get max gain")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setMaxGain(12.34f);
        CHECK(soundStream.getMaxGain() == 12.34f);
    }

    SECTION("Set/get attenuation")
    {
        TestSoundStream soundStream(playbackDevice);
        soundStream.setAttenuation(10);
        CHECK(soundStream.getAttenuation() == 10);
    }
}
