#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/Base/Math/Fabs.hpp"


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
[[nodiscard]] inline bool handleAspectRatioAwareResize(const sf::Event&  event,
                                                       const sf::Vec2f   originalSize,
                                                       sf::RenderWindow& renderWindow)
{
    const auto* eResized = event.getIf<sf::Event::Resized>();
    if (eResized == nullptr)
        return false;

    const auto newSize = eResized->size.toVec2f();

    if (newSize.x <= 0.f || newSize.y <= 0.f)
        return true;

    auto view     = renderWindow.getView();
    view.viewport = getAspectRatioAwareViewport(newSize, originalSize);
    renderWindow.setView(view);

    return true;
}


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
