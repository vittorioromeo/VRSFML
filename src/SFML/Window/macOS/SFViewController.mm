#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/SFApplication.h>
#import <SFML/Window/macOS/SFOpenGLView.h>
#import <SFML/Window/macOS/SFViewController.h>
#include <SFML/Window/macOS/WindowImplCocoa.hpp>

#include <SFML/System/Err.hpp>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

@implementation SFViewController


////////////////////////////////////////////////////////
- (id)initWithView:(NSView*)view
{
    if ((self = [super init]))
    {
        m_requester = nullptr;

        // Retain the view for our own use.
        m_view = [view retain];

        if (m_view == nil)
        {
            sf::priv::err() << "No view was given to initWithWindow:." ;
            return self;
        }

        // Create the view.
        NSRect frame   = [m_view frame];
        frame.origin.x = 0;
        frame.origin.y = 0;
        m_oglView      = [[SFOpenGLView alloc] initWithFrame:frame];

        if (m_oglView == nil)
        {
            sf::priv::err() << "Could not create an instance of NSOpenGLView "
                            << "in (SFViewController -initWithView:)." ;

            return self;
        }

        // Set the (OGL) view to the view as its "content" view.
        [m_view addSubview:m_oglView];

        [m_oglView setAutoresizingMask:[m_view autoresizingMask]];

        [m_oglView finishInit];
    }

    return self;
}


////////////////////////////////////////////////////////
- (void)dealloc
{
    [self closeWindow];

    [m_view release];
    [m_oglView release];

    [super dealloc];
}


////////////////////////////////////////////////////////
- (CGFloat)displayScaleFactor
{
    return [m_oglView displayScaleFactor];
}


////////////////////////////////////////////////////////
- (void)setRequesterTo:(sf::priv::WindowImplCocoa*)requester
{
    // Forward to the view.
    [m_oglView setRequesterTo:requester];
    m_requester = requester;
}


////////////////////////////////////////////////////////
- (sf::WindowHandle)getNativeHandle
{
    return m_view;
}


////////////////////////////////////////////////////////
- (BOOL)isMouseInside
{
    return [m_oglView isMouseInside];
}


////////////////////////////////////////////////////////
- (void)setCursorGrabbed:(BOOL)grabbed
{
    [m_oglView setCursorGrabbed:grabbed];
}


////////////////////////////////////////////////////////
- (void)setCursor:(NSCursor*)cursor
{
    [m_oglView setCursor:cursor];
}


////////////////////////////////////////////////////////////
- (NSPoint)position
{
    // Origin is bottom-left corner of the window
    return [m_view convertPoint:NSMakePoint(0, 0) toView:nil]; // nil means window
}


////////////////////////////////////////////////////////.
- (void)setWindowPositionTo:(sf::Vector2i)position
{
    (void)position;
    sf::priv::err() << "Cannot move SFML area when SFML is integrated in a NSView. Use the view handler directly "
                       "instead."
                    ;
}


////////////////////////////////////////////////////////////
- (NSSize)size
{
    return [m_oglView frame].size;
}


////////////////////////////////////////////////////////
- (void)resizeTo:(sf::Vector2u)size
{
    NSRect frame = NSMakeRect([m_view frame].origin.x, [m_view frame].origin.y, size.x, size.y);

    [m_view setFrame:frame];
    [m_oglView setFrame:frame];
}


////////////////////////////////////////////////////////
- (void)setMinimumSize:(NSSize)size
{
}


////////////////////////////////////////////////////////
- (void)setMaximumSize:(NSSize)size
{
}


////////////////////////////////////////////////////////
- (void)changeTitle:(NSString*)title
{
    (void)title;
    sf::priv::err() << "Cannot change the title of the SFML area when SFML is integrated in a NSView." ;
}


////////////////////////////////////////////////////////
- (void)hideWindow
{
    [m_view setHidden:YES];
}


////////////////////////////////////////////////////////
- (void)showWindow
{
    [m_view setHidden:NO];
}


////////////////////////////////////////////////////////
- (void)closeWindow
{
    // If the "window" is really a view, this is a no-op.
}


////////////////////////////////////////////////////////
- (void)requestFocus
{
    // Note: this doesn't imply that the view will get any event.
    // The user has to make sure events are forwarded to the view
    // with the usual responder chain.
    [[m_view window] makeKeyAndOrderFront:nil];

    // In case the app is not active, make its dock icon bounce for one sec
    [NSApp requestUserAttention:NSInformationalRequest];
}


////////////////////////////////////////////////////////////
- (BOOL)hasFocus
{
    return [NSApp keyWindow] == [m_view window];
}


////////////////////////////////////////////////////////
- (void)enableKeyRepeat
{
    [m_oglView enableKeyRepeat];
}


////////////////////////////////////////////////////////
- (void)disableKeyRepeat
{
    [m_oglView disableKeyRepeat];
}


////////////////////////////////////////////////////////
- (void)setIconTo:(sf::Vector2u)size with:(const std::uint8_t*)pixels
{
    (void)size;
    (void)pixels;
    sf::priv::err() << "Cannot set an icon when SFML is integrated in a NSView." ;
}


////////////////////////////////////////////////////////
- (void)processEvent
{
    // If we are not on the main thread we stop here and advice the user.
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        /*
         * See https://lists.apple.com/archives/cocoa-dev/2011/Feb/msg00460.html
         * for more information.
         */
        sf::priv::err() << "Cannot fetch event from a worker thread. (OS X restriction)" ;

        return;
    }

    // If we don't have a requester we don't fetch event.
    if (m_requester != nullptr)
        [SFApplication processEvent];
}


////////////////////////////////////////////////////////
- (void)applyContext:(NSOpenGLContext*)context
{
    [m_oglView setOpenGLContext:context];
    [context setView:m_oglView];
}


@end
