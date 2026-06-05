#include "AudioUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Audio/ChannelMap.hpp"

#include "Zancle/Audio/SoundChannel.hpp"

#include "Zancle/Trait/HasVirtualDestructor.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Audio] za::ChannelMap" * tst::skip(skipAudioDeviceTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::ChannelMap));
        STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::ChannelMap));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::ChannelMap));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::ChannelMap));
        STATIC_CHECK(!ZA_HAS_VIRTUAL_DESTRUCTOR(za::ChannelMap));
    }

    SECTION("Permutation check")
    {
        const za::ChannelMap cm0{za::SoundChannel::FrontLeft, za::SoundChannel::FrontRight};
        const za::ChannelMap cm1{za::SoundChannel::FrontRight, za::SoundChannel::FrontLeft};

        CHECK(cm0.getSize() == 2);
        CHECK(cm1.getSize() == 2);

        CHECK(cm0.isPermutationOf(cm0));
        CHECK(cm0.isPermutationOf(cm1));
        CHECK(cm1.isPermutationOf(cm0));
        CHECK(cm1.isPermutationOf(cm1));

        za::ChannelMap cm2;
        cm2 = cm0;

        CHECK(cm2.isPermutationOf(cm0));
        CHECK(cm2.isPermutationOf(cm1));
        CHECK(cm2.isPermutationOf(cm2));
    }


    SECTION("Assignment from init list")
    {
        za::ChannelMap cm0;

        cm0 = {za::SoundChannel::FrontLeft, za::SoundChannel::FrontRight};

        CHECK(cm0.getSize() == 2);
        CHECK(cm0[0] == za::SoundChannel::FrontLeft);
        CHECK(cm0[1] == za::SoundChannel::FrontRight);
    }
}
