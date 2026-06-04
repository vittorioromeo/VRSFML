#include "Zancle/Audio/SoundRecorder.hpp"
#include "ZancleBase/Trait/IsConstructible.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


static_assert(!ZB_IS_CONSTRUCTIBLE(za::SoundRecorder));
static_assert(!ZB_IS_COPY_CONSTRUCTIBLE(za::SoundRecorder));
static_assert(!ZB_IS_COPY_ASSIGNABLE(za::SoundRecorder));
static_assert(!ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::SoundRecorder));
static_assert(!ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::SoundRecorder));
