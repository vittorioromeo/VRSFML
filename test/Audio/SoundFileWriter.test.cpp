#include "Zancle/Audio/SoundFileWriter.hpp"

#include "ZancleBase/Trait/HasVirtualDestructor.hpp"
#include "ZancleBase/Trait/IsConstructible.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


static_assert(!ZB_IS_CONSTRUCTIBLE(za::SoundFileWriter));
static_assert(!ZB_IS_COPY_CONSTRUCTIBLE(za::SoundFileWriter));
static_assert(ZB_IS_COPY_ASSIGNABLE(za::SoundFileWriter));
static_assert(!ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::SoundFileWriter));
static_assert(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::SoundFileWriter));
static_assert(ZB_HAS_VIRTUAL_DESTRUCTOR(za::SoundFileWriter));
