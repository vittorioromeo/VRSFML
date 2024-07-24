#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#import <Cocoa/Cocoa.h>

/*
 * NB: We need pointers for C++ objects fields in Obj-C interface !
 *     The recommended way is to use PIMPL idiom.
 *
 *     It's elegant. Moreover, we do no constrain
 *     other file including this one to be Obj-C++.
 */

struct SFMLmainWindow;

// NOLINTBEGIN(readability-identifier-naming)
@interface CocoaAppDelegate : NSObject<NSApplicationDelegate>
{
@private
    NSWindow*       m_window;
    NSView*         m_sfmlView;
    NSTextField*    m_textField;
    SFMLmainWindow* m_mainWindow;
    NSTimer*        m_renderTimer;
    BOOL            m_visible;
    BOOL            m_initialized;
}
// NOLINTEND(readability-identifier-naming)

@property (retain) IBOutlet NSWindow* window;

@property (assign) IBOutlet NSView* sfmlView;

@property (assign) IBOutlet NSTextField* textField;

- (IBAction)colorChanged:(NSPopUpButton*)sender;
- (IBAction)rotationChanged:(NSSlider*)sender;
- (IBAction)visibleChanged:(NSButton*)sender;
- (IBAction)textChanged:(NSTextField*)sender;
- (IBAction)updateText:(NSButton*)sender;

@end

/*
 * This interface is used to prevent the system alert produced when the SFML view
 * has the focus and the user press a key.
 */
@interface SilentWindow : NSWindow

- (void)keyDown:(NSEvent*)theEvent;

@end
