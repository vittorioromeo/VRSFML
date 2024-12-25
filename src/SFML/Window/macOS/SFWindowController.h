#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowEnums.hpp"

#import <SFML/Window/macOS/WindowImplDelegateProtocol.h>

////////////////////////////////////////////////////////////
/// Predefine some classes
////////////////////////////////////////////////////////////
namespace sf::priv
{
class WindowImplCocoa;
}

@class SFOpenGLView;

////////////////////////////////////////////////////////////
/// \brief Implementation of WindowImplDelegateProtocol for window management
///
/// Key, mouse and Window focus events are delegated to its view, SFOpenGLView.
///
/// Used when SFML handle everything and when a NSWindow* is given
/// as handle to WindowImpl.
///
/// When grabbing the cursor, if the window is resizeable, m_restoreResize is
/// set to YES and the window is marked as not resizeable. This is to prevent
/// accidental resize by the user. When the cursor is released, the window
/// style is restored.
///
////////////////////////////////////////////////////////////
// NOLINTBEGIN(readability-identifier-naming)
@interface SFWindowController : NSResponder<WindowImplDelegateProtocol, NSWindowDelegate>
{
    NSWindow*                  m_window;        ///< Underlying Cocoa window to be controlled
    SFOpenGLView*              m_oglView;       ///< OpenGL view for rendering
    sf::priv::WindowImplCocoa* m_requester;     ///< Requester
    BOOL                       m_fullscreen;    ///< Indicate whether the window is fullscreen or not
    BOOL                       m_restoreResize; ///< See note above
    BOOL                       m_highDpi;       ///< Support high-DPI rendering or not
}
// NOLINTEND(readability-identifier-naming)

////////////////////////////////////////////////////////////
/// \brief Create the SFML window with an external Cocoa window
///
/// \param window Cocoa window to be controlled
///
/// \return an initialized controller
///
////////////////////////////////////////////////////////////
- (id)initWithWindow:(NSWindow*)window;

////////////////////////////////////////////////////////////
/// \brief Create the SFML window "from scratch" (SFML handle everything)
///
/// \param mode Video mode
/// \param style Window's style, as described by sf::Style
/// \param state Window's state
///
/// \return an initialized controller
///
////////////////////////////////////////////////////////////
- (id)initWithMode:(const sf::VideoMode&)mode andStyle:(base::U32)style andFullscreen:(bool)fullscreen;

@end
