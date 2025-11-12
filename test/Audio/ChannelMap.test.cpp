#include "SFML/Audio/ChannelMap.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>


TEST_CASE("[Audio] sf::ChannelMap" * doctest::skip(skipAudioDeviceTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::ChannelMap));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::ChannelMap));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::ChannelMap));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::ChannelMap));
        STATIC_CHECK(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::ChannelMap));
    }

    SECTION("Permutation check")
    {
        const sf::ChannelMap cm0{sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight};
        const sf::ChannelMap cm1{sf::SoundChannel::FrontRight, sf::SoundChannel::FrontLeft};

        CHECK(cm0.getSize() == 2);
        CHECK(cm1.getSize() == 2);

        CHECK(cm0.isPermutationOf(cm0));
        CHECK(cm0.isPermutationOf(cm1));
        CHECK(cm1.isPermutationOf(cm0));
        CHECK(cm1.isPermutationOf(cm1));

        sf::ChannelMap cm2;
        cm2 = cm0;

        CHECK(cm2.isPermutationOf(cm0));
        CHECK(cm2.isPermutationOf(cm1));
        CHECK(cm2.isPermutationOf(cm2));
    }


    SECTION("Assignment from init list")
    {
        sf::ChannelMap cm0;

        cm0 = {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight};

        CHECK(cm0.getSize() == 2);
        CHECK(cm0[0] == sf::SoundChannel::FrontLeft);
        CHECK(cm0[1] == sf::SoundChannel::FrontRight);
    }
}
