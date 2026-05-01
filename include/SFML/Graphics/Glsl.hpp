#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Priv/GlslFwd.hpp" // IWYU pragma: export

#include "SFML/System/Priv/Vec2Base.hpp" // IWYU pragma: export
#include "SFML/System/Vec3.hpp"          // IWYU pragma: export

#include "SFML/Base/SizeT.hpp" // IWYU pragma: export

// Implementation (must be after the forward declarations)
#include "SFML/Graphics/Glsl.inl" // IWYU pragma: export


////////////////////////////////////////////////////////////
/// \namespace sf::Glsl
/// \ingroup graphics
///
/// \details The `sf::Glsl` namespace contains types that match
/// their equivalents in GLSL, the OpenGL shading language.
/// These types are exclusively used by the `sf::Shader` class.
///
/// Types that already exist in SFML, such as `sf::Vec2<T>`
/// and `sf::Vec3<T>`, are reused as type aliases, so you
/// can use the types in this namespace as well as the original ones.
/// Others are newly defined, such as `Glsl::Vec4` or `Glsl::Mat3`.
/// Their actual type is an implementation detail and should not be used.
///
/// All vector types support a default constructor that
/// initializes every component to zero, in addition to a
/// constructor with one parameter for each component.
/// The components are stored in member variables called
/// x, y, z, and w.
///
/// All matrix types support a constructor with a `float*`
/// parameter that points to a float array of the appropriate
/// size (that is, 9 in a 3x3 matrix, 16 in a 4x4 matrix).
/// Furthermore, they can be converted from `sf::Transform`
/// objects.
///
/// \see `sf::Shader`
///
////////////////////////////////////////////////////////////
