#include <Zancle/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/InputImpl.hpp"

#include "Zancle/System/Android/Activity.hpp"
#include "Zancle/System/Err.hpp"

#include <jni.h>

#include <mutex>


namespace za::priv::InputImpl
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
String getDescription(Keyboard::Scancode code)
{
    switch (code)
    {
            // clang-format off
        case za::Keyboard::Scan::A:                 return "A";
        case za::Keyboard::Scan::B:                 return "B";
        case za::Keyboard::Scan::C:                 return "C";
        case za::Keyboard::Scan::D:                 return "D";
        case za::Keyboard::Scan::E:                 return "E";
        case za::Keyboard::Scan::F:                 return "F";
        case za::Keyboard::Scan::G:                 return "G";
        case za::Keyboard::Scan::H:                 return "H";
        case za::Keyboard::Scan::I:                 return "I";
        case za::Keyboard::Scan::J:                 return "J";
        case za::Keyboard::Scan::K:                 return "K";
        case za::Keyboard::Scan::L:                 return "L";
        case za::Keyboard::Scan::M:                 return "M";
        case za::Keyboard::Scan::N:                 return "N";
        case za::Keyboard::Scan::O:                 return "O";
        case za::Keyboard::Scan::P:                 return "P";
        case za::Keyboard::Scan::Q:                 return "Q";
        case za::Keyboard::Scan::R:                 return "R";
        case za::Keyboard::Scan::S:                 return "S";
        case za::Keyboard::Scan::T:                 return "T";
        case za::Keyboard::Scan::U:                 return "U";
        case za::Keyboard::Scan::V:                 return "V";
        case za::Keyboard::Scan::W:                 return "W";
        case za::Keyboard::Scan::X:                 return "X";
        case za::Keyboard::Scan::Y:                 return "Y";
        case za::Keyboard::Scan::Z:                 return "Z";
        case za::Keyboard::Scan::Num1:              return "1";
        case za::Keyboard::Scan::Num2:              return "2";
        case za::Keyboard::Scan::Num3:              return "3";
        case za::Keyboard::Scan::Num4:              return "4";
        case za::Keyboard::Scan::Num5:              return "5";
        case za::Keyboard::Scan::Num6:              return "6";
        case za::Keyboard::Scan::Num7:              return "7";
        case za::Keyboard::Scan::Num8:              return "8";
        case za::Keyboard::Scan::Num9:              return "9";
        case za::Keyboard::Scan::Num0:              return "0";
        case za::Keyboard::Scan::Enter:             return "Enter";
        case za::Keyboard::Scan::Escape:            return "Escape";
        case za::Keyboard::Scan::Backspace:         return "Backspace";
        case za::Keyboard::Scan::Tab:               return "Tab";
        case za::Keyboard::Scan::Space:             return "Space";
        case za::Keyboard::Scan::Hyphen:            return "-";
        case za::Keyboard::Scan::Equal:             return "=";
        case za::Keyboard::Scan::LBracket:          return "[";
        case za::Keyboard::Scan::RBracket:          return "]";
        case za::Keyboard::Scan::Backslash:         return "\\";
        case za::Keyboard::Scan::Semicolon:         return ";";
        case za::Keyboard::Scan::Apostrophe:        return "'";
        case za::Keyboard::Scan::Grave:             return "`";
        case za::Keyboard::Scan::Comma:             return ",";
        case za::Keyboard::Scan::Period:            return ".";
        case za::Keyboard::Scan::Slash:             return "/";
        case za::Keyboard::Scan::F1:                return "F1";
        case za::Keyboard::Scan::F2:                return "F2";
        case za::Keyboard::Scan::F3:                return "F3";
        case za::Keyboard::Scan::F4:                return "F4";
        case za::Keyboard::Scan::F5:                return "F5";
        case za::Keyboard::Scan::F6:                return "F6";
        case za::Keyboard::Scan::F7:                return "F7";
        case za::Keyboard::Scan::F8:                return "F8";
        case za::Keyboard::Scan::F9:                return "F9";
        case za::Keyboard::Scan::F10:               return "F10";
        case za::Keyboard::Scan::F11:               return "F11";
        case za::Keyboard::Scan::F12:               return "F12";
        case za::Keyboard::Scan::F13:               return "F13";
        case za::Keyboard::Scan::F14:               return "F14";
        case za::Keyboard::Scan::F15:               return "F15";
        case za::Keyboard::Scan::F16:               return "F16";
        case za::Keyboard::Scan::F17:               return "F17";
        case za::Keyboard::Scan::F18:               return "F18";
        case za::Keyboard::Scan::F19:               return "F19";
        case za::Keyboard::Scan::F20:               return "F20";
        case za::Keyboard::Scan::F21:               return "F21";
        case za::Keyboard::Scan::F22:               return "F22";
        case za::Keyboard::Scan::F23:               return "F23";
        case za::Keyboard::Scan::F24:               return "F24";
        case za::Keyboard::Scan::CapsLock:          return "Caps Lock";
        case za::Keyboard::Scan::PrintScreen:       return "Print Screen";
        case za::Keyboard::Scan::ScrollLock:        return "Scroll Lock";
        case za::Keyboard::Scan::Pause:             return "Pause";
        case za::Keyboard::Scan::Insert:            return "Insert";
        case za::Keyboard::Scan::Home:              return "Home";
        case za::Keyboard::Scan::PageUp:            return "Page Up";
        case za::Keyboard::Scan::Delete:            return "Delete";
        case za::Keyboard::Scan::End:               return "End";
        case za::Keyboard::Scan::PageDown:          return "Page Down";
        case za::Keyboard::Scan::Right:             return "Right Arrow";
        case za::Keyboard::Scan::Left:              return "Left Arrow";
        case za::Keyboard::Scan::Down:              return "Down Arrow";
        case za::Keyboard::Scan::Up:                return "Up Arrow";
        case za::Keyboard::Scan::NumLock:           return "Num Lock";
        case za::Keyboard::Scan::NumpadDivide:      return "Numpad /";
        case za::Keyboard::Scan::NumpadMultiply:    return "Numpad *";
        case za::Keyboard::Scan::NumpadMinus:       return "Numpad -";
        case za::Keyboard::Scan::NumpadPlus:        return "Numpad +";
        case za::Keyboard::Scan::NumpadEqual:       return "Numpad =";
        case za::Keyboard::Scan::NumpadEnter:       return "Numpad Enter";
        case za::Keyboard::Scan::NumpadDecimal:     return "Numpad .";
        case za::Keyboard::Scan::Numpad1:           return "Numpad 1";
        case za::Keyboard::Scan::Numpad2:           return "Numpad 2";
        case za::Keyboard::Scan::Numpad3:           return "Numpad 3";
        case za::Keyboard::Scan::Numpad4:           return "Numpad 4";
        case za::Keyboard::Scan::Numpad5:           return "Numpad 5";
        case za::Keyboard::Scan::Numpad6:           return "Numpad 6";
        case za::Keyboard::Scan::Numpad7:           return "Numpad 7";
        case za::Keyboard::Scan::Numpad8:           return "Numpad 8";
        case za::Keyboard::Scan::Numpad9:           return "Numpad 9";
        case za::Keyboard::Scan::Numpad0:           return "Numpad 0";
        case za::Keyboard::Scan::NonUsBackslash:    return "Non-US Backslash";
        case za::Keyboard::Scan::Application:       return "Application";
        case za::Keyboard::Scan::Execute:           return "Execute";
        case za::Keyboard::Scan::ModeChange:        return "Mode Change";
        case za::Keyboard::Scan::Help:              return "Help";
        case za::Keyboard::Scan::Menu:              return "Menu";
        case za::Keyboard::Scan::Select:            return "Select";
        case za::Keyboard::Scan::Redo:              return "Redo";
        case za::Keyboard::Scan::Undo:              return "Undo";
        case za::Keyboard::Scan::Cut:               return "Cut";
        case za::Keyboard::Scan::Copy:              return "Copy";
        case za::Keyboard::Scan::Paste:             return "Paste";
        case za::Keyboard::Scan::VolumeMute:        return "Volume Mute";
        case za::Keyboard::Scan::VolumeUp:          return "Volume Up";
        case za::Keyboard::Scan::VolumeDown:        return "Volume Down";
        case za::Keyboard::Scan::MediaPlayPause:    return "Media Play/Pause";
        case za::Keyboard::Scan::MediaStop:         return "Media Stop";
        case za::Keyboard::Scan::MediaNextTrack:    return "Media Next Track";
        case za::Keyboard::Scan::MediaPreviousTrack:return "Media Previous Track";
        case za::Keyboard::Scan::LControl:          return "Left Control";
        case za::Keyboard::Scan::LShift:            return "Left Shift";
        case za::Keyboard::Scan::LAlt:              return "Left Alt";
        case za::Keyboard::Scan::LSystem:           return "Left System";
        case za::Keyboard::Scan::RControl:          return "Right Control";
        case za::Keyboard::Scan::RShift:            return "Right Shift";
        case za::Keyboard::Scan::RAlt:              return "Right Alt";
        case za::Keyboard::Scan::RSystem:           return "Right System";
        case za::Keyboard::Scan::Back:              return "Back";
        case za::Keyboard::Scan::Forward:           return "Forward";
        case za::Keyboard::Scan::Refresh:           return "Refresh";
        case za::Keyboard::Scan::Stop:              return "Stop";
        case za::Keyboard::Scan::Search:            return "Search";
        case za::Keyboard::Scan::Favorites:         return "Favorites";
        case za::Keyboard::Scan::HomePage:          return "Home Page";
        case za::Keyboard::Scan::LaunchApplication1:return "Launch Application 1";
        case za::Keyboard::Scan::LaunchApplication2:return "Launch Application 2";
        case za::Keyboard::Scan::LaunchMail:        return "Launch Mail";
        case za::Keyboard::Scan::LaunchMediaSelect: return "Launch Media Select";
        default: return "Unknown key";
            // clang-format on
    }
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


// TODO P2: code repetition
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

} // namespace za::priv::InputImpl
