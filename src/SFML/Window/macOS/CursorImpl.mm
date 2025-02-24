#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/macOS/AutoreleasePoolWrapper.hpp"
#include "SFML/Window/macOS/CursorImpl.hpp"
#import <SFML/Window/macOS/NSImage+raw.h>

#import <AppKit/AppKit.h>


namespace
{

////////////////////////////////////////////////////////////
NSCursor* loadFromSelector(SEL selector)
{
    // The caller is responsible for retaining the cursor.
    if ([NSCursor respondsToSelector:selector])
        return [NSCursor performSelector:selector];

    return nil;
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
CursorImpl::~CursorImpl()
{
    const AutoreleasePool pool;

    if (m_cursor)
        [m_cursor release];
}


////////////////////////////////////////////////////////////
CursorImpl::CursorImpl(CursorImpl&& rhs) noexcept : m_cursor(rhs.m_cursor)
{
    rhs.m_cursor = nil;
}


////////////////////////////////////////////////////////////
CursorImpl& CursorImpl::operator=(CursorImpl&& rhs) noexcept
{
    if (&rhs == this)
    {
        m_cursor     = rhs.m_cursor;
        rhs.m_cursor = nil;
    }

    return *this;
}


////////////////////////////////////////////////////////////
bool CursorImpl::loadFromPixels(const base::U8* pixels, Vector2u size, Vector2u hotspot)
{
    const AutoreleasePool pool;
    if (m_cursor)
    {
        [m_cursor release];
        m_cursor = nil;
    }

    const NSSize   nssize    = NSMakeSize(size.x, size.y);
    NSImage* const image     = [NSImage imageWithRawData:pixels andSize:nssize];
    const NSPoint  nshotspot = NSMakePoint(hotspot.x, hotspot.y);

    m_cursor = [[NSCursor alloc] initWithImage:image hotSpot:nshotspot];

    return m_cursor != nil;
}

////////////////////////////////////////////////////////////
bool CursorImpl::loadFromSystem(Cursor::Type type)
{
    const AutoreleasePool pool;
    NSCursor*             newCursor = nil;

    // clang-format off
    switch (type)
    {
        default: return false;

        case Cursor::Type::Arrow:           newCursor = [NSCursor arrowCursor];               break;
        case Cursor::Type::Text:            newCursor = [NSCursor IBeamCursor];               break;
        case Cursor::Type::Hand:            newCursor = [NSCursor pointingHandCursor];        break;
        case Cursor::Type::SizeHorizontal:  newCursor = [NSCursor resizeLeftRightCursor];     break;
        case Cursor::Type::SizeVertical:    newCursor = [NSCursor resizeUpDownCursor];        break;
        case Cursor::Type::Cross:           newCursor = [NSCursor crosshairCursor];           break;
        case Cursor::Type::NotAllowed:      newCursor = [NSCursor operationNotAllowedCursor]; break;
        case Cursor::Type::SizeLeft:        newCursor = [NSCursor resizeLeftRightCursor];     break;
        case Cursor::Type::SizeRight:       newCursor = [NSCursor resizeLeftRightCursor];     break;
        case Cursor::Type::SizeTop:         newCursor = [NSCursor resizeUpDownCursor];        break;
        case Cursor::Type::SizeBottom:      newCursor = [NSCursor resizeUpDownCursor];        break;

        // These cursor types are undocumented, may not be available on some platforms
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundeclared-selector"
        case Cursor::Type::SizeTopRight:
        case Cursor::Type::SizeBottomLeft:
        case Cursor::Type::SizeBottomLeftTopRight:
            newCursor = loadFromSelector(@selector(_windowResizeNorthEastSouthWestCursor));
            break;

        case Cursor::Type::SizeTopLeft:
        case Cursor::Type::SizeBottomRight:
        case Cursor::Type::SizeTopLeftBottomRight:
            newCursor = loadFromSelector(@selector(_windowResizeNorthWestSouthEastCursor));
            break;

        case Cursor::Type::Help:
            newCursor = loadFromSelector(@selector(_helpCursor));
            break;
#pragma GCC diagnostic pop
    }
    // clang-format on

    if (newCursor)
    {
        [m_cursor release];
        m_cursor = newCursor;
        [m_cursor retain];
    }

    return newCursor != nil;
}


} // namespace sf::priv
