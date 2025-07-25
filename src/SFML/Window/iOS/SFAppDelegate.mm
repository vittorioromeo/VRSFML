#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/iOS/SFAppDelegate.hpp"
#include "SFML/Window/iOS/SFMain.hpp"

#import <TargetConditionals.h>
#include <vector>

#if TARGET_IPHONE_SIMULATOR
#include <crt_externs.h>
#endif


namespace
{
// Save the global instance of the delegate
SFAppDelegate* delegateInstance = nullptr;

// Current touches positions
std::vector<sf::Vec2i> touchPositions;
} // namespace


@interface SFAppDelegate ()

// NOLINTNEXTLINE(readability-identifier-naming)
@property (nonatomic) CMMotionManager* motionManager;

@end


@implementation SFAppDelegate

@synthesize sfWindow;
@synthesize backingScaleFactor;

////////////////////////////////////////////////////////////
+ (SFAppDelegate*)getInstance
{
    NSAssert(delegateInstance,
             @"SFAppDelegate instance is nil, this means SFML was not properly initialized. "
              "Make sure that the file defining your main() function includes <SFML/Main.hpp>");

    return delegateInstance;
}

////////////////////////////////////////////////////////////
- (void)runUserMain
{
#if TARGET_IPHONE_SIMULATOR
    sfmlMain(*_NSGetArgc(), *_NSGetArgv());
#else
    sfmlMain(0, nullptr);
#endif
}

////////////////////////////////////////////////////////////
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    // Save the delegate instance
    delegateInstance = self;

    [self initBackingScale];

    // Instantiate the motion manager
    self.motionManager = [[CMMotionManager alloc] init];

    // Register orientation changes notifications
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(deviceOrientationDidChange:)
               name:UIDeviceOrientationDidChangeNotification
             object:nil];

    // Change the working directory to the resources directory
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:[[NSBundle mainBundle] resourcePath]];

    // Schedule an indirect call to the user main, so that this call (and the whole
    // init sequence) can end, and the default splashscreen can be destroyed
    [self performSelector:@selector(runUserMain) withObject:nil afterDelay:0.0];

    return true;
}

- (void)initBackingScale
{
    id data = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSHighResolutionCapable"];
    if (data && [data boolValue])
        backingScaleFactor = [[UIScreen mainScreen] scale];
    else
        backingScaleFactor = 1;
}

////////////////////////////////////////////////////////////
- (void)applicationWillResignActive:(UIApplication*)application
{
    // Called when:
    // - the application is sent to background
    // - the application is interrupted by a call or message

    // Generate a FocusLost event
    if (self.sfWindow)
        sfWindow->forwardEvent(sf::Event::FocusLost{});
}

////////////////////////////////////////////////////////////
- (void)applicationDidEnterBackground:(UIApplication*)application
{
    // Called when the application is sent to background (home button pressed)
}

////////////////////////////////////////////////////////////
- (void)applicationDidBecomeActive:(UIApplication*)application
{
    // Called when:
    // - the application is sent to foreground
    // - the application was interrupted by a call or message

    // Generate a FocusGained event
    if (self.sfWindow)
        sfWindow->forwardEvent(sf::Event::FocusGained{});
}

////////////////////////////////////////////////////////////
- (void)applicationWillEnterForeground:(UIApplication*)application
{
    // Called when the application is sent to foreground (app icon pressed)
}

////////////////////////////////////////////////////////////
- (void)applicationWillTerminate:(UIApplication*)application
{
    // Generate a Closed event
    if (self.sfWindow)
        sfWindow->forwardEvent(sf::Event::Closed{});
}

- (bool)supportsOrientation:(UIDeviceOrientation)orientation
{
    if (!self.sfWindow)
        return false;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

    UIViewController* rootViewController = [((__bridge UIWindow*)(self.sfWindow->getNativeHandle())) rootViewController];

#pragma GCC diagnostic pop

    if (!rootViewController || ![rootViewController shouldAutorotate])
        return false;

    NSArray* supportedOrientations = [[NSBundle mainBundle]
        objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];
    if (!supportedOrientations)
        return (1 << orientation) & [rootViewController supportedInterfaceOrientations];

    int appFlags = 0;
    if ([supportedOrientations containsObject:@"UIInterfaceOrientationPortrait"])
        appFlags += UIInterfaceOrientationMaskPortrait;
    if ([supportedOrientations containsObject:@"UIInterfaceOrientationPortraitUpsideDown"])
        appFlags += UIInterfaceOrientationMaskPortraitUpsideDown;
    if ([supportedOrientations containsObject:@"UIInterfaceOrientationLandscapeLeft"])
        appFlags += UIInterfaceOrientationMaskLandscapeLeft;
    if ([supportedOrientations containsObject:@"UIInterfaceOrientationLandscapeRight"])
        appFlags += UIInterfaceOrientationMaskLandscapeRight;

    return (1 << orientation) & [rootViewController supportedInterfaceOrientations] & static_cast<unsigned long>(appFlags);
}

- (bool)needsToFlipFrameForOrientation:(UIDeviceOrientation)orientation
{
    sf::Vec2u size = self.sfWindow->getSize();
    return (!UIDeviceOrientationIsLandscape(orientation) && size.x > size.y) ||
           (UIDeviceOrientationIsLandscape(orientation) && size.y > size.x);
}

////////////////////////////////////////////////////////////
- (void)deviceOrientationDidChange:(NSNotification*)notification
{
    if (self.sfWindow)
    {
        // Get the new orientation
        UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
        // Filter interesting orientations
        if (UIDeviceOrientationIsValidInterfaceOrientation(orientation))
        {
            // Get the new size
            sf::Vec2u size = self.sfWindow->getSize();
            // Check if the app can switch to this orientation and if so if the window's size must be adjusted
            if ([self supportsOrientation:orientation] && [self needsToFlipFrameForOrientation:orientation])
                std::swap(size.x, size.y);

            // Send a Resized event to the current window
            sfWindow->forwardEvent(sf::Event::Resized{size});
        }
    }
}

////////////////////////////////////////////////////////////
- (void)setVirtualKeyboardVisible:(bool)visible
{
    if (self.sfWindow)
        self.sfWindow->setVirtualKeyboardVisible(visible);
}

////////////////////////////////////////////////////////////
- (sf::Vec2i)getTouchPosition:(unsigned int)index
{
    if (index < touchPositions.size())
        return touchPositions[index];

    return sf::Vec2i(-1, -1);
}

////////////////////////////////////////////////////////////
- (void)notifyTouchBegin:(unsigned int)index atPosition:(sf::Vec2i)position
{
    position.x *= static_cast<int>(backingScaleFactor);
    position.y *= static_cast<int>(backingScaleFactor);

    // save the touch position
    if (index >= touchPositions.size())
        touchPositions.resize(index + 1, sf::Vec2i(-1, -1));
    touchPositions[index] = position;

    // notify the event to the application window
    if (self.sfWindow)
        sfWindow->forwardEvent(sf::Event::TouchBegan{index, position});
}

////////////////////////////////////////////////////////////
- (void)notifyTouchMove:(unsigned int)index atPosition:(sf::Vec2i)position
{
    position.x *= static_cast<int>(backingScaleFactor);
    position.y *= static_cast<int>(backingScaleFactor);

    // save the touch position
    if (index >= touchPositions.size())
        touchPositions.resize(index + 1, sf::Vec2i(-1, -1));
    touchPositions[index] = position;

    // notify the event to the application window
    if (self.sfWindow)
        sfWindow->forwardEvent(sf::Event::TouchMoved{index, position});
}

////////////////////////////////////////////////////////////
- (void)notifyTouchEnd:(unsigned int)index atPosition:(sf::Vec2i)position
{
    // clear the touch position
    if (index < touchPositions.size())
        touchPositions[index] = sf::Vec2i(-1, -1);

    // notify the event to the application window
    if (self.sfWindow)
        sfWindow->forwardEvent(sf::Event::TouchEnded{index, position * static_cast<int>(backingScaleFactor)});
}

////////////////////////////////////////////////////////////
- (void)notifyCharacter:(char32_t)character
{
    if (self.sfWindow)
        sfWindow->forwardEvent(sf::Event::TextEntered{character});
}

@end
