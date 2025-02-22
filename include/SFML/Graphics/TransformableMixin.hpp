#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/TransformableMacros.hpp" // used, exposes macros
#include "SFML/Graphics/Transform.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API TransformableMixinBase
{
    [[nodiscard, gnu::pure, gnu::flatten]] Transform getTransform(
        Vector2f position,
        Vector2f scale,
        Vector2f origin,
        float    radians) const;
};

////////////////////////////////////////////////////////////
/// \brief Decomposed transform defined by a position, a rotation and a scale
///
////////////////////////////////////////////////////////////
template <typename T>
struct SFML_GRAPHICS_API TransformableMixin : TransformableMixinBase
{
    ////////////////////////////////////////////////////////////
    /// \brief Scale the object
    ///
    /// This function multiplies the current scale of the object,
    /// unlike `setScale` which overwrites it.
    /// Thus, it is equivalent to the following code:
    /// \code
    /// sf::Vector2f scale = object.scale;
    /// object.scale = scale.x * factor.x, scale.y * factor.y;
    /// \endcode
    ///
    /// \param factor Scale factors
    ///
    /// \see `setScale`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void scaleBy(Vector2f factor)
    {
        static_cast<T&>(*this).scale.x *= factor.x;
        static_cast<T&>(*this).scale.y *= factor.y;
    }

    ////////////////////////////////////////////////////////////
    /// \brief get the combined transform of the object
    ///
    /// \return Transform combining the position/rotation/scale/origin of the object
    ///
    /// \see `getInverseTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] Transform getTransform() const
    {
        return TransformableMixinBase::getTransform(static_cast<const T&>(*this).position,
                                                    static_cast<const T&>(*this).scale,
                                                    static_cast<const T&>(*this).origin,
                                                    static_cast<const T&>(*this).rotation.asRadians());
    }

    ////////////////////////////////////////////////////////////
    /// \brief get the inverse of the combined transform of the object
    ///
    /// \return Inverse of the combined transformations applied to the object
    ///
    /// \see `getTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] Transform getInverseTransform() const
    {
        return getTransform().getInverse();
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TransformableMixin
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Transform`
///
////////////////////////////////////////////////////////////
