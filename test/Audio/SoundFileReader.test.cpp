#include "Tst/Tst.hpp"

#include "Zancle/Audio/SoundFileReader.hpp"

#include "Zancle/Trait/HasVirtualDestructor.hpp"
#include "Zancle/Trait/IsConstructible.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Audio] za::SoundFileReader")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZA_IS_CONSTRUCTIBLE(za::SoundFileReader));
        STATIC_CHECK(!ZA_IS_COPY_CONSTRUCTIBLE(za::SoundFileReader));
        STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::SoundFileReader));
        STATIC_CHECK(!ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::SoundFileReader));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::SoundFileReader));
        STATIC_CHECK(ZA_HAS_VIRTUAL_DESTRUCTOR(za::SoundFileReader));
    }

    SECTION("Info")
    {
        const za::SoundFileReader::Info info{};
        CHECK(info.sampleCount == 0);
        CHECK(info.channelMap.getSize() == 0);
        CHECK(info.sampleRate == 0);
    }
}
