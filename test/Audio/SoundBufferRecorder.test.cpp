#include "Zancle/Audio/SoundBufferRecorder.hpp"

#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


static_assert(!ZA_IS_COPY_CONSTRUCTIBLE(za::SoundBufferRecorder));
static_assert(!ZA_IS_COPY_ASSIGNABLE(za::SoundBufferRecorder));
static_assert(!ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::SoundBufferRecorder));
static_assert(!ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::SoundBufferRecorder));
