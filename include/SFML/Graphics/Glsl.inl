#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Glsl.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Builtins/Memcpy.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
struct Transform;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Helper functions to copy `sf::Transform` to `sf::Glsl::Mat3/4`
///
////////////////////////////////////////////////////////////
void SFML_GRAPHICS_API copyMatrix(const Transform& source, Matrix<3, 3>& dest);
void SFML_GRAPHICS_API copyMatrix(const Transform& source, Matrix<4, 4>& dest);

////////////////////////////////////////////////////////////
/// \brief Copy array-based matrix with given number of elements
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline void SFML_GRAPHICS_API copyMatrix(const float* source, base::SizeT elements, float* dest)
{
    SFML_BASE_MEMCPY(dest, source, elements * sizeof(float));
}

////////////////////////////////////////////////////////////
/// \brief Matrix type, used to set uniforms in GLSL
///
////////////////////////////////////////////////////////////
template <base::SizeT Columns, base::SizeT Rows>
struct [[nodiscard]] Matrix
{
    ////////////////////////////////////////////////////////////
    /// \brief Construct from raw data
    ///
    /// \param pointer Points to the beginning of an array that
    ///                has the size of the matrix. The elements
    ///                are copied to the instance.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] explicit Matrix(const float* pointer)
    {
        copyMatrix(pointer, Columns * Rows, array);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Construct implicitly from SFML transform
    ///
    /// This constructor is only supported for 3x3 and 4x4
    /// matrices.
    ///
    /// \param transform Object containing a transform.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Matrix(const Transform& transform)
    {
        copyMatrix(transform, *this);
    }

    float array[Columns * Rows]{}; //!< Array holding matrix data
};

////////////////////////////////////////////////////////////
/// \brief 4D vector type, used to set uniforms in GLSL
///
////////////////////////////////////////////////////////////
template <typename T>
struct [[nodiscard]] Vector4
{
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor, creates a zero vector
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr Vector4() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from 4 vector components
    ///
    /// \param x Component of the 4D vector
    /// \param y Component of the 4D vector
    /// \param z Component of the 4D vector
    /// \param w Component of the 4D vector
    ///
    ////////////////////////////////////////////////////////////
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
    [[nodiscard, gnu::always_inline]] constexpr Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w)
    {
    }
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    ////////////////////////////////////////////////////////////
    /// \brief Converts the vector to another type of vector
    ///
    ////////////////////////////////////////////////////////////
    template <typename U>
    [[nodiscard, gnu::always_inline]] constexpr explicit Vector4(const Vector4<U>& other) :
    x(static_cast<T>(other.x)),
    y(static_cast<T>(other.y)),
    z(static_cast<T>(other.z)),
    w(static_cast<T>(other.w))
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Construct vector implicitly from color
    ///
    /// Vector is normalized to [0, 1] for floats, and left as-is
    /// for ints. Not defined for other template arguments.
    ///
    /// \param color Color instance
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr Vector4(Color color);

    T x{}; //!< 1st component (X) of the 4D vector
    T y{}; //!< 2nd component (Y) of the 4D vector
    T z{}; //!< 3rd component (Z) of the 4D vector
    T w{}; //!< 4th component (W) of the 4D vector
};


////////////////////////////////////////////////////////////
template <>
constexpr Vector4<float>::Vector4(Color color) :
x(color.r / 255.f),
y(color.g / 255.f),
z(color.b / 255.f),
w(color.a / 255.f)
{
}


////////////////////////////////////////////////////////////
template <>
constexpr Vector4<int>::Vector4(Color color) : x(color.r), y(color.g), z(color.b), w(color.a)
{
}

} // namespace sf::priv
