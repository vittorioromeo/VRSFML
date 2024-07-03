#if 0 // TODO
#include <SFML/Audio/AudioResource.hpp>

#include <SFML/System/Macros.hpp>

#include <Doctest.hpp>

#include <AudioUtil.hpp>

#include <type_traits>

struct ma_engine;

namespace sf::priv
{

class AudioDevice
{
public:
    static ma_engine* getEngine();
};

} // namespace sf::priv

namespace
{

bool audioDeviceInstanceCreated()
{
    return sf::priv::AudioDevice::getEngine() != nullptr;
}

struct TestAudioResource : public sf::AudioResource
{
    TestAudioResource() = default;
};

} // namespace

TEST_CASE("[Audio] sf::AudioResource" * doctest::skip(skipAudioDeviceTests))
{
    SECTION("Type traits")
    {
        static_assert(!std::is_constructible_v<sf::AudioResource>);
        static_assert(std::is_copy_constructible_v<sf::AudioResource>);
        static_assert(std::is_copy_assignable_v<sf::AudioResource>);
        static_assert(std::is_nothrow_move_constructible_v<sf::AudioResource>);
        static_assert(std::is_nothrow_move_assignable_v<sf::AudioResource>);
    }

    SECTION("No audio resources")
    {
        CHECK(!audioDeviceInstanceCreated());
    }

    SECTION("One audio resource")
    {
        CHECK(!audioDeviceInstanceCreated());

        {
            TestAudioResource tar0;
            CHECK(audioDeviceInstanceCreated());
        }

        CHECK(!audioDeviceInstanceCreated());
    }

    SECTION("Copy-construct audio resource")
    {
        CHECK(!audioDeviceInstanceCreated());

        {
            TestAudioResource tar0;
            CHECK(audioDeviceInstanceCreated());

            auto tar1 = tar0;
            CHECK(audioDeviceInstanceCreated());
        }

        CHECK(!audioDeviceInstanceCreated());
    }

    SECTION("Move-construct audio resource")
    {
        CHECK(!audioDeviceInstanceCreated());

        {
            TestAudioResource tar0;
            CHECK(audioDeviceInstanceCreated());

            auto tar1 = SFML_MOVE(tar0);
            CHECK(audioDeviceInstanceCreated());
        }

        CHECK(!audioDeviceInstanceCreated());
    }

    SECTION("Copy-assign audio resource")
    {
        CHECK(!audioDeviceInstanceCreated());

        {
            TestAudioResource tar0;
            CHECK(audioDeviceInstanceCreated());

            TestAudioResource tar1;
            CHECK(audioDeviceInstanceCreated());

            tar0 = tar1;
            CHECK(audioDeviceInstanceCreated());
        }

        CHECK(!audioDeviceInstanceCreated());
    }

    {
        CHECK(!audioDeviceInstanceCreated());

        {
            TestAudioResource tar0;
            CHECK(audioDeviceInstanceCreated());

            TestAudioResource tar1;
            CHECK(audioDeviceInstanceCreated());

            tar0 = SFML_MOVE(tar1);
            CHECK(audioDeviceInstanceCreated());
        }

        CHECK(!audioDeviceInstanceCreated());
    }
}
#endif
