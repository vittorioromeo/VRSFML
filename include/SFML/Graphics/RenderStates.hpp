#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/View.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Shader;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Define the states used for drawing to a `RenderTarget`
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RenderStates
{
    BlendMode   blendMode{};   //!< Blending mode
    StencilMode stencilMode{}; //!< Stencil mode

    // NOLINTNEXTLINE(readability-redundant-member-init)
    Transform transform{}; //!< Transform

    View view{}; //!< View (computed from the render target's size if unset)

    const Shader* shader{nullptr}; //!< Shader

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const RenderStates&) const = default;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderStates
/// \ingroup graphics
///
/// There are five global states that can be applied to
/// the drawn objects:
/// \li the blend mode: how pixels of the object are blended with the background
/// \li the stencil mode: how pixels of the object interact with the stencil buffer
/// \li the transform: how the object is positioned/rotated/scaled
/// \li the shader: what custom effect is applied to the object
///
/// Textures are not part of render states -- they are passed
/// explicitly as parameters to draw functions that require them.
///
/// The transform is a special case: sprites, texts and shapes
/// (and it's a good idea to do it with your own drawable classes
/// too) combine their transform with the one that is passed in the
/// RenderStates structure. So that you can use a "global" transform
/// on top of each object's transform.
///
/// Most objects, especially high-level drawables, can be drawn
/// directly without defining render states explicitly -- the
/// default set of states is ok in most cases.
/// \code
/// window.draw(sprite, texture);
/// \endcode
///
/// \see `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
