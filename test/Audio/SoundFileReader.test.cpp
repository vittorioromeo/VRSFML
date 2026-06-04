#include "Tst/Tst.hpp"
#include "Zancle/Audio/SoundFileReader.hpp"
#include "ZancleBase/Trait/HasVirtualDestructor.hpp"
#include "ZancleBase/Trait/IsConstructible.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Audio] za::SoundFileReader")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_CONSTRUCTIBLE(za::SoundFileReader));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::SoundFileReader));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::SoundFileReader));
        STATIC_CHECK(!ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::SoundFileReader));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::SoundFileReader));
        STATIC_CHECK(ZB_HAS_VIRTUAL_DESTRUCTOR(za::SoundFileReader));
    }

    SECTION("Info")
    {
        const za::SoundFileReader::Info info{};
        CHECK(info.sampleCount == 0);
        CHECK(info.channelMap.getSize() == 0);
        CHECK(info.sampleRate == 0);
    }
}
