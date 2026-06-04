#include "Zancle/Audio/SoundBufferRecorder.hpp"

#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


static_assert(!ZB_IS_COPY_CONSTRUCTIBLE(za::SoundBufferRecorder));
static_assert(!ZB_IS_COPY_ASSIGNABLE(za::SoundBufferRecorder));
static_assert(!ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::SoundBufferRecorder));
static_assert(!ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::SoundBufferRecorder));
