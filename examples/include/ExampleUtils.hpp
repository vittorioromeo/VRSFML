#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/MinMax.hpp"

#include <iostream>


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::FloatRect getAspectRatioAwareViewport(const sf::Vec2f newSize, const sf::Vec2f originalSize)
{
    SFML_BASE_ASSERT(newSize.x > 0.f && newSize.y > 0.f);
    SFML_BASE_ASSERT(originalSize.x > 0.f && originalSize.y > 0.f);

    const float contentAspectRatio = originalSize.x / originalSize.y; // Assume old size has correct ratio
    const float windowAspectRatio  = newSize.x / newSize.y;

    if (SFML_BASE_MATH_FABSF(windowAspectRatio - contentAspectRatio) < 0.01f)
    {
        // Aspect ratios match, viewport covers the whole window
        return {{0.f, 0.f}, {1.f, 1.f}};
    }

    if (windowAspectRatio > contentAspectRatio)
    {
        // Window is wider than content (pillarboxing)
        // Viewport height is 100%, width is scaled
        const float width = contentAspectRatio / windowAspectRatio;
        return {{(1.f - width) / 2.f, 0.f}, {width, 1.f}}; // Center horizontally
    }

    // Window is taller than content (letterboxing)
    // Viewport width is 100%, height is scaled
    const float height = windowAspectRatio / contentAspectRatio;
    return {{0.f, (1.f - height) / 2.f}, {1.f, height}}; // Center vertically
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline float getPixelPerfectScale(const sf::Vec2f windowSize, const sf::Vec2f nativeResolution)
{
    SFML_BASE_ASSERT(windowSize.x > 0.f && windowSize.y > 0.f);
    SFML_BASE_ASSERT(nativeResolution.x > 0.f && nativeResolution.y > 0.f);

    const auto scaleRatio = windowSize.componentWiseDiv(nativeResolution);

    // The final scale is the smaller of the two, floored to the nearest integer
    // This ensures that the scaled content fits entirely within the window
    return sf::base::max(1.f, sf::base::floor(sf::base::min(scaleRatio.x, scaleRatio.y)));
}


////////////////////////////////////////////////////////////
/// \brief Calculate a viewport for pixel-perfect integer scaling.
///
/// This function calculates the viewport rectangle needed to scale content
/// from its native resolution to the target window size using the largest
/// possible integer scaling factor, preventing pixel distortion. The
/// resulting view is centered within the window, with black bars
/// (letterboxing/pillarboxing) filling the unused space.
///
/// \param windowSize The current size of the window or render target.
/// \param nativeResolution The original, internal resolution of the content.
///
/// \return A `sf::FloatRect` defining the viewport in normalized coordinates.
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::FloatRect getPixelPerfectViewport(const sf::Vec2f windowSize, const sf::Vec2f nativeResolution)
{
    const float scale = getPixelPerfectScale(windowSize, nativeResolution);

    // Calculate the size of the viewport in normalized coordinates [0, 1]
    const float viewWidth  = (nativeResolution.x * scale) / windowSize.x;
    const float viewHeight = (nativeResolution.y * scale) / windowSize.y;

    // Calculate the position to center the viewport
    const float viewX = (1.f - viewWidth) / 2.f;
    const float viewY = (1.f - viewHeight) / 2.f;

    return {{viewX, viewY}, {viewWidth, viewHeight}};
}


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline bool handleResizeImpl(const sf::Event&  event,
                                           const sf::Vec2f   originalSize,
                                           sf::RenderWindow& renderWindow,
                                           auto&&            fnViewport)
{
    const auto* eResized = event.getIf<sf::Event::Resized>();
    if (eResized == nullptr)
        return false;

    const auto newSize = eResized->size.toVec2f();

    if (newSize.x <= 0.f || newSize.y <= 0.f)
        return true;

    auto view     = renderWindow.getView();
    view.viewport = fnViewport(newSize, originalSize);
    renderWindow.setView(view);

    return true;
}


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline bool handleAspectRatioAwareResize(const sf::Event&  event,
                                                       const sf::Vec2f   originalSize,
                                                       sf::RenderWindow& renderWindow)
{
    return handleResizeImpl(event, originalSize, renderWindow, &getAspectRatioAwareViewport);
}


////////////////////////////////////////////////////////////
/// \brief Handles a window resize event to maintain pixel-perfect scaling.
///
/// Call this inside your event loop. If the event is a resize event,
/// it will automatically update the window's view to maintain a
/// centered, integer-scaled viewport.
///
/// \param event The sf::Event to process.
/// \param nativeResolution The original, internal resolution of the content.
/// \param renderWindow The sf::RenderWindow to update.
///
/// \return True if the event was a resize event and was handled, false otherwise.
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline bool handlePixelPerfectResize(const sf::Event&  event,
                                                   const sf::Vec2f   nativeResolution,
                                                   sf::RenderWindow& renderWindow)
{
    return handleResizeImpl(event, nativeResolution, renderWindow, &getPixelPerfectViewport);
}


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::RenderWindow makeDPIScaledRenderWindow(const sf::WindowSettings& windowSettings)
{
    const auto  fSize                      = windowSettings.size.toVec2f();
    const float primaryDisplayContentScale = sf::VideoModeUtils::getPrimaryDisplayContentScale();

    auto adjustedWindowSettings = windowSettings;
    adjustedWindowSettings.size = (fSize * primaryDisplayContentScale).toVec2u();

    sf::RenderWindow renderWindow(adjustedWindowSettings);
    renderWindow.setView({.center = fSize / 2.f, .size = fSize});
    return renderWindow;
}
