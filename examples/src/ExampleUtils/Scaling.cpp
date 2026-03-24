// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/Scaling.hpp"

#include "ExampleUtils/MiniFmt.hpp"

#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderTextureCreateSettings.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
sf::Rect2f getAspectRatioAwareViewport(const sf::Vec2f newSize, const sf::Vec2f originalSize)
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
float getPixelPerfectScale(const sf::Vec2f windowSize, const sf::Vec2f nativeResolution)
{
    SFML_BASE_ASSERT(windowSize.x > 0.f && windowSize.y > 0.f);
    SFML_BASE_ASSERT(nativeResolution.x > 0.f && nativeResolution.y > 0.f);

    const auto scaleRatio = windowSize.componentWiseDiv(nativeResolution);

    // The final scale is the smaller of the two, floored to the nearest integer
    // This ensures that the scaled content fits entirely within the window
    return sf::base::max(1.f, sf::base::floor(sf::base::min(scaleRatio.x, scaleRatio.y)));
}


////////////////////////////////////////////////////////////
sf::Rect2f getPixelPerfectViewport(const sf::Vec2f windowSize, const sf::Vec2f nativeResolution)
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
static bool handleResizeImpl(const sf::Event& event, const sf::Vec2f originalSize, sf::View& view, auto&& fnViewport)
{
    const auto* eResized = event.getIf<sf::Event::Resized>();
    if (eResized == nullptr)
        return false;

    const auto newSize = eResized->size.toVec2f();

    if (newSize.x <= 0.f || newSize.y <= 0.f)
        return true;

    view.size     = originalSize;
    view.center   = originalSize / 2.f;
    view.viewport = fnViewport(newSize, originalSize);

    return true;
}


////////////////////////////////////////////////////////////
bool handleNonScalingResize(const sf::Event& event, [[maybe_unused]] const sf::Vec2f originalSize, sf::View& view)
{
    const auto* eResized = event.getIf<sf::Event::Resized>();
    if (eResized == nullptr)
        return false;

    const auto newSize = eResized->size.toVec2f();

    if (newSize.x <= 0.f || newSize.y <= 0.f)
        return true;

    view.size   = newSize;
    view.center = newSize / 2.f;

    return true;
}


////////////////////////////////////////////////////////////
bool handleAspectRatioAwareResize(const sf::Event& event, const sf::Vec2f originalSize, sf::View& view)
{
    return handleResizeImpl(event, originalSize, view, &getAspectRatioAwareViewport);
}


////////////////////////////////////////////////////////////
bool handlePixelPerfectResize(const sf::Event& event, const sf::Vec2f nativeResolution, sf::View& view)
{
    return handleResizeImpl(event, nativeResolution, view, &getPixelPerfectViewport);
}


////////////////////////////////////////////////////////////
sf::base::Optional<sf::RenderWindow> makeDPIScaledRenderWindow(const sf::WindowSettings& windowSettings)
{
    const auto  fSize                      = windowSettings.size.toVec2f();
    const float primaryDisplayContentScale = sf::VideoModeUtils::getPrimaryDisplayContentScale();

    auto adjustedWindowSettings = windowSettings;
    adjustedWindowSettings.size = (fSize * primaryDisplayContentScale).toVec2u();

    return sf::RenderWindow::create(adjustedWindowSettings);
}


////////////////////////////////////////////////////////////
sf::base::Optional<sf::RenderTexture> makeAARenderTexture(const sf::Vec2u                 resolution,
                                                          sf::RenderTextureCreateSettings rtCreateSettings)
{
    const auto maxAALevel = sf::RenderTexture::getMaximumAntiAliasingLevel();

    if (rtCreateSettings.antiAliasingLevel > maxAALevel)
    {
        minifmt::print("Desired AA level {} higher than supported {}, falling back to maximum",
                       rtCreateSettings.antiAliasingLevel,
                       maxAALevel);

        rtCreateSettings.antiAliasingLevel = maxAALevel;
    }

    return sf::RenderTexture::create(resolution.toVec2u(), rtCreateSettings);
}
