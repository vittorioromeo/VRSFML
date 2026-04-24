#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/SinCosLookup.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Decomposed transform defined by a position, a rotation and a scale
///
/// Inheriting types expose the standard transformable data members
/// (`position`, `scale`, `origin`, `rotation`) — typically via the
/// `SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS` macro. The derived type
/// is deduced at call time via C++23 explicit object parameters.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API TransformableMixin
{
    ////////////////////////////////////////////////////////////
    /// \brief Scale the object
    ///
    /// This function multiplies the current scale of the object,
    /// unlike `setScale` which overwrites it.
    /// Thus, it is equivalent to the following code:
    /// \code
    /// sf::Vec2f scale = object.scale;
    /// object.scale = scale.x * factor.x, scale.y * factor.y;
    /// \endcode
    ///
    /// \param factor Scale factors
    ///
    /// \see `setScale`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void scaleBy(this auto& self, const Vec2f factor)
    {
        self.scale.x *= factor.x;
        self.scale.y *= factor.y;
    }

    ////////////////////////////////////////////////////////////
    /// \brief get the combined transform of the object
    ///
    /// \return Transform combining the position/rotation/scale/origin of the object
    ///
    /// \see `getInverseTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    Transform getTransform(this const auto& self)
    {
        return buildTransform(self.position, self.scale, self.origin, self.rotation.asRadians());
    }

    ////////////////////////////////////////////////////////////
    /// \brief get the inverse of the combined transform of the object
    ///
    /// \return Inverse of the combined transformations applied to the object
    ///
    /// \see `getTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    Transform getInverseTransform(this const auto& self)
    {
        return self.getTransform().getInverse();
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Build a `sf::Transform` from raw position/scale/origin/rotation values
    ///
    /// Non-template to keep the per-derived-type instantiations of
    /// `getTransform` tiny: each deducing-this wrapper just forwards the
    /// four extracted scalars here.
    ///
    /// \param position World-space position
    /// \param scale    Per-axis scale factors
    /// \param origin   Origin of translation/rotation/scaling
    /// \param radians  Rotation angle, in radians
    ///
    /// \return Combined transform
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure, gnu::flatten]]
    static Transform buildTransform(const Vec2f position, const Vec2f scale, const Vec2f origin, const float radians)
    {
        const auto [sine, cosine] = base::sinCosLookup(radians);

        SFML_BASE_ASSERT_AND_ASSUME(sine >= -1.f && sine <= 1.f);
        SFML_BASE_ASSERT_AND_ASSUME(cosine >= -1.f && cosine <= 1.f);

        return Transform::fromPositionScaleOriginSinCos(position, scale, origin, sine, cosine);
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TransformableMixin
/// \ingroup graphics
///
/// `sf::TransformableMixin` is a mixin that adds transform-related
/// convenience methods to a derived type. The derived type must
/// declare the standard transformable public members (typically
/// via the `SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS` macro):
/// `position`, `scale`, `origin`, and `rotation`.
///
/// In exchange, the mixin provides:
/// \li `scaleBy(factor)`           -- multiply the current scale by `factor`,
/// \li `getTransform()`            -- return the combined `sf::Transform`,
/// \li `getInverseTransform()`     -- return the inverse of the combined transform.
///
/// VRSFML's built-in drawables (`sf::Sprite`, `sf::Shape`,
/// `sf::Text`, `sf::GlyphMappedText`, ...) all use this mixin
/// to expose a consistent transform API. Use it (or
/// `sf::Transformable`) for your own non-polymorphic drawable
/// types.
///
/// \see `sf::Transform`, `sf::Transformable`,
///      `SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS`
///
////////////////////////////////////////////////////////////
