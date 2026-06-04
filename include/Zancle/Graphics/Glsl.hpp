#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Priv/GlslFwd.hpp" // IWYU pragma: export

#include "Zancle/System/Priv/Vec2Base.hpp" // IWYU pragma: export
#include "Zancle/System/Vec3.hpp"          // IWYU pragma: export

#include "ZancleBase/SizeT.hpp" // IWYU pragma: export

// Implementation (must be after the forward declarations)
#include "Zancle/Graphics/Glsl.inl" // IWYU pragma: export


////////////////////////////////////////////////////////////
/// \namespace za::Glsl
/// \ingroup graphics
///
/// \details The `za::Glsl` namespace contains types that match
/// their equivalents in GLSL, the OpenGL shading language.
/// These types are exclusively used by the `za::Shader` class.
///
/// Types that already exist in SFML, such as `za::Vec2<T>`
/// and `za::Vec3<T>`, are reused as type aliases, so you
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
/// Furthermore, they can be converted from `za::Transform`
/// objects.
///
/// \see `za::Shader`
///
////////////////////////////////////////////////////////////
