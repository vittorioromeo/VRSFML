#include "Zancle/Audio/AudioDeviceHandle.hpp"
#include "ZancleBase/Trait/HasVirtualDestructor.hpp"
#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsMoveAssignable.hpp"
#include "ZancleBase/Trait/IsMoveConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"


static_assert(!ZB_HAS_VIRTUAL_DESTRUCTOR(za::priv::AudioDeviceHandle));
static_assert(!ZB_IS_TRIVIAL(za::priv::AudioDeviceHandle));
static_assert(!ZB_IS_AGGREGATE(za::priv::AudioDeviceHandle));

static_assert(ZB_IS_COPY_ASSIGNABLE(za::priv::AudioDeviceHandle));
static_assert(ZB_IS_COPY_CONSTRUCTIBLE(za::priv::AudioDeviceHandle));
static_assert(ZB_IS_MOVE_ASSIGNABLE(za::priv::AudioDeviceHandle));
static_assert(ZB_IS_MOVE_CONSTRUCTIBLE(za::priv::AudioDeviceHandle));
static_assert(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::priv::AudioDeviceHandle));
static_assert(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::priv::AudioDeviceHandle));
static_assert(ZB_IS_STANDARD_LAYOUT(za::priv::AudioDeviceHandle));

static_assert(!ZB_IS_TRIVIALLY_ASSIGNABLE(za::priv::AudioDeviceHandle, za::priv::AudioDeviceHandle));
static_assert(!ZB_IS_TRIVIALLY_COPYABLE(za::priv::AudioDeviceHandle));
static_assert(!ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::priv::AudioDeviceHandle));
