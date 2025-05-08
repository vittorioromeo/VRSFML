#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/Scaling.h>
#include "SFML/Window/macOS/cg_sf_conversion.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


namespace sf::priv
{

////////////////////////////////////////////////////////////
unsigned int modeBitsPerPixel(CGDisplayModeRef mode)
{
    unsigned int bpp = 0; // no match

    // Compare encoding.
    CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        bpp = 32;
    else if (CFStringCompare(pixEnc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        bpp = 16;
    else if (CFStringCompare(pixEnc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        bpp = 8;

    // Clean up memory.
    CFRelease(pixEnc);

    return bpp;
}


////////////////////////////////////////////////////////////
VideoMode convertCGModeToSFMode(CGDisplayModeRef cgmode)
{
    // The main documentation says the sizes returned by
    // CGDisplayModeGetWidth and CGDisplayModeGetHeight
    // are expressed in pixels. However, some additional
    // documentation [1] states they actually return
    // values in points starting with 10.8.
    //
    // We therefore needs to use the scaling factor to
    // convert the dimensions properly.
    //
    // [1]: "APIs for Supporting High Resolution" > "Additions and Changes for OS X v10.8"
    // https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/APIs/APIs.html#//apple_ref/doc/uid/TP40012302-CH5-SW27
    auto size = Vec2u(Vec2(CGDisplayModeGetPixelWidth(cgmode), CGDisplayModeGetPixelHeight(cgmode)));
    scaleInXY(size, nil);
    return VideoMode(size, modeBitsPerPixel(cgmode));
}

} // namespace sf::priv
