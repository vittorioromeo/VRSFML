#include <SFML/Audio/SoundFileWriter.hpp>

#include <CommonTraits.hpp>

static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundFileWriter));
static_assert(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundFileWriter));
static_assert(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundFileWriter));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundFileWriter));
static_assert(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundFileWriter));
static_assert(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::SoundFileWriter));
