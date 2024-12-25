#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/iOS/SFAppDelegate.hpp"
#include "SFML/Window/iOS/SFView.hpp"

#include "SFML/System/Utf.hpp"

#include "SFML/Base/Builtins/Strlen.hpp"

#include <QuartzCore/CAEAGLLayer.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

@interface SFView ()

// NOLINTNEXTLINE(readability-identifier-naming)
@property (nonatomic) NSMutableArray* touches;

@end


@implementation SFView

@synthesize context;


////////////////////////////////////////////////////////////
- (BOOL)canBecomeFirstResponder
{
    return true;
}


////////////////////////////////////////////////////////////
- (BOOL)hasText
{
    return true;
}


////////////////////////////////////////////////////////////
- (void)deleteBackward
{
    [[SFAppDelegate getInstance] notifyCharacter:'\b'];
}


////////////////////////////////////////////////////////////
- (void)insertText:(NSString*)text
{
    // Convert the NSString to UTF-8
    const char* utf8 = [text UTF8String];

    // Then convert to UTF-32 and notify the application delegate of each new character
    const char* end = utf8 + SFML_BASE_STRLEN(utf8);
    while (utf8 < end)
    {
        char32_t character = 0;
        utf8               = sf::Utf8::decode(utf8, end, character);
        [[SFAppDelegate getInstance] notifyCharacter:character];
    }
}


////////////////////////////////////////////////////////////
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    for (UITouch* touch in touches) // NOLINT(cppcoreguidelines-init-variables)
    {
        // find an empty slot for the new touch
        NSUInteger index = [self.touches indexOfObject:[NSNull null]];
        if (index != NSNotFound)
        {
            [self.touches replaceObjectAtIndex:index withObject:touch];
        }
        else
        {
            [self.touches addObject:touch];
            index = [self.touches count] - 1;
        }

        // get the touch position
        CGPoint      point = [touch locationInView:self];
        sf::Vector2i position(static_cast<int>(point.x), static_cast<int>(point.y));

        // notify the application delegate
        [[SFAppDelegate getInstance] notifyTouchBegin:(static_cast<unsigned int>(index)) atPosition:position];
    }
}


////////////////////////////////////////////////////////////
- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    for (UITouch* touch in touches) // NOLINT(cppcoreguidelines-init-variables)
    {
        // find the touch
        NSUInteger index = [self.touches indexOfObject:touch];
        if (index != NSNotFound)
        {
            // get the touch position
            CGPoint      point = [touch locationInView:self];
            sf::Vector2i position(static_cast<int>(point.x), static_cast<int>(point.y));

            // notify the application delegate
            [[SFAppDelegate getInstance] notifyTouchMove:(static_cast<unsigned int>(index)) atPosition:position];
        }
    }
}


////////////////////////////////////////////////////////////
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    for (UITouch* touch in touches) // NOLINT(cppcoreguidelines-init-variables)
    {
        // find the touch
        NSUInteger index = [self.touches indexOfObject:touch];
        if (index != NSNotFound)
        {
            // get the touch position
            CGPoint      point = [touch locationInView:self];
            sf::Vector2i position(static_cast<int>(point.x), static_cast<int>(point.y));

            // notify the application delegate
            [[SFAppDelegate getInstance] notifyTouchEnd:(static_cast<unsigned int>(index)) atPosition:position];

            // remove the touch
            [self.touches replaceObjectAtIndex:index withObject:[NSNull null]];
        }
    }
}


////////////////////////////////////////////////////////////
- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
    // Treat touch cancel events the same way as touch end
    [self touchesEnded:touches withEvent:event];
}


////////////////////////////////////////////////////////////
- (void)layoutSubviews
{
    // update the attached context's buffers
    if (self.context)
        self.context->recreateRenderBuffers(self);
}


////////////////////////////////////////////////////////////
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

////////////////////////////////////////////////////////////
- (id)initWithFrame:(CGRect)frame andContentScaleFactor:(CGFloat)factor
{
    self = [super initWithFrame:frame];

    self.contentScaleFactor = factor;

    if (self)
    {
        self.context = nullptr;
        self.touches = [NSMutableArray array];

        // Configure the EAGL layer
        auto* eaglLayer              = static_cast<CAEAGLLayer*>(self.layer);
        eaglLayer.opaque             = YES;
        eaglLayer.drawableProperties = [NSDictionary
            dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE],
                                         kEAGLDrawablePropertyRetainedBacking,
                                         kEAGLColorFormatRGBA8,
                                         kEAGLDrawablePropertyColorFormat,
                                         nil];

        // Enable user interactions on the view (multi-touch events)
        self.userInteractionEnabled = true;
        self.multipleTouchEnabled   = true;
    }

    return self;
}

////////////////////////////////////////////////////////////
- (UITextAutocorrectionType)autocorrectionType
{
    return UITextAutocorrectionTypeNo;
}


@end
