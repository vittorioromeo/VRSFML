#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/iOS/ClipboardImpl.hpp"

#include "SFML/System/String.hpp"
#include "SFML/System/StringUtfUtils.hpp"

#import <UIKit/UIKit.h>

namespace sf::priv
{

////////////////////////////////////////////////////////////
String ClipboardImpl::getString()
{
    UIPasteboard* const pboard = [UIPasteboard generalPasteboard];
    if (pboard.hasStrings)
    {
        const NSString* const data = pboard.string;

        const char*      utf8   = [data cStringUsingEncoding:NSUTF8StringEncoding];
        const NSUInteger length = [data lengthOfBytesUsingEncoding:NSUTF8StringEncoding];

        return StringUtfUtils::fromUtf8(utf8, utf8 + length);
    }

    return {};
}


////////////////////////////////////////////////////////////
void ClipboardImpl::setString(const String& text)
{
    const auto      utf8 = text.toUtf8();
    NSString* const data = [[NSString alloc]
        initWithBytes:utf8.data()
               length:utf8.length()
             encoding:NSUTF8StringEncoding];

    UIPasteboard* const pboard = [UIPasteboard generalPasteboard];
    pboard.string              = data;
}

} // namespace sf::priv
