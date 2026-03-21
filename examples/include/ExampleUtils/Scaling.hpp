#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class RenderWindow;
class RenderTexture;
class Event;
struct View;
struct WindowSettings;
} // namespace sf


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] sf::Rect2f getAspectRatioAwareViewport(sf::Vec2f newSize, sf::Vec2f originalSize);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] float getPixelPerfectScale(sf::Vec2f windowSize, sf::Vec2f nativeResolution);

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
/// \return A `sf::Rect2f` defining the viewport in normalized coordinates.
///
////////////////////////////////////////////////////////////
[[nodiscard]] sf::Rect2f getPixelPerfectViewport(sf::Vec2f windowSize, sf::Vec2f nativeResolution);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool handleNonScalingResize(const sf::Event& event, [[maybe_unused]] sf::Vec2f originalSize, sf::View& view);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool handleAspectRatioAwareResize(const sf::Event& event, sf::Vec2f originalSize, sf::View& view);

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
[[nodiscard]] bool handlePixelPerfectResize(const sf::Event& event, sf::Vec2f nativeResolution, sf::View& view);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Optional<sf::RenderWindow> makeDPIScaledRenderWindow(const sf::WindowSettings& windowSettings);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Optional<sf::RenderTexture> makeAARenderTexture(sf::Vec2u resolution, unsigned int desiredAALevel);
