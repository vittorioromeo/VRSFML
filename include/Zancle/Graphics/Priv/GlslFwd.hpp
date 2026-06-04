#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
template <typename T>
struct Vec2;

template <typename T>
struct Vec3;
} // namespace za

namespace za::priv
{
template <zb::SizeT Columns, zb::SizeT Rows>
struct [[nodiscard]] Matrix;

template <typename T>
struct [[nodiscard]] Vec4;
} // namespace za::priv


////////////////////////////////////////////////////////////
/// \brief Namespace with GLSL types
///
////////////////////////////////////////////////////////////
namespace za::Glsl
{
////////////////////////////////////////////////////////////
/// \brief 2D float vector (\p vec2 in GLSL)
///
////////////////////////////////////////////////////////////
using Vec2 = za::Vec2<float>;

////////////////////////////////////////////////////////////
/// \brief 2D int vector (\p ivec2 in GLSL)
///
////////////////////////////////////////////////////////////
using Ivec2 = za::Vec2<int>;

////////////////////////////////////////////////////////////
/// \brief 2D bool vector (\p bvec2 in GLSL)
///
////////////////////////////////////////////////////////////
using Bvec2 = za::Vec2<bool>;

////////////////////////////////////////////////////////////
/// \brief 3D float vector (\p vec3 in GLSL)
///
////////////////////////////////////////////////////////////
using Vec3 = za::Vec3<float>;

////////////////////////////////////////////////////////////
/// \brief 3D int vector (\p ivec3 in GLSL)
///
////////////////////////////////////////////////////////////
using Ivec3 = za::Vec3<int>;

////////////////////////////////////////////////////////////
/// \brief 3D bool vector (\p bvec3 in GLSL)
///
////////////////////////////////////////////////////////////
using Bvec3 = za::Vec3<bool>;

#ifdef ZA_DOXYGEN

////////////////////////////////////////////////////////////
/// \brief 4D float vector (\p vec4 in GLSL)
///
/// 4D float vectors can be implicitly converted from `za::Color`
/// instances. Each color channel is normalized from integers
/// in [0, 255] to floating point values in [0, 1].
/// \code
/// za::Glsl::Vec4 zeroVec;
/// za::Glsl::Vec4 vector(1.f, 2.f, 3.f, 4.f);
/// za::Glsl::Vec4 color = za::Color::Cyan;
/// \endcode
////////////////////////////////////////////////////////////
using Vec4 = ImplementationDefined;

////////////////////////////////////////////////////////////
/// \brief 4D int vector (\p ivec4 in GLSL)
///
/// 4D int vectors can be implicitly converted from `za::Color`
/// instances. Each color channel remains unchanged inside
/// the integer interval [0, 255].
/// \code
/// za::Glsl::Ivec4 zeroVec;
/// za::Glsl::Ivec4 vector(1, 2, 3, 4);
/// za::Glsl::Ivec4 color = za::Color::Cyan;
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
/// za::Glsl::Mat3 matrix(array);
/// \endcode
///
/// Mat3 can also be implicitly converted from `za::Transform`:
/// \code
/// za::Transform transform;
/// za::Glsl::Mat3 matrix = transform;
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
/// za::Glsl::Mat4 matrix(array);
/// \endcode
///
/// Mat4 can also be implicitly converted from `za::Transform`:
/// \code
/// za::Transform transform;
/// za::Glsl::Mat4 matrix = transform;
/// \endcode
////////////////////////////////////////////////////////////
using Mat4 = ImplementationDefined;

#else // ZA_DOXYGEN

using Vec4  = priv::Vec4<float>;
using Ivec4 = priv::Vec4<int>;
using Bvec4 = priv::Vec4<bool>;
using Mat3  = priv::Matrix<3, 3>;
using Mat4  = priv::Matrix<4, 4>;

#endif // ZA_DOXYGEN

} // namespace za::Glsl
