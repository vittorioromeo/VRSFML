#include "SFML/Audio/AudioDeviceHandle.hpp"

#include <CommonTraits.hpp>

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
