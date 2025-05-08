#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/System/Angle.hpp"         // used
#include "SFML/System/AutoWrapAngle.hpp" // used
#include "SFML/System/Vec2.hpp"          // used


////////////////////////////////////////////////////////////
/// \brief Define the data members of a transformable object's settings
///
////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE                                  \
    ::sf::Vec2f position{};      /*!< Position of the object in the 2D world */               \
    ::sf::Vec2f scale{1.f, 1.f}; /*!< Scale of the object */                                  \
    ::sf::Vec2f origin{};        /*!< Origin of translation/rotation/scaling of the object */ \
    /* NOLINTNEXTLINE(readability-redundant-member-init) */                                   \
    ::sf::Angle rotation{}; /*!< Orientation of the object */                                 \
                                                                                              \
    using sfPrivSwallowSemicolon0 = void


////////////////////////////////////////////////////////////
/// \brief Define the data members of a transformable object
///
////////////////////////////////////////////////////////////
#define SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS                                                \
    ::sf::Vec2f position{};      /*!< Position of the object in the 2D world */               \
    ::sf::Vec2f scale{1.f, 1.f}; /*!< Scale of the object */                                  \
    ::sf::Vec2f origin{};        /*!< Origin of translation/rotation/scaling of the object */ \
    /* NOLINTNEXTLINE(readability-redundant-member-init) */                                   \
    ::sf::AutoWrapAngle rotation{}; /*!< Orientation of the object */                         \
                                                                                              \
    using sfPrivSwallowSemicolon1 = void
