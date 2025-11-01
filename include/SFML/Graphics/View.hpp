#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/AutoWrapAngle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/ClampMacro.hpp"
#include "SFML/Base/SinCosLookup.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief 2D camera that defines what region is shown on screen
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API View
{
    ////////////////////////////////////////////////////////////
    /// \brief Scissor rectangle
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] ScissorRect : Rect2f
    {
        ////////////////////////////////////////////////////////////
        [[nodiscard, gnu::always_inline]] constexpr ScissorRect(Vec2f thePosition, Vec2f theSize) :
            Rect2f{thePosition, theSize}
        {
            SFML_BASE_ASSERT(position.x >= 0.f && position.x <= 1.f && "position.x must lie within [0, 1]");
            SFML_BASE_ASSERT(position.y >= 0.f && position.y <= 1.f && "position.y must lie within [0, 1]");
            SFML_BASE_ASSERT(size.x >= 0.f && "size.x must lie within [0, 1]");
            SFML_BASE_ASSERT(size.y >= 0.f && "size.y must lie within [0, 1]");
            SFML_BASE_ASSERT(position.x + size.x <= 1.f && "position.x + size.x must lie within [0, 1]");
            SFML_BASE_ASSERT(position.y + size.y <= 1.f && "position.y + size.y must lie within [0, 1]");
        }

        ////////////////////////////////////////////////////////////
        [[nodiscard, gnu::always_inline]] constexpr explicit(false) ScissorRect(const Rect2f& rect) :
            ScissorRect{rect.position, rect.size}
        {
        }

        ////////////////////////////////////////////////////////////
        [[nodiscard]] static constexpr ScissorRect fromRectClamped(sf::Rect2f rect)
        {
            // Clamp the position to the range `[0, 1]`
            rect.position.x = SFML_BASE_CLAMP(rect.position.x, 0.f, 1.f);
            rect.position.y = SFML_BASE_CLAMP(rect.position.y, 0.f, 1.f);

            // Ensure the size is non-negative
            rect.size.x = SFML_BASE_MAX(rect.size.x, 0.f);
            rect.size.y = SFML_BASE_MAX(rect.size.y, 0.f);

            // Adjust the size so that `position + size` doesn't exceed `1`
            if (rect.position.x + rect.size.x > 1.f)
                rect.size.x = 1.f - rect.position.x;

            if (rect.position.y + rect.size.y > 1.f)
                rect.size.y = 1.f - rect.position.y;

            return ScissorRect{rect};
        }
    };

    ////////////////////////////////////////////////////////////
    /// \brief Create a view from a rectangle
    ///
    /// \param rectangle Rectangle defining the zone to display
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::const]] static constexpr View fromRect(const Rect2f& rectangle)
    {
        return {.center = rectangle.position + rectangle.size / 2.f, .size = rectangle.size};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the projection transform of the view
    ///
    /// This function is meant for internal use only.
    ///
    /// \return Projection transform defining the view
    ///
    /// \see `getInverseTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] constexpr Transform getTransform() const
    {
        // Rotation components
        const float angle         = rotation.asRadians();
        const auto [sine, cosine] = base::sinCosLookup(angle);

        const float tx = -center.x * cosine - center.y * sine + center.x;
        const float ty = center.x * sine - center.y * cosine + center.y;

        // Projection components
        const float a = 2.f / size.x;
        const float b = -2.f / size.y;
        const float c = -a * center.x;
        const float d = -b * center.y;

        // Rebuild the projection matrix
        return {a * cosine, a * sine, a * tx + c, -b * sine, b * cosine, b * ty + d};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the inverse projection transform of the view
    ///
    /// This function is meant for internal use only.
    ///
    /// \return Inverse of the projection transform defining the view
    ///
    /// \see `getTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] constexpr Transform getInverseTransform() const
    {
        return getTransform().getInverse();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Transform a 2D point from world coordinates to target (pixel) coordinates.
    ///
    /// This function is the forward projection of a camera. It takes a
    /// point in the 2D world and finds its corresponding location in
    /// absolute pixels on the render target.
    ///
    /// This can be useful for tasks such as attaching a UI element
    /// (e.g., a name tag or health bar) to a sprite in the world,
    /// by calculating its screen position every frame.
    ///
    /// This function is the inverse of `unproject`.
    ///
    /// \param point The point to transform, in world coordinates.
    /// \param targetSize The size of the render target the view is applied to.
    ///
    /// \return The transformed point, in target (pixel) coordinates.
    ///
    /// \see unproject
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Vec2f project(const Vec2f point, const Vec2f targetSize) const
    {
        // First, transform the point by the view matrix into normalized device coordinates `[-1, 1]`
        const Vec2f normalized = getTransform().transformPoint(point);

        // Then convert from normalized coordinates to absolute pixel coordinates

        // 1. Map from `[-1, 1]` to `[0, 1]` and flip Y axis
        const Vec2f relativePos = (normalized.componentWiseMul({1.f, -1.f}) + Vec2f{1.f, 1.f}) * 0.5f;

        // 2. Scale by viewport size to get position relative to viewport's origin
        const Vec2f viewportPixelPos = relativePos.componentWiseMul(viewport.size.componentWiseMul(targetSize));

        // 3. Add viewport's origin to get absolute pixel position
        const Vec2f absolutePixelPos = viewportPixelPos + viewport.position.componentWiseMul(targetSize);

        return absolutePixelPos;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Transform a 2D point from target (pixel) coordinates to world coordinates.
    ///
    /// This function is the inverse projection, also known as "unprojection".
    /// It takes a pixel coordinate on the render target and finds its
    /// corresponding location in the 2D world.
    ///
    /// This is most commonly used to translate a mouse click on the screen
    /// into a position in the game world, for object selection, character
    /// movement, etc.
    ///
    /// This function is the inverse of `project`.
    ///
    /// Usage example:
    /// \code
    /// // Create a view and a hypothetical render target size
    /// sf::View gameView({100, 100}, {200, 150}); // Centered at (100, 100), showing 200x150 world units
    /// sf::Vec2f targetSize(800, 600);
    ///
    /// // Simulate a mouse click at pixel (400, 300), the center of the screen
    /// sf::Vec2f mousePixelPos(400, 300);
    ///
    /// // Find out where that click corresponds to in the game world
    /// sf::Vec2f worldPos = gameView.unproject(mousePixelPos, targetSize);
    /// // worldPos will be approximately (100, 100), the center of the view.
    ///
    /// // Now, let's go the other way to confirm.
    /// // Where would the world origin (0, 0) appear on the screen?
    /// sf::Vec2f originPixelPos = gameView.project({0, 0}, targetSize);
    /// // originPixelPos will be approximately (0, 0), the top-left of the screen.
    /// \endcode
    ///
    /// \param point The point to transform, in target (pixel) coordinates.
    /// \param targetSize The size of the render target the view is applied to.
    ///
    /// \return The transformed point, in world coordinates.
    ///
    /// \see project
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Vec2f unproject(const Vec2f point, const Vec2f targetSize) const
    {
        // First, convert from absolute pixel coordinates to normalized device coordinates `[-1, 1]`
        const Vec2f normalized = Vec2f(-1.f, 1.f) +
                                 Vec2f(2.f, -2.f)
                                     .componentWiseMul(point - viewport.position.componentWiseMul(targetSize))
                                     .componentWiseDiv(viewport.size.componentWiseMul(targetSize));

        // Then transform by the inverse of the view matrix to get world coordinates
        return getInverseTransform().transformPoint(normalized);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compare strict equality between two `View` objects
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_GRAPHICS_API constexpr bool operator==(const View& rhs) const = default;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vec2f center{500.f, 500.f}; //!< Center of the view, in scene coordinates
    Vec2f size{1000.f, 1000.f}; //!< Size of the view, in scene coordinates

    // NOLINTNEXTLINE(readability-redundant-member-init)
    AutoWrapAngle rotation{}; //!< Angle of rotation of the view rectangle

    Rect2f viewport{{0.f, 0.f}, {1.f, 1.f}}; //!< Viewport rectangle, expressed as a factor of the render-target's size
    ScissorRect scissor{{0.f, 0.f}, {1.f, 1.f}}; //!< Scissor rectangle, expressed as a factor of the render-target's size
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::View
/// \ingroup graphics
///
/// `sf::View` defines a camera in the 2D scene. This is a
/// very powerful concept: you can scroll, rotate or zoom
/// the entire scene without altering the way that your
/// drawable objects are drawn.
///
/// A view is composed of a source rectangle, which defines
/// what part of the 2D scene is shown, and a target viewport,
/// which defines where the contents of the source rectangle
/// will be displayed on the render target (window or texture).
///
/// The viewport allows to map the scene to a custom part
/// of the render target, and can be used for split-screen
/// or for displaying a minimap, for example. If the source
/// rectangle doesn't have the same size as the viewport, its
/// contents will be stretched to fit in.
///
/// The scissor rectangle allows for specifying regions of the
/// render target to which modifications can be made by draw
/// and clear operations. Only pixels that are within the region
/// will be able to be modified. Pixels outside of the region will
/// not be modified by draw or clear operations.
///
/// Certain effects can be created by either using the viewport or
/// scissor rectangle. While the results appear identical, there
/// can be times where one method should be preferred over the other.
/// Viewport transformations are applied during the vertex processing
/// stage of the graphics pipeline, before the primitives are
/// rasterized into fragments for fragment processing. Since
/// viewport processing has to be performed and cannot be disabled,
/// effects that are performed using the viewport transform are
/// basically free performance-wise. Scissor testing is performed in
/// the per-sample processing stage of the graphics pipeline, after
/// fragment processing has been performed. Because per-sample
/// processing is performed at the last stage of the pipeline,
/// fragments that are discarded at this stage will cause the
/// highest waste of GPU resources compared to any method that
/// would have discarded vertices or fragments earlier in the
/// pipeline. There are situations in which scissor testing has
/// to be used to control whether fragments are discarded or not.
/// An example of such a situation is when performing the viewport
/// transform on vertices is necessary but a subset of the generated
/// fragments should not have an effect on the stencil buffer or
/// blend with the color buffer.
//
/// To apply a view, you have to assign it to the render target.
/// Then, objects drawn in this render target will be
/// affected by the view until you use another view.
///
/// Usage example:
/// \code
/// sf::RenderWindow window;
///
/// // Initialize the view to a rectangle located at (100, 100) and with a size of 400x200
/// sf::View view([{100, 100}, {400, 200}});
///
/// // Rotate it by 45 degrees
/// view.rotation += sf::degrees(45);
///
/// // Set its target viewport to be half of the window
/// view.setViewport([{0.f, 0.f}, {0.5f, 1.f}});
///
/// // Apply it
/// const auto defaultView = window.getView();
/// window.setView(view);
///
/// // Render stuff
/// window.draw(someSprite);
///
/// // Set the default view back
/// window.setView(defaultView);
///
/// // Render stuff not affected by the view
/// window.draw(someText);
/// \endcode
///
/// See also the note on coordinates and undistorted rendering in `sf::Transformable`.
///
/// \see `sf::RenderWindow`, `sf::RenderTexture`
///
////////////////////////////////////////////////////////////
