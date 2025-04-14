#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/AutoWrapAngle.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/ClampMacro.hpp"
#include "SFML/Base/FastSinCos.hpp"


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
    struct [[nodiscard]] ScissorRect : FloatRect
    {
        ////////////////////////////////////////////////////////////
        [[nodiscard, gnu::always_inline]] constexpr ScissorRect(Vector2f thePosition, Vector2f theSize) :
        FloatRect{thePosition, theSize}
        {
            SFML_BASE_ASSERT(position.x >= 0.f && position.x <= 1.f && "position.x must lie within [0, 1]");
            SFML_BASE_ASSERT(position.y >= 0.f && position.y <= 1.f && "position.y must lie within [0, 1]");
            SFML_BASE_ASSERT(size.x >= 0.f && "size.x must lie within [0, 1]");
            SFML_BASE_ASSERT(size.y >= 0.f && "size.y must lie within [0, 1]");
            SFML_BASE_ASSERT(position.x + size.x <= 1.f && "position.x + size.x must lie within [0, 1]");
            SFML_BASE_ASSERT(position.y + size.y <= 1.f && "position.y + size.y must lie within [0, 1]");
        }

        ////////////////////////////////////////////////////////////
        [[nodiscard, gnu::always_inline]] constexpr explicit(false) ScissorRect(const FloatRect& rect) :
        ScissorRect{rect.position, rect.size}
        {
        }

        ////////////////////////////////////////////////////////////
        [[nodiscard]] static constexpr ScissorRect fromRectClamped(sf::FloatRect rect)
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
    [[nodiscard, gnu::const]] static constexpr View fromRect(const FloatRect& rectangle)
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
        const auto [sine, cosine] = base::fastSinCos(angle);

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
    /// \brief Compare strict equality between two `View` objects
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_GRAPHICS_API constexpr bool operator==(const View& rhs) const = default;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vector2f center{500.f, 500.f}; //!< Center of the view, in scene coordinates
    Vector2f size{1000.f, 1000.f}; //!< Size of the view, in scene coordinates

    // NOLINTNEXTLINE(readability-redundant-member-init)
    AutoWrapAngle rotation{}; //!< Angle of rotation of the view rectangle

    FloatRect viewport{{0.f, 0.f}, {1.f, 1.f}}; //!< Viewport rectangle, expressed as a factor of the render-target's size
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
