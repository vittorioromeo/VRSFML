#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class DrawableBatch;
class GraphicsContext;
class Shader;
class Shape;
class Texture;
class Transform;
class VertexBuffer;
struct BlendMode;
struct Sprite;
struct StencilMode;
struct StencilValue;
struct Vertex;
struct View;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Base class for all render targets (window, texture, ...)
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API RenderTarget
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~RenderTarget();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget(const RenderTarget&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget& operator=(const RenderTarget&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget(RenderTarget&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget& operator=(RenderTarget&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Clear the entire target with a single color
    ///
    /// This function is usually called once every frame,
    /// to clear the previous contents of the target.
    ///
    /// \param color Fill color to use to clear the render target
    ///
    ////////////////////////////////////////////////////////////
    void clear(Color color = Color::Black);

    ////////////////////////////////////////////////////////////
    /// \brief Clear the stencil buffer to a specific value
    ///
    /// The specified value is truncated to the bit width of
    /// the current stencil buffer.
    ///
    /// \param stencilValue Stencil value to clear to
    ///
    ////////////////////////////////////////////////////////////
    void clearStencil(StencilValue stencilValue);

    ////////////////////////////////////////////////////////////
    /// \brief Clear the entire target with a single color and stencil value
    ///
    /// The specified stencil value is truncated to the bit
    /// width of the current stencil buffer.
    ///
    /// \param color        Fill color to use to clear the render target
    /// \param stencilValue Stencil value to clear to
    ///
    ////////////////////////////////////////////////////////////
    void clear(Color color, StencilValue stencilValue);

    ////////////////////////////////////////////////////////////
    /// \brief Change the current active view
    ///
    /// The view is like a 2D camera, it controls which part of
    /// the 2D scene is visible, and how it is viewed in the
    /// render target.
    /// The new view will affect everything that is drawn, until
    /// another view is set.
    /// The render target keeps its own copy of the view object,
    /// so it is not necessary to keep the original one alive
    /// after calling this function.
    /// To restore the original view of the target, you can pass
    /// the result of `getDefaultView()` to this function.
    ///
    /// \param view New view to use
    ///
    /// \see `getView`, `getDefaultView`
    ///
    ////////////////////////////////////////////////////////////
    void setView(const View& view);

    ////////////////////////////////////////////////////////////
    /// \brief Get the view currently in use in the render target
    ///
    /// \return The view object that is currently used
    ///
    /// \see `setView`, `getDefaultView`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const View& getView() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the default view of the render target
    ///
    /// The default view has the initial size of the render target,
    /// and never changes after the target has been created.
    ///
    /// \return The default view of the render target
    ///
    /// \see `setView`, `getView`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] View getDefaultView() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the viewport of a view, applied to this render target
    ///
    /// The viewport is defined in the view as a ratio, this function
    /// simply applies this ratio to the current dimensions of the
    /// render target to calculate the pixels rectangle that the viewport
    /// actually covers in the target.
    ///
    /// \param view The view for which we want to compute the viewport
    ///
    /// \return Viewport rectangle, expressed in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] IntRect getViewport(const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the scissor rectangle of a view, applied to this render target
    ///
    /// The scissor rectangle is defined in the view as a ratio. This
    /// function simply applies this ratio to the current dimensions
    /// of the render target to calculate the pixels rectangle
    /// that the scissor rectangle actually covers in the target.
    ///
    /// \param view The view for which we want to compute the scissor rectangle
    ///
    /// \return Scissor rectangle, expressed in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] IntRect getScissor(const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from target coordinates to world
    ///        coordinates, using the current view
    ///
    /// This function is an overload of the mapPixelToCoords
    /// function that implicitly uses the current view.
    /// It is equivalent to:
    /// \code
    /// target.mapPixelToCoords(point, target.getView());
    /// \endcode
    ///
    /// \param point Pixel to convert
    ///
    /// \return The converted point, in "world" coordinates
    ///
    /// \see `mapCoordsToPixel`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2f mapPixelToCoords(Vector2i point) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from target coordinates to world coordinates
    ///
    /// This function finds the 2D position that matches the
    /// given pixel of the render target. In other words, it does
    /// the inverse of what the graphics card does, to find the
    /// initial position of a rendered pixel.
    ///
    /// Initially, both coordinate systems (world units and target pixels)
    /// match perfectly. But if you define a custom view or resize your
    /// render target, this assertion is not `true` anymore, i.e. a point
    /// located at (10, 50) in your render target may map to the point
    /// (150, 75) in your 2D world -- if the view is translated by (140, 25).
    ///
    /// For render-windows, this function is typically used to find
    /// which point (or object) is located below the mouse cursor.
    ///
    /// This version uses a custom view for calculations, see the other
    /// overload of the function if you want to use the current view of the
    /// render target.
    ///
    /// \param point Pixel to convert
    /// \param view The view to use for converting the point
    ///
    /// \return The converted point, in "world" units
    ///
    /// \see `mapCoordsToPixel`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2f mapPixelToCoords(Vector2i point, const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from world coordinates to target
    ///        coordinates, using the current view
    ///
    /// This function is an overload of the `mapCoordsToPixel`
    /// function that implicitly uses the current view.
    /// It is equivalent to:
    /// \code
    /// target.mapCoordsToPixel(point, target.getView());
    /// \endcode
    ///
    /// \param point Point to convert
    ///
    /// \return The converted point, in target coordinates (pixels)
    ///
    /// \see `mapPixelToCoords`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2i mapCoordsToPixel(Vector2f point) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from world coordinates to target coordinates
    ///
    /// This function finds the pixel of the render target that matches
    /// the given 2D point. In other words, it goes through the same process
    /// as the graphics card, to compute the final position of a rendered point.
    ///
    /// Initially, both coordinate systems (world units and target pixels)
    /// match perfectly. But if you define a custom view or resize your
    /// render target, this assertion is not `true` anymore, i.e. a point
    /// located at (150, 75) in your 2D world may map to the pixel
    /// (10, 50) of your render target -- if the view is translated by (140, 25).
    ///
    /// This version uses a custom view for calculations, see the other
    /// overload of the function if you want to use the current view of the
    /// render target.
    ///
    /// \param point Point to convert
    /// \param view The view to use for converting the point
    ///
    /// \return The converted point, in target coordinates (pixels)
    ///
    /// \see `mapPixelToCoords`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2i mapCoordsToPixel(Vector2f point, const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief Draw a drawable object to the render target
    ///
    /// \param drawable Object to draw
    /// \param states   Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    template <typename DrawableObject>
    void draw(const DrawableObject& drawableObject, const RenderStates& states = RenderStates::Default)
        requires(requires { drawableObject.draw(*this, states); })
    {
        drawableObject.draw(*this, states);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Draw a sprite object to the render target
    ///
    /// The texture associated with a sprite must be passed while drawing.
    ///
    /// \param sprite  Sprite to draw
    /// \param texture Texture associated with the sprite
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Sprite& sprite, const Texture& texture, RenderStates states = RenderStates::Default);

    ////////////////////////////////////////////////////////////
    /// \brief Deleted overload of `draw` for sprites without a texture
    ///
    /// The texture associated with a sprite must be passed while drawing.
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Sprite&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Draw a shape object to the render target
    ///
    /// A texture associated with a shape can be passed while drawing.
    ///
    /// \param shape   Shape to draw
    /// \param texture Texture associated with the shape
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Shape& shape, const Texture* texture, const RenderStates& states = RenderStates::Default);

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by an array of vertices
    ///
    /// \param vertices    Pointer to the vertices
    /// \param vertexCount Number of vertices in the array
    /// \param type        Type of primitives to draw
    /// \param states      Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Vertex*       vertices,
              base::SizeT         vertexCount,
              PrimitiveType       type,
              const RenderStates& states = RenderStates::Default);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawIndexedVertices(const Vertex*       vertices,
                             base::SizeT         vertexCount,
                             const unsigned int* indices,
                             base::SizeT         indexCount,
                             PrimitiveType       type,
                             const RenderStates& states = RenderStates::Default);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P0: docs
    ///
    ////////////////////////////////////////////////////////////
    void draw(const DrawableBatch& drawableBatch, const RenderStates& renderStates);

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a contiguous container of vertices
    ///
    /// \tparam ContiguousVertexRange Type of the contiguous container,
    ///         must support `.data()` and `.size()` operations.
    ///
    /// \param vertices    Reference to the contiguous vertex container
    /// \param type        Type of primitives to draw
    /// \param states      Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    template <typename ContiguousVertexRange>
    void draw(const ContiguousVertexRange& vertices, PrimitiveType type, const RenderStates& states = RenderStates::Default)
        requires(requires { draw(vertices.data(), vertices.size(), type, states); })
    {
        draw(vertices.data(), vertices.size(), type, states);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a C-style array of vertices
    ///
    /// \param vertices    Reference to the C-style vertex array
    /// \param type        Type of primitives to draw
    /// \param states      Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    template <base::SizeT N>
    void draw(const Vertex (&vertices)[N], PrimitiveType type, const RenderStates& states = RenderStates::Default)
    {
        draw(vertices, N, type, states);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a vertex buffer
    ///
    /// \param vertexBuffer Vertex buffer
    /// \param states       Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const VertexBuffer& vertexBuffer, const RenderStates& states = RenderStates::Default);

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a vertex buffer
    ///
    /// \param vertexBuffer Vertex buffer
    /// \param firstVertex  Index of the first vertex to render
    /// \param vertexCount  Number of vertices to render
    /// \param states       Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const VertexBuffer& vertexBuffer,
              base::SizeT         firstVertex,
              base::SizeT         vertexCount,
              const RenderStates& states = RenderStates::Default);

    ////////////////////////////////////////////////////////////
    /// \brief Return the size of the rendering region of the target
    ///
    /// \return Size in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual Vector2u getSize() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the render target will use sRGB encoding when drawing on it
    ///
    /// \return `true` if the render target use sRGB encoding, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool isSrgb() const;

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the render target for rendering
    ///
    /// This function makes the render target's context current for
    /// future OpenGL rendering operations (so you shouldn't care
    /// about it if you're not doing direct OpenGL stuff).
    /// A render target's context is active only on the current thread,
    /// if you want to make it active on another thread you have
    /// to deactivate it on the previous thread first if it was active.
    /// Only one context can be current in a thread, so if you
    /// want to draw OpenGL geometry to another render target
    /// don't forget to activate it again. Activating a render
    /// target will automatically deactivate the previously active
    /// context (if any).
    ///
    /// \param active `true` to activate, `false` to deactivate
    ///
    /// \return `true` if operation was successful, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool setActive(bool active = true);

    ////////////////////////////////////////////////////////////
    /// \brief Reset the internal OpenGL states so that the target is ready for drawing
    ///
    /// This function can be used when you mix SFML drawing
    /// and direct OpenGL rendering. It makes sure that all OpenGL
    /// states needed by SFML are set, so that subsequent `draw()`
    /// calls will work as expected.
    ///
    /// Example:
    /// \code
    /// // OpenGL code here...
    /// glPushAttrib(...);
    /// window.resetGLStates();
    /// window.draw(...);
    /// window.draw(...);
    /// glPopAttrib(...);
    /// // OpenGL code here...
    /// \endcode
    ///
    ////////////////////////////////////////////////////////////
    void resetGLStates();

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor from graphics context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RenderTarget(GraphicsContext& graphicsContext);

    ////////////////////////////////////////////////////////////
    /// \brief Performs the common initialization step after creation
    ///
    /// The derived classes must call this function after the
    /// target is created and ready for drawing.
    ///
    ////////////////////////////////////////////////////////////
    void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Get the window context (used by derived types)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] GraphicsContext& getGraphicsContext();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Perform common cleaning operations prior to GL calls
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool clearImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Apply a view
    ///
    ////////////////////////////////////////////////////////////
    void applyView(const View& view);

    ////////////////////////////////////////////////////////////
    /// \brief Apply a new blending mode
    ///
    /// \param mode Blending mode to apply
    ///
    ////////////////////////////////////////////////////////////
    void applyBlendMode(const BlendMode& mode);

    ////////////////////////////////////////////////////////////
    /// \brief Apply a new stencil mode
    ///
    /// \param mode Stencil mode to apply
    ///
    ////////////////////////////////////////////////////////////
    void applyStencilMode(const StencilMode& mode);

    ////////////////////////////////////////////////////////////
    /// \brief Unbind any bound texture
    ///
    ////////////////////////////////////////////////////////////
    void unapplyTexture();

    ////////////////////////////////////////////////////////////
    /// \brief Setup environment for drawing
    ///
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void setupDraw(const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Setup environment for drawing: MVP matrix
    ///
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void setupDrawMVP(const RenderStates& states, const Transform& viewTransform);

    ////////////////////////////////////////////////////////////
    /// \brief Setup environment for drawing: texture
    ///
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void setupDrawTexture(const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Draw non-indexed primitives
    ///
    /// \param type        Type of primitives to draw
    /// \param firstVertex Index of the first vertex to use when drawing
    /// \param vertexCount Number of vertices to use when drawing
    ///
    ////////////////////////////////////////////////////////////
    void drawPrimitives(PrimitiveType type, base::SizeT firstVertex, base::SizeT vertexCount);

    ////////////////////////////////////////////////////////////
    /// \brief Draw indexed primitives
    ///
    /// \param type        Type of primitives to draw
    /// \param indexCount  Number of indices to use when drawing
    ///
    ////////////////////////////////////////////////////////////
    void drawIndexedPrimitives(PrimitiveType type, base::SizeT indexCount);

    ////////////////////////////////////////////////////////////
    /// \brief Clean up environment after drawing
    ///
    /// \param states Render states used for drawing
    ///
    ////////////////////////////////////////////////////////////
    void cleanupDraw(const RenderStates& states);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 768> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderTarget
/// \ingroup graphics
///
/// `sf::RenderTarget` defines the common behavior of all the
/// 2D render targets usable in the graphics module. It makes
/// it possible to draw 2D entities like sprites, shapes, text
/// without using any OpenGL command directly.
///
/// A `sf::RenderTarget` is also able to use views (`sf::View`),
/// which are a kind of 2D cameras. With views you can globally
/// scroll, rotate or zoom everything that is drawn,
/// without having to transform every single entity. See the
/// documentation of `sf::View` for more details and sample pieces of
/// code about this class.
///
/// On top of that, render targets are still able to render direct
/// OpenGL stuff. It is even possible to mix together OpenGL calls
/// and regular SFML drawing commands. When doing so, make sure that
/// OpenGL states are not messed up by calling `resetGLStates`.
///
/// While render targets are moveable, it is not valid to move them
/// between threads. This will cause your program to crash. The
/// problem boils down to OpenGL being limited with regard to how it
/// works in multithreaded environments. Please ensure you only move
/// render targets within the same thread.
///
/// \see `sf::RenderWindow`, `sf::RenderTexture`, `sf::View`
///
////////////////////////////////////////////////////////////
