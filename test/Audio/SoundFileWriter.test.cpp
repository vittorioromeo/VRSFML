#include "Zancle/Audio/SoundFileWriter.hpp"

#include "Zancle/Trait/HasVirtualDestructor.hpp"
#include "Zancle/Trait/IsConstructible.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


static_assert(!ZA_IS_CONSTRUCTIBLE(za::SoundFileWriter));
static_assert(!ZA_IS_COPY_CONSTRUCTIBLE(za::SoundFileWriter));
static_assert(ZA_IS_COPY_ASSIGNABLE(za::SoundFileWriter));
static_assert(!ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::SoundFileWriter));
static_assert(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::SoundFileWriter));
static_assert(ZA_HAS_VIRTUAL_DESTRUCTOR(za::SoundFileWriter));
