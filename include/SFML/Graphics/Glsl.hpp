#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
template <base::SizeT Columns, base::SizeT Rows>
struct [[nodiscard]] Matrix;

template <typename T>
struct [[nodiscard]] Vec4;
} // namespace sf::priv


////////////////////////////////////////////////////////////
/// \brief Namespace with GLSL types
///
////////////////////////////////////////////////////////////
namespace sf::Glsl
{
////////////////////////////////////////////////////////////
/// \brief 2D float vector (\p vec2 in GLSL)
///
////////////////////////////////////////////////////////////
using Vec2 = sf::Vec2<float>;

////////////////////////////////////////////////////////////
/// \brief 2D int vector (\p ivec2 in GLSL)
///
////////////////////////////////////////////////////////////
using Ivec2 = sf::Vec2<int>;

////////////////////////////////////////////////////////////
/// \brief 2D bool vector (\p bvec2 in GLSL)
///
////////////////////////////////////////////////////////////
using Bvec2 = sf::Vec2<bool>;

////////////////////////////////////////////////////////////
/// \brief 3D float vector (\p vec3 in GLSL)
///
////////////////////////////////////////////////////////////
using Vec3 = sf::Vec3<float>;

////////////////////////////////////////////////////////////
/// \brief 3D int vector (\p ivec3 in GLSL)
///
////////////////////////////////////////////////////////////
using Ivec3 = sf::Vec3<int>;

////////////////////////////////////////////////////////////
/// \brief 3D bool vector (\p bvec3 in GLSL)
///
////////////////////////////////////////////////////////////
using Bvec3 = sf::Vec3<bool>;

#ifdef SFML_DOXYGEN

////////////////////////////////////////////////////////////
/// \brief 4D float vector (\p vec4 in GLSL)
///
/// 4D float vectors can be implicitly converted from `sf::Color`
/// instances. Each color channel is normalized from integers
/// in [0, 255] to floating point values in [0, 1].
/// \code
/// sf::Glsl::Vec4 zeroVec;
/// sf::Glsl::Vec4 vector(1.f, 2.f, 3.f, 4.f);
/// sf::Glsl::Vec4 color = sf::Color::Cyan;
/// \endcode
////////////////////////////////////////////////////////////
using Vec4 = ImplementationDefined;

////////////////////////////////////////////////////////////
/// \brief 4D int vector (\p ivec4 in GLSL)
///
/// 4D int vectors can be implicitly converted from `sf::Color`
/// instances. Each color channel remains unchanged inside
/// the integer interval [0, 255].
/// \code
/// sf::Glsl::Ivec4 zeroVec;
/// sf::Glsl::Ivec4 vector(1, 2, 3, 4);
/// sf::Glsl::Ivec4 color = sf::Color::Cyan;
/// \endcode
////////////////////////////////////////////////////////////
using Ivec4 = ImplementationDefined;

////////////////////////////////////////////////////////////
/// \brief 4D bool vector (\p bvec4 in GLSL)
///
////////////////////////////////////////////////////////////
using Bvec4 = ImplementationDefined;

////////////////////////////////////////////////////////////
/// \brief 3x3 float matrix (\p mat3 in GLSL)
///
/// The matrix can be constructed from an array with 3x3
/// elements, aligned in column-major order. For example,
/// a translation by (x, y) looks as follows:
/// \code
/// float array[9] =
/// {
///     1, 0, 0,
///     0, 1, 0,
///     x, y, 1
/// };
///
/// sf::Glsl::Mat3 matrix(array);
/// \endcode
///
/// Mat3 can also be implicitly converted from `sf::Transform`:
/// \code
/// sf::Transform transform;
/// sf::Glsl::Mat3 matrix = transform;
/// \endcode
////////////////////////////////////////////////////////////
using Mat3 = ImplementationDefined;

////////////////////////////////////////////////////////////
/// \brief 4x4 float matrix (\p mat4 in GLSL)
///
/// The matrix can be constructed from an array with 4x4
/// elements, aligned in column-major order. For example,
/// a translation by (x, y, z) looks as follows:
/// \code
/// float array[16] =
/// {
///     1, 0, 0, 0,
///     0, 1, 0, 0,
///     0, 0, 1, 0,
///     x, y, z, 1
/// };
///
/// sf::Glsl::Mat4 matrix(array);
/// \endcode
///
/// Mat4 can also be implicitly converted from `sf::Transform`:
/// \code
/// sf::Transform transform;
/// sf::Glsl::Mat4 matrix = transform;
/// \endcode
////////////////////////////////////////////////////////////
using Mat4 = ImplementationDefined;

#else // SFML_DOXYGEN

using Vec4  = priv::Vec4<float>;
using Ivec4 = priv::Vec4<int>;
using Bvec4 = priv::Vec4<bool>;
using Mat3  = priv::Matrix<3, 3>;
using Mat4  = priv::Matrix<4, 4>;

#endif // SFML_DOXYGEN

} // namespace sf::Glsl

#include "SFML/Graphics/Glsl.inl"


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
