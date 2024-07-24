#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#import "NSString+stdstring.h"

#include <SFML/Base/Algorithm.hpp>
#include <SFML/System/Utf.hpp>

@implementation NSString (NSString_stdstring)

+ (id)stringWithstdstring:(const std::string&)string
{
    std::string utf8;
    utf8.reserve(string.size() + 1);

    sf::Utf8::fromAnsi(string.begin(), string.end(), base::BackInserter(utf8));

    NSString* str = [NSString stringWithCString:utf8.c_str() encoding:NSUTF8StringEncoding];
    return str;
}

+ (id)stringWithstdwstring:(const std::wstring&)string
{
    const void* data = static_cast<const void*>(string.data());
    auto        size = static_cast<unsigned>(string.size() * sizeof(wchar_t));

    NSString* str = [[[NSString alloc] initWithBytes:data length:size
                                            encoding:NSUTF32LittleEndianStringEncoding] autorelease];
    return str;
}

- (std::string)tostdstring
{
    // Not sure about the encoding to use. Using [self UTF8String] doesn't
    // work for characters like é or à.
    const char* cstr = [self cStringUsingEncoding:NSISOLatin1StringEncoding];

    if (cstr != nullptr)
        return std::string(cstr);

    return "";
}

- (std::wstring)tostdwstring
{
    // According to Wikipedia, macOS is Little Endian on x86 and x86-64
    // https://en.wikipedia.org/wiki/Endianness
    NSData* asData = [self dataUsingEncoding:NSUTF32LittleEndianStringEncoding];
    return std::wstring(static_cast<const wchar_t*>([asData bytes]), [asData length] / sizeof(wchar_t));
}

@end
