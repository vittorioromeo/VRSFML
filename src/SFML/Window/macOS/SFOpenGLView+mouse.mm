#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/macOS/WindowImplCocoa.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Optional.hpp"

#import <SFML/Window/macOS/SFOpenGLView+mouse_priv.h>
#import <SFML/Window/macOS/SFOpenGLView.h>


#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


////////////////////////////////////////////////////////////
/// In this file, we implement mouse handling for SFOpenGLView
///
////////////////////////////////////////////////////////////

@implementation SFOpenGLView (mouse)

////////////////////////////////////////////////////////
- (void)setCursor:(NSCursor*)cursor
{
    m_cursor = cursor;

    // indirect call to resetCursorRects to set the cursor
    [self.window invalidateCursorRectsForView:self];
}


////////////////////////////////////////////////////////
- (void)resetCursorRects
{
    // addCursorRect:cursor: has to be called from within this function!
    [self addCursorRect:[self frame] cursor:m_cursor];
    [m_cursor setOnMouseEntered:YES];
}


////////////////////////////////////////////////////////
- (BOOL)isMouseInside
{
    NSPoint relativeToWindow = [[self window] mouseLocationOutsideOfEventStream];
    NSPoint relativeToView   = [self convertPoint:relativeToWindow fromView:nil];

    return NSPointInRect(relativeToView, [self bounds]);
}


////////////////////////////////////////////////////////
- (void)updateMouseState
{
    // Update in/out state
    BOOL mouseWasIn = m_mouseIsIn;
    m_mouseIsIn     = [self isMouseInside];

    // Send event if needed.
    if (m_requester != nil)
    {
        if (mouseWasIn && !m_mouseIsIn)
            m_requester->mouseMovedOut();
        else if (!mouseWasIn && m_mouseIsIn)
            m_requester->mouseMovedIn();
    }
}


////////////////////////////////////////////////////////
- (void)setCursorGrabbed:(BOOL)grabbed
{
    m_cursorGrabbed = grabbed;

    [self updateCursorGrabbed];
}


////////////////////////////////////////////////////////
- (void)mouseDown:(NSEvent*)theEvent
{
    [self handleMouseDown:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] mouseDown:theEvent];
}


////////////////////////////////////////////////////////
- (void)rightMouseDown:(NSEvent*)theEvent
{
    [self handleMouseDown:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] rightMouseDown:theEvent];
}


////////////////////////////////////////////////////////
- (void)otherMouseDown:(NSEvent*)theEvent
{
    [self handleMouseDown:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] otherMouseDown:theEvent];
}


////////////////////////////////////////////////////////
- (void)handleMouseDown:(NSEvent*)theEvent
{
    const base::Optional<sf::Mouse::Button> button = [SFOpenGLView mouseButtonFromEvent:theEvent];

    if (m_requester != nil)
    {
        NSPoint loc = [self cursorPositionFromEvent:theEvent];

        if (button)
            m_requester->mouseDownAt(*button, sf::Vec2i(sf::Vec2(loc.x, loc.y)));
    }
}


////////////////////////////////////////////////////////
- (void)mouseUp:(NSEvent*)theEvent
{
    [self handleMouseUp:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] mouseUp:theEvent];
}


////////////////////////////////////////////////////////
- (void)rightMouseUp:(NSEvent*)theEvent
{
    [self handleMouseUp:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] rightMouseUp:theEvent];
}


////////////////////////////////////////////////////////
- (void)otherMouseUp:(NSEvent*)theEvent
{
    [self handleMouseUp:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] otherMouseUp:theEvent];
}


////////////////////////////////////////////////////////////
- (void)handleMouseUp:(NSEvent*)theEvent
{
    const base::Optional<sf::Mouse::Button> button = [SFOpenGLView mouseButtonFromEvent:theEvent];

    if (m_requester != nil)
    {
        NSPoint loc = [self cursorPositionFromEvent:theEvent];

        if (button)
            m_requester->mouseUpAt(*button, sf::Vec2i(sf::Vec2(loc.x, loc.y)));
    }
}


////////////////////////////////////////////////////////
- (void)mouseMoved:(NSEvent*)theEvent
{
    [self handleMouseMove:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] mouseMoved:theEvent];
}


////////////////////////////////////////////////////////
- (void)rightMouseDragged:(NSEvent*)theEvent
{
    [self handleMouseMove:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] rightMouseDragged:theEvent];
}


////////////////////////////////////////////////////////
- (void)mouseDragged:(NSEvent*)theEvent
{
    [self handleMouseMove:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] mouseDragged:theEvent];
}


////////////////////////////////////////////////////////
- (void)otherMouseDragged:(NSEvent*)theEvent
{
    [self handleMouseMove:theEvent];

    // Transmit to non-SFML responder
    [[self nextResponder] otherMouseUp:theEvent];
}


////////////////////////////////////////////////////////
- (void)handleMouseMove:(NSEvent*)theEvent
{
    NSPoint loc = [self cursorPositionFromEvent:theEvent];

    // If the cursor is grabbed, cursorPositionFromEvent: will
    // return its correct position but not move actually it
    // so we do it now.
    if ([self isCursorCurrentlyGrabbed])
        [self moveCursorTo:loc];

    // Make sure the point is inside the view.
    // (mouseEntered: and mouseExited: are not immediately called
    //  when the mouse is dragged. That would be too easy!)
    [self updateMouseState];
    if ((m_requester != nil) && m_mouseIsIn)
        m_requester->mouseMovedAt(sf::Vec2i(sf::Vec2(loc.x, loc.y)));
}


////////////////////////////////////////////////////////
- (BOOL)isCursorCurrentlyGrabbed
{
    return [[self window] isKeyWindow] && m_cursorGrabbed;
}


////////////////////////////////////////////////////////
- (void)updateCursorGrabbed
{
    // Disable/enable normal movements of the cursor
    // and project the cursor if needed.
    if ([self isCursorCurrentlyGrabbed])
    {
        CGAssociateMouseAndMouseCursorPosition(NO);

        // Similarly to handleMouseMove: but without event.
        NSPoint loc = [self cursorPositionFromEvent:nil];
        [self moveCursorTo:loc];
    }
    else
    {
        CGAssociateMouseAndMouseCursorPosition(YES);
    }
}


////////////////////////////////////////////////////////
- (void)moveCursorTo:(NSPoint)loc
{
    // Convert the point from SFML coord system to screen coord system.
    NSPoint screenLocation = [self computeGlobalPositionOfRelativePoint:loc];

    // This won't produce a move event, which is perfect if the cursor was grabbed
    // as we move it manually based on delta values of the cursor.
    CGDisplayMoveCursorToPoint([self displayId], NSPointToCGPoint(screenLocation));
}


////////////////////////////////////////////////////////
- (CGDirectDisplayID)displayId
{
    NSScreen* screen    = [[self window] screen];
    NSNumber* displayId = [[screen deviceDescription] objectForKey:@"NSScreenNumber"];
    return static_cast<unsigned int>([displayId intValue]);
}


////////////////////////////////////////////////////////
- (void)scrollWheel:(NSEvent*)theEvent
{
    if (m_requester != nil)
    {
        NSPoint loc = [self cursorPositionFromEvent:theEvent];
        m_requester->mouseWheelScrolledAt(static_cast<float>([theEvent deltaX]),
                                          static_cast<float>([theEvent deltaY]),
                                          sf::Vec2i(sf::Vec2(loc.x, loc.y)));
    }

    // Transmit to non-SFML responder
    [[self nextResponder] scrollWheel:theEvent];
}


////////////////////////////////////////////////////////
- (void)mouseEntered:(NSEvent*)theEvent
{
    (void)theEvent;
    [self updateMouseState];
}


////////////////////////////////////////////////////////
- (void)mouseExited:(NSEvent*)theEvent
{
    (void)theEvent;
    [self updateMouseState];
}


////////////////////////////////////////////////////////
- (NSPoint)cursorPositionFromEvent:(NSEvent*)eventOrNil
{
    NSPoint rawPos;

    // If no event given then get current mouse pos.
    if (eventOrNil == nil)
        rawPos = [[self window] mouseLocationOutsideOfEventStream];
    else
        rawPos = [eventOrNil locationInWindow];

    if ([self isCursorCurrentlyGrabbed])
    {
        if (eventOrNil != nil)
        {
            // Special case when the mouse is grabbed:
            // we need to take into account the delta since the cursor
            // is dissociated from its position.

            // Ignore any non-move related event
            if (([eventOrNil type] == NSMouseMoved) || ([eventOrNil type] == NSLeftMouseDragged) ||
                ([eventOrNil type] == NSRightMouseDragged) || ([eventOrNil type] == NSOtherMouseDragged))
            {
                // Without this factor, the cursor flies around waaay too fast!
                // But I don't know if it because of retina display or because
                // some event are sent twice (and that in itself is another mystery).
                CGFloat factor = 2;

                // Also, this factor is not the same when keeping track of how much
                // we move the cursor (buffers) when projecting the cursor into the
                // view when grabbing the cursor for the first time.
                CGFloat factorBuffer = m_fullscreen ? 1 : 2;

                CGFloat deltaX = [eventOrNil deltaX];
                CGFloat deltaY = [eventOrNil deltaY];

                // If the buffer for X is empty, move the cursor;
                // otherwise decrement this buffer a bit.
                if (m_deltaXBuffer <= 0)
                    rawPos.x += deltaX / factor;
                else
                    m_deltaXBuffer -= base::fabs(deltaX / factorBuffer);

                // Rinse and repeat for Y.
                if (m_deltaYBuffer <= 0)
                    rawPos.y -= deltaY / factor;
                else
                    m_deltaYBuffer -= base::fabs(deltaY / factorBuffer);
            }
        }

        // We also make sure the new point is inside the view
        NSSize  size   = [self frame].size;
        NSPoint origin = [self frame].origin;
        NSPoint oldPos = rawPos;
        rawPos.x       = base::clamp(rawPos.x, origin.x, origin.x + size.width - 1);
        rawPos.y       = base::clamp(rawPos.y, origin.y + 1, origin.y + size.height);
        // Note: the `-1` and `+1` on the two lines above prevent the user to click
        // on the left or below the window, respectively, and therefore prevent the
        // application to lose focus by accident. The sign of this offset is determined
        // by the direction of the x and y axis.

        // Increase X and Y buffer with the distance of the projection
        m_deltaXBuffer += base::fabs(rawPos.x - oldPos.x);
        m_deltaYBuffer += base::fabs(rawPos.y - oldPos.y);
    }

    NSPoint loc = [self convertPoint:rawPos fromView:nil];

    // Don't forget to change to SFML coord system.
    const double h = [self frame].size.height;
    loc.y          = h - loc.y;

    return loc;
}


////////////////////////////////////////////////////////
+ (base::Optional<sf::Mouse::Button>)mouseButtonFromEvent:(NSEvent*)event
{
    switch ([event buttonNumber])
    {
        case 0:
            return sf::Mouse::Button::Left;
        case 1:
            return sf::Mouse::Button::Right;
        case 2:
            return sf::Mouse::Button::Middle;
        case 3:
            return sf::Mouse::Button::Extra1;
        case 4:
            return sf::Mouse::Button::Extra2;
        default:
            return base::nullOpt; // Never happens! (hopefully)
    }
}


@end
