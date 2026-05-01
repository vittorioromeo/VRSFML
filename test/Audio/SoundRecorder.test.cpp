#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"


static_assert(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundRecorder));
static_assert(!SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundRecorder));
