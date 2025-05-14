#if 0 // TODO P0

    #include "SFML/Audio/PlaybackDevice.hpp"
    #include "SFML/Audio/Priv/MiniaudioSoundSource.hpp"
    #include "SFML/Audio/SoundBase.hpp"

    #include "SFML/System/Time.hpp"

    #include "SFML/Base/FloatMax.hpp"

    #include <Doctest.hpp>

    #include <AudioUtil.hpp>
    #include <CommonTraits.hpp>
    #include <SystemUtil.hpp>


namespace
{
////////////////////////////////////////////////////////////
class TestSoundSource : public sf::priv::MiniaudioSoundSource
{
public:
    void setPlayingOffset(sf::Time) override
    {
    }

    [[nodiscard]] sf::priv::MiniaudioUtils::SoundBase& getSoundBase() const override
    {
        return soundBase;
    }

    sf::priv::MiniaudioUtils::SoundBase soundBase; //!< Sound base, needs to be first member
};

} // namespace

    #define SFML_TEST_CHECK_DEFAULT_VALUES(xSoundSource)                      \
        CHECK((xSoundSource).getPitch() == 1.f);                              \
        CHECK((xSoundSource).getPan() == 0.f);                                \
        CHECK((xSoundSource).getVolume() == 1.f);                             \
        CHECK((xSoundSource).isSpatializationEnabled());                      \
        CHECK((xSoundSource).getPosition() == sf::Vec3{0.f, 0.f, 0.f});       \
        CHECK((xSoundSource).getDirection() == sf::Vec3f{0.f, 0.f, -1.f});    \
        CHECK((xSoundSource).getCone().innerAngle == sf::radians(6.283185f)); \
        CHECK((xSoundSource).getCone().outerAngle == sf::radians(6.283185f)); \
        CHECK((xSoundSource).getCone().outerGain == 1.f);                     \
        CHECK((xSoundSource).getVelocity() == sf::Vec3f{});                   \
        CHECK((xSoundSource).getDopplerFactor() == 1.f);                      \
        CHECK((xSoundSource).getDirectionalAttenuationFactor() == 1.f);       \
        CHECK(!(xSoundSource).isRelativeToListener());                        \
        CHECK((xSoundSource).getMinDistance() == 1.f);                        \
        CHECK((xSoundSource).getMaxDistance() == SFML_BASE_FLOAT_MAX);        \
        CHECK((xSoundSource).getMinGain() == 0);                              \
        CHECK((xSoundSource).getMaxGain() == 1.f);                            \
        CHECK((xSoundSource).getAttenuation() == 1.f);                        \
        CHECK((xSoundSource).getStatus() == sf::priv::MiniaudioSoundSource::Status::Stopped);

TEST_CASE("[Audio] sf::priv::MiniaudioSoundSource" * doctest::skip(skipAudioDeviceTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::priv::MiniaudioSoundSource));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::priv::MiniaudioSoundSource));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::priv::MiniaudioSoundSource));
        STATIC_CHECK(!SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::priv::MiniaudioSoundSource));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::priv::MiniaudioSoundSource));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::priv::MiniaudioSoundSource));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::priv::MiniaudioSoundSource));
    }

    SECTION("Construction")
    {
        const TestSoundSource soundSource;
        SFML_TEST_CHECK_DEFAULT_VALUES(soundSource);
    }

    SECTION("Copy semantics")
    {
        const TestSoundSource soundSource;

        SECTION("Construction")
        {
            const TestSoundSource soundSourceCopy(soundSource); // NOLINT(performance-unnecessary-copy-initialization)
            SFML_TEST_CHECK_DEFAULT_VALUES(soundSourceCopy);
        }

        SECTION("Assignment")
        {
            TestSoundSource soundSourceCopy;
            soundSourceCopy = soundSource;
            SFML_TEST_CHECK_DEFAULT_VALUES(soundSourceCopy);
        }
    }

    SECTION("Set/get pitch")
    {
        TestSoundSource soundSource;
        soundSource.setPitch(42.f);
        CHECK(soundSource.getPitch() == 42.f);
    }

    SECTION("Set/get pan")
    {
        TestSoundSource soundSource;

        soundSource.setPan(1.f);
        CHECK(soundSource.getPan() == 1.f);
    }

    SECTION("Set/get volume")
    {
        TestSoundSource soundSource;

        soundSource.setVolume(0.f);
        CHECK(soundSource.getVolume() == 0.f);

        soundSource.setVolume(0.5f);
        CHECK(soundSource.getVolume() == 0.5f);

        soundSource.setVolume(1.f);
        CHECK(soundSource.getVolume() == 1.f);
    }

    SECTION("Set/get spatialization enabled")
    {
        TestSoundSource soundSource;

        soundSource.setSpatializationEnabled(true);
        CHECK(soundSource.isSpatializationEnabled());
    }

    SECTION("Set/get position")
    {
        TestSoundSource soundSource;

        soundSource.setPosition({1, 2, 3});
        CHECK(soundSource.getPosition() == sf::Vec3f{1, 2, 3});
    }

    SECTION("Set/get direction")
    {
        TestSoundSource soundSource;

        soundSource.setDirection({4, 5, 6});
        CHECK(soundSource.getDirection() == sf::Vec3f{4, 5, 6});
    }

    SECTION("Set/get cone")
    {
        TestSoundSource soundSource;

        soundSource.setCone({sf::radians(1), sf::radians(2), 3});
        CHECK(soundSource.getCone().innerAngle == sf::radians(1));
        CHECK(soundSource.getCone().outerAngle == sf::radians(2));
        CHECK(soundSource.getCone().outerGain == 3);
    }

    SECTION("Set/get velocity")
    {
        TestSoundSource soundSource;

        soundSource.setVelocity({7, 8, 9});
        CHECK(soundSource.getVelocity() == sf::Vec3f{7, 8, 9});
    }

    SECTION("Set/get doppler factor")
    {
        TestSoundSource soundSource;

        soundSource.setDopplerFactor(1);
        CHECK(soundSource.getDopplerFactor() == 1);
    }

    SECTION("Set/get directional attenuation factor")
    {
        TestSoundSource soundSource;

        soundSource.setDirectionalAttenuationFactor(1);
        CHECK(soundSource.getDirectionalAttenuationFactor() == 1);
    }

    SECTION("Set/get relative to listener")
    {
        TestSoundSource soundSource;

        soundSource.setRelativeToListener(true);
        CHECK(soundSource.isRelativeToListener());
    }

    SECTION("Set/get min distance")
    {
        TestSoundSource soundSource;

        soundSource.setMinDistance(12.34f);
        CHECK(soundSource.getMinDistance() == 12.34f);
    }

    SECTION("Set/get max distance")
    {
        TestSoundSource soundSource;

        soundSource.setMaxDistance(12.34f);
        CHECK(soundSource.getMaxDistance() == 12.34f);
    }

    SECTION("Set/get min gain")
    {
        TestSoundSource soundSource;

        soundSource.setMinGain(12.34f);
        CHECK(soundSource.getMinGain() == 12.34f);
    }

    SECTION("Set/get max gain")
    {
        TestSoundSource soundSource;
        soundSource.setMaxGain(12.34f);
        CHECK(soundSource.getMaxGain() == 12.34f);
    }

    SECTION("Set/get attenuation")
    {
        TestSoundSource soundSource;

        soundSource.setAttenuation(10);
        CHECK(soundSource.getAttenuation() == 10);
    }
}

#endif
