#include "SFML/Audio/SoundFileWriter.hpp"

#include "SFML/Base/Trait/HasVirtualDestructor.hpp"
#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"


static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundFileWriter));
static_assert(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundFileWriter));
static_assert(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundFileWriter));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundFileWriter));
static_assert(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundFileWriter));
static_assert(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::SoundFileWriter));
