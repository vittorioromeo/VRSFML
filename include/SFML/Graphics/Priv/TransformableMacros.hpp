#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/System/Angle.hpp"         // used
#include "SFML/System/AutoWrapAngle.hpp" // used
#include "SFML/System/Vector2.hpp"       // used


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE                                     \
    ::sf::Vector2f position{};      /*!< Position of the object in the 2D world */               \
    ::sf::Vector2f scale{1.f, 1.f}; /*!< Scale of the object */                                  \
    ::sf::Vector2f origin{};        /*!< Origin of translation/rotation/scaling of the object */ \
    /* NOLINTNEXTLINE(readability-redundant-member-init) */                                      \
    ::sf::Angle rotation{}; /*!< Orientation of the object */                                    \
    using sfPrivSwallowSemicolon0 = void


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
#define SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS                                                   \
    ::sf::Vector2f position{};      /*!< Position of the object in the 2D world */               \
    ::sf::Vector2f scale{1.f, 1.f}; /*!< Scale of the object */                                  \
    ::sf::Vector2f origin{};        /*!< Origin of translation/rotation/scaling of the object */ \
    /* NOLINTNEXTLINE(readability-redundant-member-init) */                                      \
    ::sf::AutoWrapAngle rotation{}; /*!< Orientation of the object */                            \
    using sfPrivSwallowSemicolon1 = void
