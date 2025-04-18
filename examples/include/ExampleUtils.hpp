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
// DPI scaling (TODO P0: review)
inline void scaleWithDPI(sf::RenderWindow& renderWindow)
{
    const auto size = renderWindow.getSize().toVector2f();

    renderWindow.setSize((size * renderWindow.getWindowDisplayScale()).toVector2u());
    renderWindow.setView({.center = size / 2.f, .size = size});
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline bool handleAspectRatioAwareResize(const sf::Event& event, const sf::Vector2f originalSize, sf::RenderWindow& renderWindow)
{
    const auto* eResized = event.getIf<sf::Event::Resized>();
    if (eResized == nullptr)
        return false;

    const auto newSize = eResized->size.toVector2f();

    if (newSize.x <= 0.f || newSize.y <= 0.f)
        return true;

    const float contentAspectRatio = originalSize.x / originalSize.y; // Assume old size has correct ratio
    const float windowAspectRatio  = newSize.x / newSize.y;

    auto view = renderWindow.getView();

    if (SFML_BASE_MATH_FABSF(windowAspectRatio - contentAspectRatio) < 0.01f)
    {
        // Aspect ratios match, viewport covers the whole window
        view.viewport = {{0.f, 0.f}, {1.f, 1.f}};
    }
    else if (windowAspectRatio > contentAspectRatio)
    {
        // Window is wider than content (pillarboxing)
        // Viewport height is 100%, width is scaled

        view.viewport.size     = {contentAspectRatio / windowAspectRatio, 1.f};
        view.viewport.position = {(1.f - view.viewport.size.x) / 2.f, 0.f}; // Center horizontally
    }
    else
    {
        // Window is taller than content (letterboxing)
        // Viewport width is 100%, height is scaled

        view.viewport.size     = {1.f, windowAspectRatio / contentAspectRatio};
        view.viewport.position = {0.f, (1.f - view.viewport.size.y) / 2.f}; // Center vertically
    }

    renderWindow.setView(view);
    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::RenderWindow makeDPIScaledRenderWindow(const sf::WindowSettings& windowSettings)
{
    const auto  fSize                      = windowSettings.size.toVector2f();
    const float primaryDisplayContentScale = sf::VideoModeUtils::getPrimaryDisplayContentScale();

    auto adjustedWindowSettings = windowSettings;
    adjustedWindowSettings.size = (fSize * primaryDisplayContentScale).toVector2u();

    sf::RenderWindow renderWindow(adjustedWindowSettings);
    renderWindow.setView({.center = fSize / 2.f, .size = fSize});
    return renderWindow;
}
