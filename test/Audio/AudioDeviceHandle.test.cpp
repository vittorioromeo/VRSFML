#include "Zancle/Audio/AudioDeviceHandle.hpp"

#include "Zancle/Trait/HasVirtualDestructor.hpp"
#include "Zancle/Trait/IsAggregate.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsMoveAssignable.hpp"
#include "Zancle/Trait/IsMoveConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"
#include "Zancle/Trait/IsStandardLayout.hpp"
#include "Zancle/Trait/IsTrivial.hpp"
#include "Zancle/Trait/IsTriviallyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyable.hpp"
#include "Zancle/Trait/IsTriviallyDestructible.hpp"


static_assert(!ZA_HAS_VIRTUAL_DESTRUCTOR(za::priv::AudioDeviceHandle));
static_assert(!ZA_IS_TRIVIAL(za::priv::AudioDeviceHandle));
static_assert(!ZA_IS_AGGREGATE(za::priv::AudioDeviceHandle));

static_assert(ZA_IS_COPY_ASSIGNABLE(za::priv::AudioDeviceHandle));
static_assert(ZA_IS_COPY_CONSTRUCTIBLE(za::priv::AudioDeviceHandle));
static_assert(ZA_IS_MOVE_ASSIGNABLE(za::priv::AudioDeviceHandle));
static_assert(ZA_IS_MOVE_CONSTRUCTIBLE(za::priv::AudioDeviceHandle));
static_assert(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::priv::AudioDeviceHandle));
static_assert(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::priv::AudioDeviceHandle));
static_assert(ZA_IS_STANDARD_LAYOUT(za::priv::AudioDeviceHandle));

static_assert(!ZA_IS_TRIVIALLY_ASSIGNABLE(za::priv::AudioDeviceHandle, za::priv::AudioDeviceHandle));
static_assert(!ZA_IS_TRIVIALLY_COPYABLE(za::priv::AudioDeviceHandle));
static_assert(!ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::priv::AudioDeviceHandle));
