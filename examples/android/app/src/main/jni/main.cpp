#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/Sprite.hpp"
#include "Zancle/Graphics/Text.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/View.hpp"

#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/EventUtils.hpp"
#include "Zancle/Window/VideoModeUtils.hpp"

#include "Zancle/System/Path.hpp"
#include "Zancle/System/Thread.hpp"

#include "ZancleBase/Optional.hpp"

// Do we want to showcase direct JNI/NDK interaction?
// Undefine this to get real cross-platform code.
// Uncomment this to try JNI access; this seems to be broken in latest NDKs
// #define USE_JNI

#if defined(USE_JNI)
    // These headers are only needed for direct NDK/JDK interaction
    #include <android/native_activity.h>
    #include <jni.h>

    // Since we want to get the native activity from Zancle, we'll have to use an
    // extra header here:
    #include "Zancle/System/NativeActivity.hpp"

// NDK/JNI sub example - call Java code from native code
int vibrate(za::Time duration)
{
    // First we'll need the native activity handle
    ANativeActivity& activity = *za::getNativeActivity();

    // Retrieve the JVM and JNI environment
    JavaVM& vm  = *activity.vm;
    JNIEnv& env = *activity.env;

    // First, attach this thread to the main thread
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name    = "NativeThread";
    attachargs.group   = nullptr;
    jint res           = vm.AttachCurrentThread(&env, &attachargs);

    if (res == JNI_ERR)
        return EXIT_FAILURE;

    // Retrieve class information
    jclass natact  = env.FindClass("android/app/NativeActivity");
    jclass context = env.FindClass("android/content/Context");

    // Get the value of a constant
    jfieldID fid    = env.GetStaticFieldID(context, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    jobject  svcstr = env.GetStaticObjectField(context, fid);

    // Get the method 'getSystemService' and call it
    jmethodID getss   = env.GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject   vib_obj = env.CallObjectMethod(activity.clazz, getss, svcstr);

    // Get the object's class and retrieve the member name
    jclass    vib_cls = env.GetObjectClass(vib_obj);
    jmethodID vibrate = env.GetMethodID(vib_cls, "vibrate", "(J)V");

    // Determine the timeframe
    jlong length = duration.asMilliseconds();

    // Bzzz!
    env.CallVoidMethod(vib_obj, vibrate, length);

    // Free references
    env.DeleteLocalRef(vib_obj);
    env.DeleteLocalRef(vib_cls);
    env.DeleteLocalRef(svcstr);
    env.DeleteLocalRef(context);
    env.DeleteLocalRef(natact);

    // Detach thread again
    vm.DetachCurrentThread();
}
#endif

// This is the actual Android example. You don't have to write any platform
// specific code, unless you want to use things not directly exposed.
// ('vibrate()' in this example; undefine 'USE_JNI' above to disable it)
int main(int, char**)
{
    // Create the graphics context
    auto graphicsContext = za::GraphicsContext::create().value();

    const auto [size, bitsPerPixel] = za::VideoModeUtils::getDesktopMode();

    auto window = za::RenderWindow::create({.size = size, .bitsPerPixel = bitsPerPixel, .framerateLimit = 30}).value();
    const auto defaultView = window.getView();

    const auto texture = za::Texture::loadFromFile("image.png").value();

    za::Sprite image{.textureRect = texture.getRect()};
    image.position = size.toVec2f() / 2.f;
    image.origin   = texture.getSize().toVec2f() / 2.f;

    const auto font = za::Font::openFromFile("tuffy.ttf").value();

    za::Text text(font, {.string = "Tap anywhere to move the logo.", .characterSize = 64u});
    text.setFillColor(za::Color::Black);
    text.position = {10, 10};

    za::View view = defaultView;

    za::Color background = za::Color::White;

    // We shouldn't try drawing to the screen while in background
    // so we'll have to track that. You can do minor background
    // work, but keep battery life in mind.
    bool active = true;

    while (true)
    {
        while (const zb::Optional event = active ? window.pollEvent() : window.waitEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (const auto* resized = event->getIf<za::Event::Resized>())
            {
                const auto fSize = resized->size.toVec2f();
                view.size        = fSize;
                view.center      = fSize / 2.f;
                window.setView(view);
            }
            else if (event->is<za::Event::FocusLost>())
            {
                background = za::Color::Black;
            }
            else if (event->is<za::Event::FocusGained>())
            {
                background = za::Color::White;
            }
            // On Android MouseLeft/MouseEntered are (for now) triggered,
            // whenever the app loses or gains focus.
            else if (event->is<za::Event::MouseLeft>())
            {
                active = false;
            }
            else if (event->is<za::Event::MouseEntered>())
            {
                active = true;
            }
            else if (const auto* touchBegan = event->getIf<za::Event::TouchBegan>())
            {
                if (touchBegan->finger == 0)
                {
                    image.position = touchBegan->position.toVec2f();
#if defined(USE_JNI)
                    vibrate(za::milliseconds(10));
#endif
                }
            }
        }

        if (active)
        {
            window.clear(background);
            window.draw(image, texture);
            window.draw(text);
            window.display();
        }
        else
        {
            za::ThisThread::sleepFor(za::milliseconds(100));
        }
    }
}
