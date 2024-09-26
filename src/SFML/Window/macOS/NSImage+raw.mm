#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/NSImage+raw.h>

@implementation NSImage (raw)

+ (NSImage*)imageWithRawData:(const base::U8*)pixels andSize:(NSSize)size
{
    // Create an empty image representation.
    NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:nil // if nil: only allocate memory
                      pixelsWide:(static_cast<NSInteger>(size.width))pixelsHigh
                                :(static_cast<NSInteger>(size.height))bitsPerSample:8 // The number of bits used to specify
                 // one pixel in a single component of the data.
                 samplesPerPixel:4 // 3 if no alpha, 4 with it
                        hasAlpha:YES
                        isPlanar:NO // I don't know what it is but it works
                  colorSpaceName:NSCalibratedRGBColorSpace
                     bytesPerRow:0   // 0 == determine automatically
                    bitsPerPixel:0]; // 0 == determine automatically

    // Load data pixels.
    for (unsigned int y = 0; y < size.height; ++y)
    {
        for (unsigned int x = 0; x < size.width; ++x, pixels += 4)
        {
            NSUInteger pixel[4] = {pixels[0], pixels[1], pixels[2], pixels[3]};
            [bitmap setPixel:pixel atX:x y:y];
        }
    }

    // Create an image from the representation.
    NSImage* image = [[NSImage alloc] initWithSize:size];
    [image addRepresentation:bitmap];

    [bitmap release];

    return image;
}

@end
