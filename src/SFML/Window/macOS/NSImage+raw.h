#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#import <AppKit/AppKit.h>

#include "SFML/Base/IntTypes.hpp"

////////////////////////////////////////////////////////////
/// Extends NSImage with a convenience method to load images
/// from raw data.
///
////////////////////////////////////////////////////////////

@interface NSImage (raw)

////////////////////////////////////////////////////////////
/// \brief Load an image from raw RGBA pixels
///
/// \param pixels array of 4 * `size` bytes representing the image
/// \param size size of the image
///
/// \return an instance of NSImage that needs to be released by the caller
///
////////////////////////////////////////////////////////////
+ (NSImage*)imageWithRawData:(const base::U8*)pixels andSize:(NSSize)size;

@end
