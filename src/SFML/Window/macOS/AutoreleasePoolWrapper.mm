#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/macOS/AutoreleasePoolWrapper.hpp"

#import <Foundation/Foundation.h>

namespace sf
{

////////////////////////////////////////////////////////
AutoreleasePool::AutoreleasePool()
{
    m_pool = [[NSAutoreleasePool alloc] init];
}


////////////////////////////////////////////////////////
AutoreleasePool::~AutoreleasePool()
{
    [m_pool drain];
}

} // namespace sf
