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
    BlendMode   blendMode{};   //!< Blending mode
    StencilMode stencilMode{}; //!< Stencil mode

    // NOLINTNEXTLINE(readability-redundant-member-init)
    Transform transform{}; //!< Transform

    View view{}; //!< View (computed from the render target's size if unset)

    const Texture* texture{nullptr}; //!< Texture
    const Shader*  shader{nullptr};  //!< Shader

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const RenderStates&) const = default;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::RenderStates
/// \ingroup graphics
///
/// `sf::RenderStates` is the bag of state passed to every
/// `sf::RenderTarget::draw` call. It is an aggregate of six
/// public members:
///
/// \li `blendMode`   -- how pixels of the object are blended with the background
/// \li `stencilMode` -- how pixels of the object interact with the stencil buffer
/// \li `transform`   -- how the object is positioned, rotated, and scaled
/// \li `view`        -- which view (camera) is active for this draw call
/// \li `texture`     -- which texture is sampled by the shader
/// \li `shader`      -- which custom shader runs (if any)
///
/// High-level objects such as `sf::Sprite`, `sf::Shape`, and
/// `sf::Text` may need some of these to be set up before drawing.
/// In particular, `sf::Sprite` and `sf::Shape` do not own a
/// texture in VRSFML: you must either pass the texture as a draw
/// argument or assign it to `RenderStates::texture` yourself.
///
/// The transform is special: drawables that have their own
/// `transform` (sprites, texts, shapes, ...) combine their local
/// transform with the one passed in `RenderStates`, so a "global"
/// transform composes naturally on top of every per-object
/// transform.
///
/// `sf::RenderStates` is an aggregate, so it is most often
/// constructed inline with C++20 designated initializers:
///
/// \code
/// // Default render states.
/// window.draw(sprite, texture);
///
/// // Custom shader, defaults for everything else.
/// window.draw(sprite, texture, {.shader = &myShader});
///
/// // Custom view + custom blend mode.
/// window.draw(shape, {.blendMode = sf::BlendAdd, .view = myView});
/// \endcode
///
/// \see `sf::RenderTarget`, `sf::BlendMode`, `sf::StencilMode`,
///      `sf::Transform`, `sf::View`, `sf::Shader`, `sf::Texture`
///
////////////////////////////////////////////////////////////
