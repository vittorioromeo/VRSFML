#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Transform.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Shader;
class Texture;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Define the states used for drawing to a `RenderTarget`
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RenderStates
{
    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const RenderStates Default; //!< Special instance holding the default render states

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    BlendMode   blendMode{BlendAlpha}; //!< Blending mode
    StencilMode stencilMode{};         //!< Stencil mode

    // NOLINTNEXTLINE(readability-redundant-member-init)
    Transform transform{}; //!< Transform

    const Texture* texture{}; //!< Texture
    const Shader*  shader{};  //!< Shader
};

inline constexpr RenderStates RenderStates::Default{.blendMode = BlendAlpha};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderStates
/// \ingroup graphics
///
/// There are six global states that can be applied to
/// the drawn objects:
/// \li the blend mode: how pixels of the object are blended with the background
/// \li the stencil mode: how pixels of the object interact with the stencil buffer
/// \li the transform: how the object is positioned/rotated/scaled
/// \li the texture: what image is mapped to the object
/// \li the shader: what custom effect is applied to the object
///
/// High-level objects such as sprites or text force some of
/// these states when they are drawn. For example, a sprite
/// will set its own texture, so that you don't have to care
/// about it when drawing the sprite.
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
/// window.draw(sprite);
/// \endcode
///
/// If you want to use a single specific render state,
/// for example a shader, you can pass it directly to the Draw
/// function: `sf::RenderStates` has an implicit one-argument
/// constructor for each state.
/// \code
/// window.draw(sprite, shader);
/// \endcode
///
/// When you're inside the `draw` function of a drawable
/// object, you can either pass the render states unmodified,
/// or change some of them.
/// For example, a transformable object will combine the
/// current transform with its own transform. A sprite will
/// set its texture. Etc.
///
/// \see `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
