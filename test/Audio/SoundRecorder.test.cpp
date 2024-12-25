#include "SFML/Audio/SoundRecorder.hpp"

#include <CommonTraits.hpp>

static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundRecorder));
