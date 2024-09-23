#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/macOS/AutoreleasePoolWrapper.hpp"
#include "SFML/Window/macOS/ClipboardImpl.hpp"

#include "SFML/System/String.hpp"
#include "SFML/System/StringUtfUtils.hpp"

#import <AppKit/AppKit.h>
#include <string>


namespace sf::priv
{

////////////////////////////////////////////////////////////
String ClipboardImpl::getString()
{
    const AutoreleasePool pool;
    NSPasteboard* const   pboard = [NSPasteboard generalPasteboard];
    NSString* const       data   = [pboard stringForType:NSPasteboardTypeString];

    const char*      utf8   = [data cStringUsingEncoding:NSUTF8StringEncoding];
    const NSUInteger length = [data lengthOfBytesUsingEncoding:NSUTF8StringEncoding];

    return StringUtfUtils::fromUtf8(utf8, utf8 + length);
}


////////////////////////////////////////////////////////////
void ClipboardImpl::setString(const String& text)
{
    const AutoreleasePool pool;
    const auto            utf8 = text.toUtf8<std::u8string>();
    NSString* const       data = [[NSString alloc]
        initWithBytes:utf8.data()
               length:utf8.length()
             encoding:NSUTF8StringEncoding];

    NSPasteboard* const pboard = [NSPasteboard generalPasteboard];
    [pboard declareTypes:@[NSPasteboardTypeString] owner:nil];
    [pboard setString:data forType:NSPasteboardTypeString];

    [data release];
}

} // namespace sf::priv
