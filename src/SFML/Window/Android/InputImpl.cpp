#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"

#include "SFML/System/Android/Activity.hpp"
#include "SFML/System/Err.hpp"

#include <jni.h>

#include <mutex>


namespace sf::priv::InputImpl
{
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Key /* key */)
{
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Scancode /* codes */)
{
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
Keyboard::Key localize(Keyboard::Scancode /* code */)
{
    // Not applicable
    return Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
Keyboard::Scancode delocalize(Keyboard::Key /* key */)
{
    // Not applicable
    return Keyboard::Scan::Unknown;
}


////////////////////////////////////////////////////////////
String getDescription(Keyboard::Scancode /* code */)
{
    // Not applicable
    return "";
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(bool visible)
{
    // TODO P2: Check if the window is active

    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);

    // Initializes JNI
    const jint lFlags = 0;

    JavaVM* lJavaVM = states.activity->vm;
    JNIEnv* lJNIEnv = states.activity->env;

    JavaVMAttachArgs lJavaVMAttachArgs;
    lJavaVMAttachArgs.version = JNI_VERSION_1_6;
    lJavaVMAttachArgs.name    = "NativeThread";
    lJavaVMAttachArgs.group   = nullptr;

    const jint lResult = lJavaVM->AttachCurrentThread(&lJNIEnv, &lJavaVMAttachArgs);

    if (lResult == JNI_ERR)
        priv::err() << "Failed to initialize JNI, couldn't switch the keyboard visibility";

    // Retrieves NativeActivity
    jobject lNativeActivity     = states.activity->clazz;
    jclass  classNativeActivity = lJNIEnv->GetObjectClass(lNativeActivity);

    // Retrieves Context.INPUT_METHOD_SERVICE
    jclass   classContext            = lJNIEnv->FindClass("android/content/Context");
    jfieldID fieldInputMethodService = lJNIEnv->GetStaticFieldID(classContext,
                                                                 "INPUT_METHOD_SERVICE",
                                                                 "Ljava/lang/String;");
    jobject  inputMethodService      = lJNIEnv->GetStaticObjectField(classContext, fieldInputMethodService);
    lJNIEnv->DeleteLocalRef(classContext);

    // Runs getSystemService(Context.INPUT_METHOD_SERVICE)
    jclass    classInputMethodManager = lJNIEnv->FindClass("android/view/inputmethod/InputMethodManager");
    jmethodID methodGetSystemService  = lJNIEnv->GetMethodID(classNativeActivity,
                                                            "getSystemService",
                                                            "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject lInputMethodManager = lJNIEnv->CallObjectMethod(lNativeActivity, methodGetSystemService, inputMethodService);
    lJNIEnv->DeleteLocalRef(inputMethodService);

    // Runs getWindow().getDecorView()
    jmethodID methodGetWindow    = lJNIEnv->GetMethodID(classNativeActivity, "getWindow", "()Landroid/view/Window;");
    jobject   lWindow            = lJNIEnv->CallObjectMethod(lNativeActivity, methodGetWindow);
    jclass    classWindow        = lJNIEnv->FindClass("android/view/Window");
    jmethodID methodGetDecorView = lJNIEnv->GetMethodID(classWindow, "getDecorView", "()Landroid/view/View;");
    jobject   lDecorView         = lJNIEnv->CallObjectMethod(lWindow, methodGetDecorView);
    lJNIEnv->DeleteLocalRef(lWindow);
    lJNIEnv->DeleteLocalRef(classWindow);

    if (visible)
    {
        // Runs lInputMethodManager.showSoftInput(...)
        jmethodID methodShowSoftInput = lJNIEnv->GetMethodID(classInputMethodManager,
                                                             "showSoftInput",
                                                             "(Landroid/view/View;I)Z");
        lJNIEnv->CallBooleanMethod(lInputMethodManager, methodShowSoftInput, lDecorView, lFlags);
    }
    else
    {
        // Runs lWindow.getViewToken()
        jclass    classView            = lJNIEnv->FindClass("android/view/View");
        jmethodID methodGetWindowToken = lJNIEnv->GetMethodID(classView, "getWindowToken", "()Landroid/os/IBinder;");
        jobject   lBinder              = lJNIEnv->CallObjectMethod(lDecorView, methodGetWindowToken);
        lJNIEnv->DeleteLocalRef(classView);

        // lInputMethodManager.hideSoftInput(...)
        jmethodID methodHideSoftInput = lJNIEnv->GetMethodID(classInputMethodManager,
                                                             "hideSoftInputFromWindow",
                                                             "(Landroid/os/IBinder;I)Z");
        lJNIEnv->CallBooleanMethod(lInputMethodManager, methodHideSoftInput, lBinder, lFlags);
        lJNIEnv->DeleteLocalRef(lBinder);
    }
    lJNIEnv->DeleteLocalRef(classNativeActivity);
    lJNIEnv->DeleteLocalRef(classInputMethodManager);
    lJNIEnv->DeleteLocalRef(lDecorView);

    // Finished with the JVM
    lJavaVM->DetachCurrentThread();
}


////////////////////////////////////////////////////////////
bool isMouseButtonPressed(Mouse::Button button)
{
    while (ALooper_pollOnce(0, nullptr, nullptr, nullptr) >= 0)
        ;

    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);

    return states.isButtonPressed[button];
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition()
{
    while (ALooper_pollOnce(0, nullptr, nullptr, nullptr) >= 0)
        ;

    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);

    return states.mousePosition;
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition(const WindowBase& /* relativeTo */)
{
    return getMousePosition();
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i /* position */)
{
    // Injecting events is impossible on Android
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position, const WindowBase& /* relativeTo */)
{
    setMousePosition(position);
}


////////////////////////////////////////////////////////////
bool isTouchDown(unsigned int finger)
{
    while (ALooper_pollOnce(0, nullptr, nullptr, nullptr) >= 0)
        ;

    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);

    return states.touchEvents.find(static_cast<int>(finger)) != states.touchEvents.end();
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger)
{
    while (ALooper_pollOnce(0, nullptr, nullptr, nullptr) >= 0)
        ;

    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);

    return states.touchEvents.find(static_cast<int>(finger))->second;
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger, const WindowBase& /* relativeTo */)
{
    return getTouchPosition(finger);
}

} // namespace sf::priv::InputImpl
