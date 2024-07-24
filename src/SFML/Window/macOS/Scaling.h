#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/WindowImplDelegateProtocol.h>

#import <AppKit/AppKit.h>

namespace sf::priv
{

////////////////////////////////////////////////////////////
/// \brief Get the scale factor of the main screen
///
////////////////////////////////////////////////////////////
inline CGFloat getDefaultScaleFactor()
{
    return [[NSScreen mainScreen] backingScaleFactor];
}

////////////////////////////////////////////////////////////
/// \brief Scale SFML coordinates to backing coordinates
///
/// \param in SFML coordinates to be converted
/// \param delegate an object implementing WindowImplDelegateProtocol, or nil for default scale
///
////////////////////////////////////////////////////////////
template <class T>
void scaleIn(T& in, id<WindowImplDelegateProtocol> delegate)
{
    in /= static_cast<T>(delegate ? [delegate displayScaleFactor] : getDefaultScaleFactor());
}

template <class T>
void scaleInXY(T& in, id<WindowImplDelegateProtocol> delegate)
{
    scaleIn(in.x, delegate);
    scaleIn(in.y, delegate);
}

////////////////////////////////////////////////////////////
/// \brief Scale backing coordinates to SFML coordinates
///
/// \param out backing coordinates to be converted
/// \param delegate an object implementing WindowImplDelegateProtocol, or nil for default scale
///
////////////////////////////////////////////////////////////
template <class T>
void scaleOut(T& out, id<WindowImplDelegateProtocol> delegate)
{
    out *= static_cast<T>(delegate ? [delegate displayScaleFactor] : getDefaultScaleFactor());
}

template <class T>
void scaleOutXY(T& out, id<WindowImplDelegateProtocol> delegate)
{
    scaleOut(out.x, delegate);
    scaleOut(out.y, delegate);
}

} // namespace sf::priv
