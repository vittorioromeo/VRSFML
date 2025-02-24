#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/WindowImplDelegateProtocol.h>


////////////////////////////////////////////////////////////
/// Predefine some classes
////////////////////////////////////////////////////////////
@class SFOpenGLView;

////////////////////////////////////////////////////////////
/// \brief Implementation of WindowImplDelegateProtocol for view management
///
////////////////////////////////////////////////////////////
// NOLINTBEGIN(readability-identifier-naming)
@interface SFViewController : NSObject<WindowImplDelegateProtocol>
{
    NSView*                    m_view;      ///< Underlying Cocoa view
    SFOpenGLView*              m_oglView;   ///< OpenGL view
    sf::priv::WindowImplCocoa* m_requester; ///< View's requester
}
// NOLINTEND(readability-identifier-naming)

////////////////////////////////////////////////////////////
/// \brief Initialize the view controller
///
/// \param view view to be controlled
///
/// \return an initialized view controller
///
////////////////////////////////////////////////////////////
- (id)initWithView:(NSView*)view;

@end
