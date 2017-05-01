////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2017 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Android/ClipboardImpl.hpp>
#include <SFML/System/Android/Activity.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Lock.hpp>
#include <jni.h>

namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
String ClipboardImpl::getString()
{
    String content;

    ActivityStates* states = getActivity(NULL);
    Lock lock(states->mutex);

    // Initializes JNI
    jint lResult;
    jint lFlags = 0;

    JavaVM* lJavaVM = states->activity->vm;
    JNIEnv* lJNIEnv = states->activity->env;

    JavaVMAttachArgs lJavaVMAttachArgs;
    lJavaVMAttachArgs.version = JNI_VERSION_1_6;
    lJavaVMAttachArgs.name = "NativeThread";
    lJavaVMAttachArgs.group = NULL;

    lResult=lJavaVM->AttachCurrentThread(&lJNIEnv, &lJavaVMAttachArgs);

    if (lResult == JNI_ERR)
        err() << "Failed to initialize JNI, couldn't switch the keyboard visibility" << std::endl;

    // Retrieves NativeActivity
    jobject lNativeActivity = states->activity->clazz;
    jclass ClassNativeActivity = lJNIEnv->GetObjectClass(lNativeActivity);

    // Retrieves Context.CLIPBOARD_SERVICE
    jclass ClassContext = lJNIEnv->FindClass("android/content/Context");
    jfieldID FieldCLIPBOARD_SERVICE = lJNIEnv->GetStaticFieldID(ClassContext,
        "CLIPBOARD_SERVICE", "Ljava/lang/String;");
    jobject CLIPBOARD_SERVICE = lJNIEnv->GetStaticObjectField(ClassContext,
        FieldCLIPBOARD_SERVICE);
    lJNIEnv->DeleteLocalRef(ClassContext);

    // Runs getSystemService(Context.CLIPBOARD_SERVICE)
    jclass ClassInputMethodManager =
        lJNIEnv->FindClass("android/view/inputmethod/InputMethodManager");
    jmethodID MethodGetSystemService = lJNIEnv->GetMethodID(ClassNativeActivity,
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject lInputMethodManager = lJNIEnv->CallObjectMethod(lNativeActivity,
        MethodGetSystemService, CLIPBOARD_SERVICE);
    lJNIEnv->DeleteLocalRef(CLIPBOARD_SERVICE);

    /*if (visible)
    {
        // Runs lInputMethodManager.showSoftInput(...)
        jmethodID MethodShowSoftInput = lJNIEnv->GetMethodID(ClassInputMethodManager,
            "showSoftInput", "(Landroid/view/View;I)Z");
        jboolean lResult = lJNIEnv->CallBooleanMethod(lInputMethodManager,
            MethodShowSoftInput, lDecorView, lFlags);
    }
    else
    {
        // Runs lWindow.getViewToken()
        jclass ClassView = lJNIEnv->FindClass("android/view/View");
        jmethodID MethodGetWindowToken = lJNIEnv->GetMethodID(ClassView,
            "getWindowToken", "()Landroid/os/IBinder;");
        jobject lBinder = lJNIEnv->CallObjectMethod(lDecorView,
            MethodGetWindowToken);
        lJNIEnv->DeleteLocalRef(ClassView);

        // lInputMethodManager.hideSoftInput(...)
        jmethodID MethodHideSoftInput = lJNIEnv->GetMethodID(ClassInputMethodManager,
            "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
        jboolean lRes = lJNIEnv->CallBooleanMethod(lInputMethodManager,
            MethodHideSoftInput, lBinder, lFlags);
        lJNIEnv->DeleteLocalRef(lBinder);
    }*/
    lJNIEnv->DeleteLocalRef(ClassNativeActivity);
    lJNIEnv->DeleteLocalRef(ClassInputMethodManager);

    // Finished with the JVM
    lJavaVM->DetachCurrentThread();

    return content;
}


////////////////////////////////////////////////////////////
void ClipboardImpl::setString(const String& text)
{
    sf::err() << "Clipboard API not implemented for Android.\n";
}

} // namespace priv

} // namespace sf
