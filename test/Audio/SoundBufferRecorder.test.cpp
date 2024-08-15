#include <SFML/Audio/SoundBufferRecorder.hpp>

#include <CommonTraits.hpp>

static_assert(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundBufferRecorder));
static_assert(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundBufferRecorder));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundBufferRecorder));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundBufferRecorder));
