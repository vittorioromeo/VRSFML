#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#import <Foundation/Foundation.h>
#import <string>

@interface NSString (NSString_stdstring)

+ (id)stringWithstdstring:(const std::string&)string;

+ (id)stringWithstdwstring:(const std::wstring&)string;

- (std::string)tostdstring;

- (std::wstring)tostdwstring;

@end
