#include <SFML/Audio/OutputSoundFile.hpp>

#include <CommonTraits.hpp>

static_assert(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::OutputSoundFile));
static_assert(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::OutputSoundFile));
static_assert(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::OutputSoundFile));
static_assert(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::OutputSoundFile));
static_assert(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::OutputSoundFile));
