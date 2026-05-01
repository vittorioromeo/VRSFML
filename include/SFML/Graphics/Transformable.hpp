#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/TransformableMacros.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Standalone aggregate that exposes the standard transformable members
///
/// `sf::Transformable` is a flat, non-polymorphic struct that
/// holds the position, scale, origin, and rotation members
/// (defined by `SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS`) plus
/// the helper methods provided by `TransformableMixin` (such as
/// `getTransform`, `getInverseTransform`, ...).
///
/// Use it as a member of your own types when you want to
/// expose the same transform-management API as the built-in
/// drawables, without the burden of inheriting from a virtual
/// base class.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API Transformable : TransformableMixin
{
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Transformable
/// \ingroup graphics
///
/// This class is provided for convenience, on top of `sf::Transform`.
///
/// `sf::Transform`, as a low-level class, offers a great level of
/// flexibility but it is not always convenient to manage. Indeed,
/// one can easily combine any kind of operation, such as a translation
/// followed by a rotation followed by a scaling, but once the result
/// transform is built, there's no way to go backward and, let's say,
/// change only the rotation without modifying the translation and scaling.
/// The entire transform must be recomputed, which means that you
/// need to retrieve the initial translation and scale factors as
/// well, and combine them the same way you did before updating the
/// rotation. This is a tedious operation, and it requires to store
/// all the individual components of the final transform.
///
/// That's exactly what `sf::Transformable` was written for: it hides
/// these variables and the composed transform behind an easy to use
/// interface. You can set or get any of the individual components
/// without worrying about the others. It also provides the composed
/// transform (as a `sf::Transform`), and keeps it up-to-date.
///
/// In addition to the position, rotation and scale, `sf::Transformable`
/// provides an "origin" component, which represents the local origin
/// of the three other components. Take an example with a 10x10
/// pixels sprite. By default, the sprite is positioned/rotated/scaled
/// relative to its top-left corner, because it is the local point
/// (0, 0). But if we change the origin to be (5, 5), the sprite will
/// be positioned/rotated/scaled around its center instead. And if
/// we set the origin to (10, 10), it will be transformed around its
/// bottom-right corner.
///
/// To keep the `sf::Transformable` class simple, there's only one
/// origin for all the components. You cannot position the sprite
/// relative to its top-left corner while rotating it around its
/// center, for example. To do such things, use `sf::Transform` directly.
///
/// `sf::Transformable` can be used as a **non-polymorphic** base
/// class -- VRSFML deliberately removes virtual interfaces from
/// the drawable hierarchy.
/// \code
/// struct MyEntity : sf::Transformable
/// {
///     void draw(sf::RenderTarget& target, sf::RenderStates states) const
///     {
///         states.transform *= getTransform();
///         target.draw(/* vertex data */, states);
///     }
/// };
///
/// MyEntity entity;
/// entity.position = {10.f, 20.f};
/// entity.rotation = sf::degrees(45.f);
/// window.draw(entity);
/// \endcode
///
/// It can also be embedded as a member, if you do not want to
/// expose its public members directly (e.g. because your code
/// uses different naming conventions):
/// \code
/// class MyEntity
/// {
/// public:
///     void setPosition(const MyVec2& v)
///     {
///         myTransform.position = {v.x(), v.y()};
///     }
///
///     void draw(sf::RenderTarget& target) const
///     {
///         target.draw(/* vertex data */, {.transform = myTransform.getTransform()});
///     }
///
/// private:
///     sf::Transformable myTransform;
/// };
/// \endcode
///
/// A note on coordinates and undistorted rendering: by default,
/// the GPU may interpolate drawable objects when rendering.
/// While this enables smooth transitions for moving or rotating
/// content, it can lead to unwanted blur or distortion in some
/// cases. To render an entity pixel-perfectly, make sure that
/// the involved coordinates allow a 1:1 mapping between window
/// pixels and texture texels. More specifically:
/// * The object's `position`, `origin`, and `scale` have no
///   fractional part.
/// * The object's `rotation` and the view's `rotation` are
///   multiples of 90 degrees.
/// * The view's `center` and `size` have no fractional part.
///
/// \see `sf::Transform`, `sf::TransformableMixin`
///
////////////////////////////////////////////////////////////
