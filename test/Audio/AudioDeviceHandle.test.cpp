#include "SFML/Audio/AudioDeviceHandle.hpp"

#include "SFML/Base/Trait/HasVirtualDestructor.hpp"
#include "SFML/Base/Trait/IsAggregate.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsMoveAssignable.hpp"
#include "SFML/Base/Trait/IsMoveConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"
#include "SFML/Base/Trait/IsStandardLayout.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"


static_assert(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::priv::AudioDeviceHandle));
static_assert(!SFML_BASE_IS_TRIVIAL(sf::priv::AudioDeviceHandle));
static_assert(!SFML_BASE_IS_AGGREGATE(sf::priv::AudioDeviceHandle));

static_assert(SFML_BASE_IS_COPY_ASSIGNABLE(sf::priv::AudioDeviceHandle));
static_assert(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::priv::AudioDeviceHandle));
static_assert(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::priv::AudioDeviceHandle));
static_assert(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::priv::AudioDeviceHandle));
static_assert(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::priv::AudioDeviceHandle));
static_assert(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::priv::AudioDeviceHandle));
static_assert(SFML_BASE_IS_STANDARD_LAYOUT(sf::priv::AudioDeviceHandle));

static_assert(!SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::priv::AudioDeviceHandle, sf::priv::AudioDeviceHandle));
static_assert(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::priv::AudioDeviceHandle));
static_assert(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::priv::AudioDeviceHandle));
