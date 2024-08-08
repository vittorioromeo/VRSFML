#include <SFML/Config.hpp>

#include <SFML/ImGui/ImGui.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Cursor.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/Touch.hpp>
#include <SFML/Window/Window.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/StringUtfUtils.hpp>

#include <SFML/OpenGL.hpp>
#include <algorithm>
#include <imgui.h>
#include <memory>
#include <vector>

#include <cassert>
#include <cmath>   // abs
#include <cstring> // memcpy

#if defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef ANDROID
#ifdef USE_JNI

#include <SFML/System/NativeActivity.hpp>

#include <android/native_activity.h>
#include <jni.h>

int openKeyboardIME()
{
    ANativeActivity* activity = sf::getNativeActivity();
    JavaVM*          vm       = activity->vm;
    JNIEnv*          env      = activity->env;
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name    = "NativeThread";
    attachargs.group   = nullptr;
    jint res           = vm->AttachCurrentThread(&env, &attachargs);
    if (res == JNI_ERR)
        return EXIT_FAILURE;

    jclass natact  = env->FindClass("android/app/NativeActivity");
    jclass context = env->FindClass("android/content/Context");

    jfieldID fid    = env->GetStaticFieldID(context, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
    jobject  svcstr = env->GetStaticObjectField(context, fid);

    jmethodID getss   = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject   imm_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);

    jclass    imm_cls         = env->GetObjectClass(imm_obj);
    jmethodID toggleSoftInput = env->GetMethodID(imm_cls, "toggleSoftInput", "(II)V");

    env->CallVoidMethod(imm_obj, toggleSoftInput, 2, 0);

    env->DeleteLocalRef(imm_obj);
    env->DeleteLocalRef(imm_cls);
    env->DeleteLocalRef(svcstr);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(natact);

    vm->DetachCurrentThread();

    return EXIT_SUCCESS;
}

int closeKeyboardIME()
{
    ANativeActivity* activity = sf::getNativeActivity();
    JavaVM*          vm       = activity->vm;
    JNIEnv*          env      = activity->env;
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name    = "NativeThread";
    attachargs.group   = nullptr;
    jint res           = vm->AttachCurrentThread(&env, &attachargs);
    if (res == JNI_ERR)
        return EXIT_FAILURE;

    jclass natact  = env->FindClass("android/app/NativeActivity");
    jclass context = env->FindClass("android/content/Context");

    jfieldID fid    = env->GetStaticFieldID(context, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
    jobject  svcstr = env->GetStaticObjectField(context, fid);

    jmethodID getss   = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject   imm_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);

    jclass    imm_cls         = env->GetObjectClass(imm_obj);
    jmethodID toggleSoftInput = env->GetMethodID(imm_cls, "toggleSoftInput", "(II)V");

    env->CallVoidMethod(imm_obj, toggleSoftInput, 1, 0);

    env->DeleteLocalRef(imm_obj);
    env->DeleteLocalRef(imm_cls);
    env->DeleteLocalRef(svcstr);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(natact);

    vm->DetachCurrentThread();

    return EXIT_SUCCESS;
}

#endif
#endif

// NOLINTBEGIN

#define IMGUI_IMPL_OPENGL_ES3
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY


#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include <stdint.h> // intptr_t
#include <stdio.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option" // warning: ignore unknown flags
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wunused-macros"          // warning: macro is not used
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#pragma clang diagnostic ignored "-Wcast-function-type" // warning: cast between incompatible function types (for loader)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"                // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunknown-warning-option" // warning: unknown warning group 'xxx'
#pragma GCC diagnostic ignored "-Wcast-function-type" // warning: cast between incompatible function types (for loader)
#endif

// GL includes
#if defined(IMGUI_IMPL_OPENGL_ES2)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES2/gl.h> // Use GL ES 2
#else
#include <GLES2/gl2.h> // Use GL ES 2
#endif
#if defined(__EMSCRIPTEN__)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#endif
#elif defined(IMGUI_IMPL_OPENGL_ES3)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES3/gl.h> // Use GL ES 3
#else
#include <GLES3/gl3.h> // Use GL ES 3
#endif
#elif !defined(IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
// Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
// Helper libraries are often used for this purpose! Here we are using our own minimal custom loader based on gl3w.
// In the rest of your app/engine, you can use another loader of your choice (gl3w, glew, glad, glbinding, glext, glLoadGen, etc.).
// If you happen to be developing a new feature for this backend (imgui_impl_opengl3.cpp):
// - You may need to regenerate imgui_impl_opengl3_loader.h to add new symbols. See https://github.com/dearimgui/gl3w_stripped
// - You can temporarily use an unstripped version. See https://github.com/dearimgui/gl3w_stripped/releases
// Changes to this backend using new APIs should be accompanied by a regenerated stripped loader version.
#define IMGL3W_IMPL
#include "imgui_impl_opengl3_loader.h"
#endif

// Vertex arrays are not supported on ES2/WebGL1 unless Emscripten which uses an extension
#ifndef IMGUI_IMPL_OPENGL_ES2
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#elif defined(__EMSCRIPTEN__)
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#define glBindVertexArray       glBindVertexArrayOES
#define glGenVertexArrays       glGenVertexArraysOES
#define glDeleteVertexArrays    glDeleteVertexArraysOES
#define GL_VERTEX_ARRAY_BINDING GL_VERTEX_ARRAY_BINDING_OES
#endif

// Desktop GL 2.0+ has extension and glPolygonMode() which GL ES and WebGL don't have..
// A desktop ES context can technically compile fine with our loader, so we also perform a runtime checks
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
#define IMGUI_IMPL_OPENGL_HAS_EXTENSIONS        // has glGetIntegerv(GL_NUM_EXTENSIONS)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE // may have glPolygonMode()
#endif

// Desktop GL 2.1+ and GL ES 3.0+ have glBindBuffer() with GL_PIXEL_UNPACK_BUFFER target.
#if !defined(IMGUI_IMPL_OPENGL_ES2)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_BUFFER_PIXEL_UNPACK
#endif

// Desktop GL 3.1+ has GL_PRIMITIVE_RESTART state
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_1)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
#endif

// Desktop GL 3.2+ has glDrawElementsBaseVertex() which GL ES and WebGL don't have.
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_2)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
#endif

// Desktop GL 3.3+ and GL ES 3.0+ have glBindSampler()
#if !defined(IMGUI_IMPL_OPENGL_ES2) && (defined(IMGUI_IMPL_OPENGL_ES3) || defined(GL_VERSION_3_3))
#define IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
#endif

// OpenGL Data
struct ImGui_ImplOpenGL3_Data
{
    GLuint GlVersion; // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
    char   GlslVersionString[32]; // Specified by user or detected based on compile time GL settings.
    bool   GlProfileIsES2;
    bool   GlProfileIsES3;
    bool   GlProfileIsCompat;
    GLint  GlProfileMask;
    GLuint FontTexture;
    GLuint ShaderHandle;
    GLint  AttribLocationTex; // Uniforms location
    GLint  AttribLocationProjMtx;
    GLuint AttribLocationVtxPos; // Vertex attributes location
    GLuint AttribLocationVtxUV;
    GLuint AttribLocationVtxColor;
    unsigned int VboHandle, ElementsHandle;
    GLsizeiptr   VertexBufferSize;
    GLsizeiptr   IndexBufferSize;
    bool         HasPolygonMode;
    bool         HasClipOrigin;
    bool         UseBufferSubData;

    ImGui_ImplOpenGL3_Data()
    {
        memset((void*)this, 0, sizeof(*this));
    }
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplOpenGL3_Data* ImGui_ImplOpenGL3_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL3_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

// OpenGL vertex attribute state (for ES 1.0 and ES 2.0 only)
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
struct ImGui_ImplOpenGL3_VtxAttribState
{
    GLint   Enabled, Size, Type, Normalized, Stride;
    GLvoid* Ptr;

    void GetState(GLint index)
    {
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &Enabled);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &Size);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &Type);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &Normalized);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &Stride);
        glGetVertexAttribPointerv(index, GL_VERTEX_ATTRIB_ARRAY_POINTER, &Ptr);
    }
    void SetState(GLint index)
    {
        glVertexAttribPointer(index, Size, Type, (GLboolean)Normalized, Stride, Ptr);
        if (Enabled)
            glEnableVertexAttribArray(index);
        else
            glDisableVertexAttribArray(index);
    }
};
#endif

// Functions
bool ImGui_ImplOpenGL3_Init(const char* glsl_version)
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Initialize our loader
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && !defined(IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
    if (imgl3wInit() != 0)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }
#endif

    // Setup backend capabilities flags
    ImGui_ImplOpenGL3_Data* bd = IM_NEW(ImGui_ImplOpenGL3_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName     = "imgui_impl_opengl3";

    // Query for GL version (e.g. 320 for GL 3.2)
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GLES 2
    bd->GlVersion      = 200;
    bd->GlProfileIsES2 = true;
#else
    // Desktop or GLES 3
    const char* gl_version_str = (const char*)glGetString(GL_VERSION);
    GLint       major          = 0;
    GLint       minor          = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major == 0 && minor == 0)
        sscanf(gl_version_str, "%d.%d", &major, &minor); // Query GL_VERSION in desktop GL 2.x, the string will start with "<major>.<minor>"
    bd->GlVersion = (GLuint)(major * 100 + minor * 10);
#if defined(GL_CONTEXT_PROFILE_MASK)
    if (bd->GlVersion >= 320)
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &bd->GlProfileMask);
    bd->GlProfileIsCompat = (bd->GlProfileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) != 0;
#endif

#if defined(IMGUI_IMPL_OPENGL_ES3)
    bd->GlProfileIsES3 = true;
#else
    if (strncmp(gl_version_str, "OpenGL ES 3", 11) == 0)
        bd->GlProfileIsES3 = true;
#endif

    bd->UseBufferSubData = false;
    /*
    // Query vendor to enable glBufferSubData kludge
#ifdef _WIN32
    if (const char* vendor = (const char*)glGetString(GL_VENDOR))
        if (strncmp(vendor, "Intel", 5) == 0)
            bd->UseBufferSubData = true;
#endif
    */
#endif

#ifdef IMGUI_IMPL_OPENGL_DEBUG
    printf(
        "GlVersion = %d, \"%s\"\nGlProfileIsCompat = %d\nGlProfileMask = 0x%X\nGlProfileIsES2 = %d, GlProfileIsES3 = "
        "%d\nGL_VENDOR = '%s'\nGL_RENDERER = '%s'\n",
        bd->GlVersion,
        gl_version_str,
        bd->GlProfileIsCompat,
        bd->GlProfileMask,
        bd->GlProfileIsES2,
        bd->GlProfileIsES3,
        (const char*)glGetString(GL_VENDOR),
        (const char*)glGetString(GL_RENDERER)); // [DEBUG]
#endif

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
    if (bd->GlVersion >= 320)
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
#endif

    // Store GLSL version string so we can refer to it later in case we recreate shaders.
    // Note: GLSL version is NOT the same as GL version. Leave this to nullptr if unsure.
    if (glsl_version == nullptr)
    {
#if defined(IMGUI_IMPL_OPENGL_ES2)
        glsl_version = "#version 100";
#elif defined(IMGUI_IMPL_OPENGL_ES3)
        glsl_version = "#version 300 es";
#elif defined(__APPLE__)
        glsl_version = "#version 150";
#else
        glsl_version = "#version 130";
#endif
    }
    IM_ASSERT((int)strlen(glsl_version) + 2 < IM_ARRAYSIZE(bd->GlslVersionString));
    strcpy(bd->GlslVersionString, glsl_version);
    strcat(bd->GlslVersionString, "\n");

    // Make an arbitrary GL call (we don't actually need the result)
    // IF YOU GET A CRASH HERE: it probably means the OpenGL function loader didn't do its job. Let us know!
    GLint current_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);

    // Detect extensions we support
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    bd->HasPolygonMode = (!bd->GlProfileIsES2 && !bd->GlProfileIsES3);
#endif
    bd->HasClipOrigin = (bd->GlVersion >= 450);
#ifdef IMGUI_IMPL_OPENGL_HAS_EXTENSIONS
    GLint num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (GLint i = 0; i < num_extensions; i++)
    {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if (extension != nullptr && strcmp(extension, "GL_ARB_clip_control") == 0)
            bd->HasClipOrigin = true;
    }
#endif

    return true;
}

void ImGui_ImplOpenGL3_DestroyDeviceObjects();
bool ImGui_ImplOpenGL3_CreateDeviceObjects();
bool ImGui_ImplOpenGL3_CreateFontsTexture();

void ImGui_ImplOpenGL3_Shutdown()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    io.BackendRendererName     = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    IM_DELETE(bd);
}

void ImGui_ImplOpenGL3_NewFrame()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplOpenGL3_Init()?");

    if (!bd->ShaderHandle)
        ImGui_ImplOpenGL3_CreateDeviceObjects();
    if (!bd->FontTexture)
        ImGui_ImplOpenGL3_CreateFontsTexture();
}

static void ImGui_ImplOpenGL3_SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object)
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    glCheck(glEnable(GL_BLEND));
    glCheck(glBlendEquation(GL_FUNC_ADD));
    glCheck(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
    glCheck(glDisable(GL_CULL_FACE));
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glDisable(GL_STENCIL_TEST));
    glCheck(glEnable(GL_SCISSOR_TEST));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
    if (bd->GlVersion >= 310)
        glCheck(glDisable(GL_PRIMITIVE_RESTART));
#endif
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    if (bd->HasPolygonMode)
        glCheck(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
#endif

        // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
#if defined(GL_CLIP_ORIGIN)
    bool clip_origin_lower_left = true;
    if (bd->HasClipOrigin)
    {
        GLenum current_clip_origin = 0;
        glCheck(glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&current_clip_origin));
        if (current_clip_origin == GL_UPPER_LEFT)
            clip_origin_lower_left = false;
    }
#endif

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    glCheck(glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height));
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
#if defined(GL_CLIP_ORIGIN)
    if (!clip_origin_lower_left)
    {
        float tmp = T;
        T         = B;
        B         = tmp;
    } // Swap top and bottom if origin is upper left
#endif
    const float ortho_projection[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
    };
    glCheck(glUseProgram(bd->ShaderHandle));
    glCheck(glUniform1i(bd->AttribLocationTex, 0));
    glCheck(glUniformMatrix4fv(bd->AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]));

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
    if (bd->GlVersion >= 330 || bd->GlProfileIsES3)
        glCheck(glBindSampler(0, 0)); // We use combined texture/sampler state. Applications using GL 3.3 and GL ES 3.0 may set that otherwise.
#endif

    (void)vertex_array_object;
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glCheck(glBindVertexArray(vertex_array_object));
#endif

    // Bind vertex/index buffers and setup attributes for ImDrawVert
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, bd->VboHandle));
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bd->ElementsHandle));
    glCheck(glEnableVertexAttribArray(bd->AttribLocationVtxPos));
    glCheck(glEnableVertexAttribArray(bd->AttribLocationVtxUV));
    glCheck(glEnableVertexAttribArray(bd->AttribLocationVtxColor));
    glCheck(glVertexAttribPointer(bd->AttribLocationVtxPos,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  sizeof(ImDrawVert),
                                  (GLvoid*)offsetof(ImDrawVert, pos)));
    glCheck(
        glVertexAttribPointer(bd->AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv)));
    glCheck(glVertexAttribPointer(bd->AttribLocationVtxColor,
                                  4,
                                  GL_UNSIGNED_BYTE,
                                  GL_TRUE,
                                  sizeof(ImDrawVert),
                                  (GLvoid*)offsetof(ImDrawVert, col)));
}

// OpenGL3 Render function.
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly.
// This is in order to be able to run within an OpenGL engine that doesn't do so.
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width  = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    SFML_BASE_ASSERT(bd != nullptr);

    // Backup GL state
    GLenum last_active_texture;
    glCheck(glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture));
    glCheck(glActiveTexture(GL_TEXTURE0));
    GLuint last_program;
    glCheck(glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program));
    GLuint last_texture;
    glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
    GLuint last_sampler;
    if (bd->GlVersion >= 330 || bd->GlProfileIsES3)
    {
        glCheck(glGetIntegerv(GL_SAMPLER_BINDING, (GLint*)&last_sampler));
    }
    else
    {
        last_sampler = 0;
    }
#endif
    GLuint last_array_buffer;
    glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer));
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    // This is part of VAO on OpenGL 3.0+ and OpenGL ES 3.0+.
    GLint last_element_array_buffer;
    glCheck(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer));
    ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_pos;
    last_vtx_attrib_state_pos.GetState(bd->AttribLocationVtxPos);
    ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_uv;
    last_vtx_attrib_state_uv.GetState(bd->AttribLocationVtxUV);
    ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_color;
    last_vtx_attrib_state_color.GetState(bd->AttribLocationVtxColor);
#endif
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    GLuint last_vertex_array_object;
    glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object));
#endif
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    GLint last_polygon_mode[2];
    if (bd->HasPolygonMode)
    {
        glCheck(glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode));
    }
#endif
    GLint last_viewport[4];
    glCheck(glGetIntegerv(GL_VIEWPORT, last_viewport));
    GLint last_scissor_box[4];
    glCheck(glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box));
    GLenum last_blend_src_rgb;
    glCheck(glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb));
    GLenum last_blend_dst_rgb;
    glCheck(glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb));
    GLenum last_blend_src_alpha;
    glCheck(glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha));
    GLenum last_blend_dst_alpha;
    glCheck(glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha));
    GLenum last_blend_equation_rgb;
    glCheck(glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb));
    GLenum last_blend_equation_alpha;
    glCheck(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha));
    GLboolean last_enable_blend        = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face    = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test   = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
    GLboolean last_enable_primitive_restart = (bd->GlVersion >= 310) ? glIsEnabled(GL_PRIMITIVE_RESTART) : GL_FALSE;
#endif

    // Setup desired GL state
    // Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
    // The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
    GLuint vertex_array_object = 0;
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glCheck(glGenVertexArrays(1, &vertex_array_object));
#endif
    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        // - OpenGL drivers are in a very sorry state nowadays....
        //   During 2021 we attempted to switch from glBufferData() to orphaning+glBufferSubData() following reports
        //   of leaks on Intel GPU when using multi-viewports on Windows.
        // - After this we kept hearing of various display corruptions issues. We started disabling on non-Intel GPU, but issues still got reported on Intel.
        // - We are now back to using exclusively glBufferData(). So bd->UseBufferSubData IS ALWAYS FALSE in this code.
        //   We are keeping the old code path for a while in case people finding new issues may want to test the bd->UseBufferSubData path.
        // - See https://github.com/ocornut/imgui/issues/4468 and please report any corruption issues.
        const GLsizeiptr vtx_buffer_size = (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert);
        const GLsizeiptr idx_buffer_size = (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx);
        if (bd->UseBufferSubData)
        {
            if (bd->VertexBufferSize < vtx_buffer_size)
            {
                bd->VertexBufferSize = vtx_buffer_size;
                glCheck(glBufferData(GL_ARRAY_BUFFER, bd->VertexBufferSize, nullptr, GL_STREAM_DRAW));
            }
            if (bd->IndexBufferSize < idx_buffer_size)
            {
                bd->IndexBufferSize = idx_buffer_size;
                glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, bd->IndexBufferSize, nullptr, GL_STREAM_DRAW));
            }
            glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, vtx_buffer_size, (const GLvoid*)cmd_list->VtxBuffer.Data));
            glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, idx_buffer_size, (const GLvoid*)cmd_list->IdxBuffer.Data));
        }
        else
        {
            glCheck(glBufferData(GL_ARRAY_BUFFER, vtx_buffer_size, (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW));
            glCheck(
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_buffer_size, (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW));
        }

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                glCheck(glScissor((int)clip_min.x,
                                  (int)((float)fb_height - clip_max.y),
                                  (int)(clip_max.x - clip_min.x),
                                  (int)(clip_max.y - clip_min.y)));

                // Bind texture, Draw
                glCheck(glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID()));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
                if (bd->GlVersion >= 320)
                    glCheck(glDrawElementsBaseVertex(GL_TRIANGLES,
                                                     (GLsizei)pcmd->ElemCount,
                                                     sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                                     (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)),
                                                     (GLint)pcmd->VtxOffset));
                else
#endif
                    glCheck(glDrawElements(GL_TRIANGLES,
                                           (GLsizei)pcmd->ElemCount,
                                           sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                           (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx))));
            }
        }
    }

    // Destroy the temporary VAO
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glCheck(glDeleteVertexArrays(1, &vertex_array_object));
#endif

    // Restore modified GL state
    // This "glIsProgram()" check is required because if the program is "pending deletion" at the time of binding backup, it will have been deleted by now and will cause an OpenGL error. See #6220.
    if (last_program == 0 || glIsProgram(last_program))
        glCheck(glUseProgram(last_program));
    glCheck(glBindTexture(GL_TEXTURE_2D, last_texture));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
    if (bd->GlVersion >= 330 || bd->GlProfileIsES3)
        glCheck(glBindSampler(0, last_sampler));
#endif
    glCheck(glActiveTexture(last_active_texture));
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glCheck(glBindVertexArray(last_vertex_array_object));
#endif
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer));
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer));
    last_vtx_attrib_state_pos.SetState(bd->AttribLocationVtxPos);
    last_vtx_attrib_state_uv.SetState(bd->AttribLocationVtxUV);
    last_vtx_attrib_state_color.SetState(bd->AttribLocationVtxColor);
#endif
    glCheck(glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha));
    glCheck(glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha));
    if (last_enable_blend)
        glCheck(glEnable(GL_BLEND));
    else
        glCheck(glDisable(GL_BLEND));
    if (last_enable_cull_face)
        glCheck(glEnable(GL_CULL_FACE));
    else
        glCheck(glDisable(GL_CULL_FACE));
    if (last_enable_depth_test)
        glCheck(glEnable(GL_DEPTH_TEST));
    else
        glCheck(glDisable(GL_DEPTH_TEST));
    if (last_enable_stencil_test)
        glCheck(glEnable(GL_STENCIL_TEST));
    else
        glCheck(glDisable(GL_STENCIL_TEST));
    if (last_enable_scissor_test)
        glCheck(glEnable(GL_SCISSOR_TEST));
    else
        glCheck(glDisable(GL_SCISSOR_TEST));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
    if (bd->GlVersion >= 310)
    {
        if (last_enable_primitive_restart)
            glCheck(glEnable(GL_PRIMITIVE_RESTART));
        else
            glCheck(glDisable(GL_PRIMITIVE_RESTART));
    }
#endif

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    // Desktop OpenGL 3.0 and OpenGL 3.1 had separate polygon draw modes for front-facing and back-facing faces of polygons
    if (bd->HasPolygonMode)
    {
        if (bd->GlVersion <= 310 || bd->GlProfileIsCompat)
        {
            glCheck(glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]));
            glCheck(glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]));
        }
        else
        {
            glCheck(glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]));
        }
    }
#endif // IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE

    glCheck(glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]));
    glCheck(glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]));
    (void)bd; // Not all compilation paths use this
}

bool ImGui_ImplOpenGL3_CreateFontsTexture()
{
    ImGuiIO&                io = ImGui::GetIO();
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // Build texture atlas
    unsigned char* pixels;
    int            width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Upload texture to graphics system
    // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
    GLint last_texture;
    glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
    glCheck(glGenTextures(1, &bd->FontTexture));
    glCheck(glBindTexture(GL_TEXTURE_2D, bd->FontTexture));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
    glCheck(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
#endif
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->FontTexture);

    // Restore state
    glCheck(glBindTexture(GL_TEXTURE_2D, last_texture));

    return true;
}

void ImGui_ImplOpenGL3_DestroyFontsTexture()
{
    ImGuiIO&                io = ImGui::GetIO();
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    if (bd->FontTexture)
    {
        glCheck(glDeleteTextures(1, &bd->FontTexture));
        io.Fonts->SetTexID(0);
        bd->FontTexture = 0;
    }
}

// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc)
{
    ImGui_ImplOpenGL3_Data* bd     = ImGui_ImplOpenGL3_GetBackendData();
    GLint                   status = 0, log_length = 0;
    glCheck(glGetShaderiv(handle, GL_COMPILE_STATUS, &status));
    glCheck(glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length));
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr,
                "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s! With GLSL: %s\n",
                desc,
                bd->GlslVersionString);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glCheck(glGetShaderInfoLog(handle, log_length, nullptr, (GLchar*)buf.begin()));
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc)
{
    ImGui_ImplOpenGL3_Data* bd     = ImGui_ImplOpenGL3_GetBackendData();
    GLint                   status = 0, log_length = 0;
    glCheck(glGetProgramiv(handle, GL_LINK_STATUS, &status));
    glCheck(glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length));
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr,
                "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! With GLSL %s\n",
                desc,
                bd->GlslVersionString);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glCheck(glGetProgramInfoLog(handle, log_length, nullptr, (GLchar*)buf.begin()));
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

bool ImGui_ImplOpenGL3_CreateDeviceObjects()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // Backup GL state
    GLint last_texture, last_array_buffer;
    glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
    glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_BUFFER_PIXEL_UNPACK
    GLint last_pixel_unpack_buffer = 0;
    if (bd->GlVersion >= 210)
    {
        glCheck(glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &last_pixel_unpack_buffer));
        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    }
#endif
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    GLint last_vertex_array;
    glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array));
#endif

    // Parse GLSL version string
    int glsl_version = 130;
    sscanf(bd->GlslVersionString, "#version %d", &glsl_version);

    const GLchar* vertex_shader_glsl_120 =
        "uniform mat4 ProjMtx;\n"
        "attribute vec2 Position;\n"
        "attribute vec2 UV;\n"
        "attribute vec4 Color;\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* vertex_shader_glsl_130 =
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* vertex_shader_glsl_300_es =
        "precision highp float;\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "layout (location = 2) in vec4 Color;\n"
        "uniform mat4 ProjMtx;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* vertex_shader_glsl_410_core =
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "layout (location = 2) in vec4 Color;\n"
        "uniform mat4 ProjMtx;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader_glsl_120 =
        "#ifdef GL_ES\n"
        "    precision mediump float;\n"
        "#endif\n"
        "uniform sampler2D Texture;\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
        "}\n";

    const GLchar* fragment_shader_glsl_130 =
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    const GLchar* fragment_shader_glsl_300_es =
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "layout (location = 0) out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    const GLchar* fragment_shader_glsl_410_core =
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "uniform sampler2D Texture;\n"
        "layout (location = 0) out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    // Select shaders matching our GLSL versions
    const GLchar* vertex_shader   = nullptr;
    const GLchar* fragment_shader = nullptr;
    if (glsl_version < 130)
    {
        vertex_shader   = vertex_shader_glsl_120;
        fragment_shader = fragment_shader_glsl_120;
    }
    else if (glsl_version >= 410)
    {
        vertex_shader   = vertex_shader_glsl_410_core;
        fragment_shader = fragment_shader_glsl_410_core;
    }
    else if (glsl_version == 300)
    {
        vertex_shader   = vertex_shader_glsl_300_es;
        fragment_shader = fragment_shader_glsl_300_es;
    }
    else
    {
        vertex_shader   = vertex_shader_glsl_130;
        fragment_shader = fragment_shader_glsl_130;
    }

    // Create shaders
    const GLchar* vertex_shader_with_version[2] = {bd->GlslVersionString, vertex_shader};
    GLuint        vert_handle                   = glCreateShader(GL_VERTEX_SHADER);
    glCheck(glShaderSource(vert_handle, 2, vertex_shader_with_version, nullptr));
    glCheck(glCompileShader(vert_handle));
    CheckShader(vert_handle, "vertex shader");

    const GLchar* fragment_shader_with_version[2] = {bd->GlslVersionString, fragment_shader};
    GLuint        frag_handle                     = glCreateShader(GL_FRAGMENT_SHADER);
    glCheck(glShaderSource(frag_handle, 2, fragment_shader_with_version, nullptr));
    glCheck(glCompileShader(frag_handle));
    CheckShader(frag_handle, "fragment shader");

    // Link
    bd->ShaderHandle = glCreateProgram();
    glCheck(glAttachShader(bd->ShaderHandle, vert_handle));
    glCheck(glAttachShader(bd->ShaderHandle, frag_handle));
    glCheck(glLinkProgram(bd->ShaderHandle));
    CheckProgram(bd->ShaderHandle, "shader program");

    glCheck(glDetachShader(bd->ShaderHandle, vert_handle));
    glCheck(glDetachShader(bd->ShaderHandle, frag_handle));
    glCheck(glDeleteShader(vert_handle));
    glCheck(glDeleteShader(frag_handle));

    bd->AttribLocationTex      = glGetUniformLocation(bd->ShaderHandle, "Texture");
    bd->AttribLocationProjMtx  = glGetUniformLocation(bd->ShaderHandle, "ProjMtx");
    bd->AttribLocationVtxPos   = (GLuint)glGetAttribLocation(bd->ShaderHandle, "Position");
    bd->AttribLocationVtxUV    = (GLuint)glGetAttribLocation(bd->ShaderHandle, "UV");
    bd->AttribLocationVtxColor = (GLuint)glGetAttribLocation(bd->ShaderHandle, "Color");

    // Create buffers
    glCheck(glGenBuffers(1, &bd->VboHandle));
    glCheck(glGenBuffers(1, &bd->ElementsHandle));

    ImGui_ImplOpenGL3_CreateFontsTexture();

    // Restore modified GL state
    glCheck(glBindTexture(GL_TEXTURE_2D, last_texture));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_BUFFER_PIXEL_UNPACK
    if (bd->GlVersion >= 210)
    {
        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, last_pixel_unpack_buffer));
    }
#endif
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glCheck(glBindVertexArray(last_vertex_array));
#endif

    return true;
}

void ImGui_ImplOpenGL3_DestroyDeviceObjects()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    if (bd->VboHandle)
    {
        glCheck(glDeleteBuffers(1, &bd->VboHandle));
        bd->VboHandle = 0;
    }
    if (bd->ElementsHandle)
    {
        glCheck(glDeleteBuffers(1, &bd->ElementsHandle));
        bd->ElementsHandle = 0;
    }
    if (bd->ShaderHandle)
    {
        glCheck(glDeleteProgram(bd->ShaderHandle));
        bd->ShaderHandle = 0;
    }
    ImGui_ImplOpenGL3_DestroyFontsTexture();
}

//-----------------------------------------------------------------------------

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef IMGUI_DISABLE


// NOLINTEND


static_assert(sizeof(GLuint) <= sizeof(ImTextureID), "ImTextureID is not large enough to fit GLuint.");

namespace
{
// various helper functions
ImColor      toImColor(sf::Color c);
ImVec2       toImVec2(const sf::Vector2f& v);
sf::Vector2f toSfVector2f(const ImVec2& v);
ImVec2       getTopLeftAbsolute(const sf::FloatRect& rect);
ImVec2       getDownRightAbsolute(const sf::FloatRect& rect);


struct StickInfo
{
    sf::Joystick::Axis xAxis{sf::Joystick::Axis::X};
    sf::Joystick::Axis yAxis{sf::Joystick::Axis::Y};

    bool xInverted{false};
    bool yInverted{false};

    float threshold{15};
};

struct TriggerInfo
{
    sf::Joystick::Axis axis{sf::Joystick::Axis::Z};
    float              threshold{0};
};

// data
constexpr unsigned int nullJoystickId = sf::Joystick::Count;

unsigned int getConnectedJoystickId()
{
    for (unsigned int i = 0; i < static_cast<unsigned int>(sf::Joystick::Count); ++i)
    {
        if (sf::Joystick::isConnected(i))
            return i;
    }

    return nullJoystickId;
}

struct ImGuiWindowContext
{
    const sf::Window* window;
    ImGuiContext*     imContext{::ImGui::CreateContext()};

    sf::base::Optional<sf::Texture> fontTexture; // internal font atlas which is used if user
                                                 // doesn't set a custom sf::Texture.

    bool             windowHasFocus;
    bool             mouseMoved{false};
    bool             mousePressed[3] = {false};
    ImGuiMouseCursor lastCursor{ImGuiMouseCursor_COUNT};

    bool         touchDown[3] = {false};
    sf::Vector2i touchPos;

    unsigned int joystickId{getConnectedJoystickId()};
    ImGuiKey     joystickMapping[sf::Joystick::ButtonCount] = {ImGuiKey_None};
    StickInfo    dPadInfo;
    StickInfo    lStickInfo;
    StickInfo    rStickInfo;
    TriggerInfo  lTriggerInfo;
    TriggerInfo  rTriggerInfo;

    sf::base::Optional<sf::Cursor> mouseCursors[ImGuiMouseCursor_COUNT];

#ifdef ANDROID
#ifdef USE_JNI
    bool wantTextInput{false};
#endif
#endif

    ImGuiWindowContext(const sf::Window* w) : window(w), windowHasFocus(window->hasFocus())
    {
    }
    ~ImGuiWindowContext()
    {
        ::ImGui::DestroyContext(imContext);
    }

    ImGuiWindowContext(const ImGuiWindowContext&)            = delete; // non construction-copyable
    ImGuiWindowContext& operator=(const ImGuiWindowContext&) = delete; // non copyable
};

std::vector<std::unique_ptr<ImGuiWindowContext>> sWindowContexts;
ImGuiWindowContext*                              sCurrWindowCtx = nullptr;

struct SpriteTextureData
{
    ImVec2      uv0;
    ImVec2      uv1;
    ImTextureID textureID{};
};

ImColor toImColor(sf::Color c)
{
    return {static_cast<int>(c.r), static_cast<int>(c.g), static_cast<int>(c.b), static_cast<int>(c.a)};
}
ImVec2 toImVec2(const sf::Vector2f& v)
{
    return {v.x, v.y};
}
sf::Vector2f toSfVector2f(const ImVec2& v)
{
    return {v.x, v.y};
}
ImVec2 getTopLeftAbsolute(const sf::FloatRect& rect)
{
    return toImVec2(toSfVector2f(::ImGui::GetCursorScreenPos()) + rect.position);
}
ImVec2 getDownRightAbsolute(const sf::FloatRect& rect)
{
    return toImVec2(toSfVector2f(::ImGui::GetCursorScreenPos()) + rect.position + rect.size);
}

ImTextureID convertGLTextureHandleToImTextureID(GLuint glTextureHandle)
{
    ImTextureID textureID = nullptr;
    std::memcpy(&textureID, &glTextureHandle, sizeof(GLuint));
    return textureID;
}

SpriteTextureData getSpriteTextureData(const sf::Sprite& sprite, const sf::Texture& texture)
{
    const auto textureSize(texture.getSize().to<sf::Vector2f>());
    const auto textureRect(sprite.getTextureRect().to<sf::FloatRect>());

    return {toImVec2(textureRect.position.cwiseDiv(textureSize)),
            toImVec2((textureRect.position + textureRect.size).cwiseDiv(textureSize)),
            convertGLTextureHandleToImTextureID(texture.getNativeHandle())};
}

GLuint convertImTextureIDToGLTextureHandle(ImTextureID textureID)
{
    GLuint glTextureHandle = 0;
    std::memcpy(&glTextureHandle, &textureID, sizeof(GLuint));
    return glTextureHandle;
}


// NOLINTBEGIN


////////////////////////////////////////////////////////////
template <auto FnGen, auto FnBind, auto FnGet, auto FnDelete>
class OpenGLRAII
{
public:
    [[nodiscard]] explicit OpenGLRAII()
    {
        SFML_BASE_ASSERT(m_id == 0u);
        FnGen(m_id);
        SFML_BASE_ASSERT(m_id != 0u);
    }

    [[nodiscard]] bool isBound() const
    {
        int out{};
        FnGet(out);
        return out != 0u;
    }

    void bind() const
    {
        SFML_BASE_ASSERT(m_id != 0u);
        FnBind(m_id);

        SFML_BASE_ASSERT(isBound());
    }

    ~OpenGLRAII()
    {
        if (m_id != 0u)
            FnDelete(m_id);
    }

    OpenGLRAII(const OpenGLRAII&)            = delete;
    OpenGLRAII& operator=(const OpenGLRAII&) = delete;

    OpenGLRAII(OpenGLRAII&& rhs) noexcept : m_id(sf::base::exchange(rhs.m_id, 0u))
    {
    }

    OpenGLRAII& operator=(OpenGLRAII&& rhs) noexcept
    {
        if (&rhs == this)
            return *this;

        m_id = sf::base::exchange(rhs.m_id, 0u);
        return *this;
    }

private:
    unsigned int m_id{};
};


////////////////////////////////////////////////////////////
using VAO = OpenGLRAII<[](auto& id) { glCheck(glGenVertexArrays(1, &id)); },
                       [](auto id) { glCheck(glBindVertexArray(id)); },
                       [](auto& id) { glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &id)); },
                       [](auto& id) { glCheck(glDeleteVertexArrays(1, &id)); }>;


////////////////////////////////////////////////////////////
using VBO = OpenGLRAII<[](auto& id) { glCheck(glGenBuffers(1, &id)); },
                       [](auto id) { glCheck(glBindBuffer(GL_ARRAY_BUFFER, id)); },
                       [](auto& id) { glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id)); },
                       [](auto& id) { glCheck(glDeleteBuffers(1, &id)); }>;


// copied from imgui/backends/imgui_impl_opengl2.cpp
void SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height)
{
    // static VAO imguivao; //!< Vertex array object associated with the render target
    // static VBO imguivbo; //!< Vertex buffer object associated with the render target

    // imguivao.bind();
    // imguivbo.bind();

    glCheck(glUseProgram(0));
    glCheck(glBindVertexArray(0));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor
    // enabled, vertex/texcoord/color pointers, polygon fill.
    glCheck(glEnable(GL_BLEND));
    glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); //
    // In order to composite our output buffer we need to preserve alpha
    glCheck(glDisable(GL_CULL_FACE));
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glDisable(GL_STENCIL_TEST));
    glCheck(glDisable(GL_LIGHTING));
    glCheck(glDisable(GL_COLOR_MATERIAL));
    glCheck(glEnable(GL_SCISSOR_TEST));
    glCheck(glEnableClientState(GL_VERTEX_ARRAY));
    glCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
    glCheck(glEnableClientState(GL_COLOR_ARRAY));
    glCheck(glDisableClientState(GL_NORMAL_ARRAY));
    glCheck(glEnable(GL_TEXTURE_2D));
    glCheck(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    glCheck(glShadeModel(GL_SMOOTH));
    glCheck(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single
    // viewport apps.
    glCheck(glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height));
    glCheck(glMatrixMode(GL_PROJECTION));
    glCheck(glPushMatrix());
    glCheck(glLoadIdentity());
#ifdef GL_VERSION_ES_CL_1_1
    glCheck(glOrthof(draw_data->DisplayPos.x,
                     draw_data->DisplayPos.x + draw_data->DisplaySize.x,
                     draw_data->DisplayPos.y + draw_data->DisplaySize.y,
                     draw_data->DisplayPos.y,
                     -1.0f,
                     +1.0f));
#else
    glCheck(glOrtho(draw_data->DisplayPos.x,
                    draw_data->DisplayPos.x + draw_data->DisplaySize.x,
                    draw_data->DisplayPos.y + draw_data->DisplaySize.y,
                    draw_data->DisplayPos.y,
                    -1.0f,
                    +1.0f));
#endif
    glCheck(glMatrixMode(GL_MODELVIEW));
    glCheck(glPushMatrix());
    glCheck(glLoadIdentity());
}

// Rendering callback
void RenderDrawLists(ImDrawData* draw_data)
{
    ImGui::GetDrawData();
    if (draw_data->CmdListsCount == 0)
    {
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    SFML_BASE_ASSERT(io.Fonts->TexID != (ImTextureID) nullptr); // You forgot to create and set font texture

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates !=
    // framebuffer coordinates)
    const int fb_width  = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    const int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    // Backup GL state
    GLint last_program = 0;
    glCheck(glGetIntegerv(GL_CURRENT_PROGRAM, &last_program));
    GLint last_texture = 0;
    glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
#ifndef SFML_SYSTEM_EMSCRIPTEN
    GLint last_polygon_mode[2];
    glCheck(glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode));
#endif
    GLint last_viewport[4];
    glCheck(glGetIntegerv(GL_VIEWPORT, last_viewport));
    GLint last_scissor_box[4];
    glCheck(glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box));
#ifndef SFML_SYSTEM_EMSCRIPTEN
    GLint last_shade_model = 0;
    glCheck(glGetIntegerv(GL_SHADE_MODEL, &last_shade_model));
#endif
    GLint last_tex_env_mode = 0;
    glCheck(glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &last_tex_env_mode));
    GLint last_vertex_array;
    glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array));
    GLint last_array_buffer;
    glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer));
    GLint last_element_array_buffer;
    glCheck(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer));

    glCheck(glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT));

    // Setup desired GL state
    SetupRenderState(draw_data, fb_width, fb_height);

    // Will project scissor/clipping rectangles into framebuffer space
    const ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
    const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display
                                                           // which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list   = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx*  idx_buffer = cmd_list->IdxBuffer.Data;
        glCheck(glVertexPointer(2,
                                GL_FLOAT,
                                sizeof(ImDrawVert),
                                (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos))));
        glCheck(glTexCoordPointer(2,
                                  GL_FLOAT,
                                  sizeof(ImDrawVert),
                                  (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv))));
        glCheck(glColorPointer(4,
                               GL_UNSIGNED_BYTE,
                               sizeof(ImDrawVert),
                               (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col))));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to
                // request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState(draw_data, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < static_cast<float>(fb_width) && clip_rect.y < static_cast<float>(fb_height) &&
                    clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Apply scissor/clipping rectangle
                    glCheck(glScissor((int)clip_rect.x,
                                      (int)(static_cast<float>(fb_height) - clip_rect.w),
                                      (int)(clip_rect.z - clip_rect.x),
                                      (int)(clip_rect.w - clip_rect.y)));

                    // Bind texture, Draw
                    const GLuint textureHandle = convertImTextureIDToGLTextureHandle(pcmd->TextureId);
                    glCheck(glBindTexture(GL_TEXTURE_2D, textureHandle));
                    glCheck(glDrawElements(GL_TRIANGLES,
                                           (GLsizei)pcmd->ElemCount,
                                           sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                           idx_buffer + pcmd->IdxOffset));
                }
            }
        }
    }

    // Restore modified GL state
    glCheck(glDisableClientState(GL_COLOR_ARRAY));
    glCheck(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
    glCheck(glDisableClientState(GL_VERTEX_ARRAY));
    glCheck(glUseProgram(last_program));
    glCheck(glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture));
    glCheck(glMatrixMode(GL_MODELVIEW));
    glCheck(glPopMatrix());
    glCheck(glMatrixMode(GL_PROJECTION));
    glCheck(glPopMatrix());
    glCheck(glPopAttrib());
#ifndef SFML_SYSTEM_EMSCRIPTEN
    glCheck(glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]));
    glCheck(glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]));
#endif
    glCheck(glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]));
    glCheck(glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]));
#ifndef SFML_SYSTEM_EMSCRIPTEN
    glCheck(glShadeModel((GLenum)last_shade_model));
#endif
    glCheck(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, last_tex_env_mode));
    glCheck(glBindVertexArray(last_vertex_array));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer));
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer));
    glCheck(glDisable(GL_SCISSOR_TEST));
}
// NOLINTEND

void initDefaultJoystickMapping()
{
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadFaceDown, 0);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadFaceRight, 1);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadFaceLeft, 2);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadFaceUp, 3);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadL1, 4);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadR1, 5);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadBack, 6);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadStart, 7);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadL3, 9);
    sf::ImGui::SetJoystickMapping(ImGuiKey_GamepadR3, 10);

    sf::ImGui::SetDPadXAxis(sf::Joystick::Axis::PovX);
    // D-pad Y axis is inverted on Windows
#ifdef _WIN32
    sf::ImGui::SetDPadYAxis(sf::Joystick::Axis::PovY, true);
#else
    sf::ImGui::SetDPadYAxis(sf::Joystick::Axis::PovY);
#endif

    sf::ImGui::SetLStickXAxis(sf::Joystick::Axis::X);
    sf::ImGui::SetLStickYAxis(sf::Joystick::Axis::Y);
    sf::ImGui::SetRStickXAxis(sf::Joystick::Axis::U);
    sf::ImGui::SetRStickYAxis(sf::Joystick::Axis::V);
    sf::ImGui::SetLTriggerAxis(sf::Joystick::Axis::Z);
    sf::ImGui::SetRTriggerAxis(sf::Joystick::Axis::R);

    sf::ImGui::SetJoystickDPadThreshold(5.f);
    sf::ImGui::SetJoystickLStickThreshold(5.f);
    sf::ImGui::SetJoystickRStickThreshold(15.f);
    sf::ImGui::SetJoystickLTriggerThreshold(0.f);
    sf::ImGui::SetJoystickRTriggerThreshold(0.f);
}

void updateJoystickButtonState(ImGuiIO& io)
{
    for (int i = 0; i < static_cast<int>(sf::Joystick::ButtonCount); ++i)
    {
        const ImGuiKey key = sCurrWindowCtx->joystickMapping[i];
        if (key != ImGuiKey_None)
        {
            const bool isPressed = sf::Joystick::isButtonPressed(sCurrWindowCtx->joystickId, static_cast<unsigned>(i));
            if (sCurrWindowCtx->windowHasFocus || !isPressed)
            {
                io.AddKeyEvent(key, isPressed);
            }
        }
    }
}

void updateJoystickAxis(ImGuiIO& io, ImGuiKey key, sf::Joystick::Axis axis, float threshold, float maxThreshold, bool inverted)
{
    float pos = sf::Joystick::getAxisPosition(sCurrWindowCtx->joystickId, axis);
    if (inverted)
    {
        pos = -pos;
    }
    const bool passedThreshold = (pos > threshold) == (maxThreshold > threshold);
    if (passedThreshold && sCurrWindowCtx->windowHasFocus)
    {
        io.AddKeyAnalogEvent(key, true, std::abs(pos / 100.f));
    }
    else
    {
        io.AddKeyAnalogEvent(key, false, 0);
    }
}

void updateJoystickAxisPair(ImGuiIO& io, ImGuiKey key1, ImGuiKey key2, sf::Joystick::Axis axis, float threshold, bool inverted)
{
    updateJoystickAxis(io, key1, axis, -threshold, -100, inverted);
    updateJoystickAxis(io, key2, axis, threshold, 100, inverted);
}

void updateJoystickDPadState(ImGuiIO& io)
{
    updateJoystickAxisPair(io,
                           ImGuiKey_GamepadDpadLeft,
                           ImGuiKey_GamepadDpadRight,
                           sCurrWindowCtx->dPadInfo.xAxis,
                           sCurrWindowCtx->dPadInfo.threshold,
                           sCurrWindowCtx->dPadInfo.xInverted);
    updateJoystickAxisPair(io,
                           ImGuiKey_GamepadDpadUp,
                           ImGuiKey_GamepadDpadDown,
                           sCurrWindowCtx->dPadInfo.yAxis,
                           sCurrWindowCtx->dPadInfo.threshold,
                           sCurrWindowCtx->dPadInfo.yInverted);
}

void updateJoystickAxisState(ImGuiIO& io)
{
    updateJoystickAxisPair(io,
                           ImGuiKey_GamepadLStickLeft,
                           ImGuiKey_GamepadLStickRight,
                           sCurrWindowCtx->lStickInfo.xAxis,
                           sCurrWindowCtx->lStickInfo.threshold,
                           sCurrWindowCtx->lStickInfo.xInverted);
    updateJoystickAxisPair(io,
                           ImGuiKey_GamepadLStickUp,
                           ImGuiKey_GamepadLStickDown,
                           sCurrWindowCtx->lStickInfo.yAxis,
                           sCurrWindowCtx->lStickInfo.threshold,
                           sCurrWindowCtx->lStickInfo.yInverted);

    updateJoystickAxisPair(io,
                           ImGuiKey_GamepadRStickLeft,
                           ImGuiKey_GamepadRStickRight,
                           sCurrWindowCtx->rStickInfo.xAxis,
                           sCurrWindowCtx->rStickInfo.threshold,
                           sCurrWindowCtx->rStickInfo.xInverted);
    updateJoystickAxisPair(io,
                           ImGuiKey_GamepadRStickUp,
                           ImGuiKey_GamepadRStickDown,
                           sCurrWindowCtx->rStickInfo.yAxis,
                           sCurrWindowCtx->rStickInfo.threshold,
                           sCurrWindowCtx->rStickInfo.yInverted);

    updateJoystickAxis(io, ImGuiKey_GamepadL2, sCurrWindowCtx->lTriggerInfo.axis, sCurrWindowCtx->lTriggerInfo.threshold, 100, false);
    updateJoystickAxis(io, ImGuiKey_GamepadR2, sCurrWindowCtx->rTriggerInfo.axis, sCurrWindowCtx->rTriggerInfo.threshold, 100, false);
}

std::string sClipboardText;

void setClipboardText(void* /*userData*/, const char* text)
{
    sf::Clipboard::setString(sf::StringUtfUtils::fromUtf8(text, text + std::strlen(text)));
}

const char* getClipboardText(void* /*userData*/)
{
    auto tmp = sf::Clipboard::getString().toUtf8();
    sClipboardText.assign(tmp.begin(), tmp.end());
    return sClipboardText.c_str();
}

void loadMouseCursor(ImGuiMouseCursor imguiCursorType, sf::Cursor::Type sfmlCursorType)
{
    sCurrWindowCtx->mouseCursors[imguiCursorType] = sf::Cursor::loadFromSystem(sfmlCursorType);
}

void updateMouseCursor(sf::Window& window)
{
    return;
    const ImGuiIO& io = ::ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
    {
        const ImGuiMouseCursor cursor = ::ImGui::GetMouseCursor();
        if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
        {
            window.setMouseCursorVisible(false);
        }
        else
        {
            window.setMouseCursorVisible(true);

            const sf::Cursor& c = sCurrWindowCtx->mouseCursors[cursor]
                                      ? *sCurrWindowCtx->mouseCursors[cursor]
                                      : *sCurrWindowCtx->mouseCursors[ImGuiMouseCursor_Arrow];
            window.setMouseCursor(c);
        }
    }
}

ImGuiKey keycodeToImGuiKey(sf::Keyboard::Key code)
{
    // clang-format off
    switch (code)
    {
        case sf::Keyboard::Key::Tab:        return ImGuiKey_Tab;
        case sf::Keyboard::Key::Left:       return ImGuiKey_LeftArrow;
        case sf::Keyboard::Key::Right:      return ImGuiKey_RightArrow;
        case sf::Keyboard::Key::Up:         return ImGuiKey_UpArrow;
        case sf::Keyboard::Key::Down:       return ImGuiKey_DownArrow;
        case sf::Keyboard::Key::PageUp:     return ImGuiKey_PageUp;
        case sf::Keyboard::Key::PageDown:   return ImGuiKey_PageDown;
        case sf::Keyboard::Key::Home:       return ImGuiKey_Home;
        case sf::Keyboard::Key::End:        return ImGuiKey_End;
        case sf::Keyboard::Key::Insert:     return ImGuiKey_Insert;
        case sf::Keyboard::Key::Delete:     return ImGuiKey_Delete;
        case sf::Keyboard::Key::Backspace:  return ImGuiKey_Backspace;
        case sf::Keyboard::Key::Space:      return ImGuiKey_Space;
        case sf::Keyboard::Key::Enter:      return ImGuiKey_Enter;
        case sf::Keyboard::Key::Escape:     return ImGuiKey_Escape;
        case sf::Keyboard::Key::Apostrophe: return ImGuiKey_Apostrophe;
        case sf::Keyboard::Key::Comma:      return ImGuiKey_Comma;
        case sf::Keyboard::Key::Hyphen:     return ImGuiKey_Minus;
        case sf::Keyboard::Key::Period:     return ImGuiKey_Period;
        case sf::Keyboard::Key::Slash:      return ImGuiKey_Slash;
        case sf::Keyboard::Key::Semicolon:  return ImGuiKey_Semicolon;
        case sf::Keyboard::Key::Equal:      return ImGuiKey_Equal;
        case sf::Keyboard::Key::LBracket:   return ImGuiKey_LeftBracket;
        case sf::Keyboard::Key::Backslash:  return ImGuiKey_Backslash;
        case sf::Keyboard::Key::RBracket:   return ImGuiKey_RightBracket;
        case sf::Keyboard::Key::Grave:      return ImGuiKey_GraveAccent;
        // case :                           return ImGuiKey_CapsLock;
        // case :                           return ImGuiKey_ScrollLock;
        // case :                           return ImGuiKey_NumLock;
        // case :                           return ImGuiKey_PrintScreen;
        case sf::Keyboard::Key::Pause:      return ImGuiKey_Pause;
        case sf::Keyboard::Key::Numpad0:    return ImGuiKey_Keypad0;
        case sf::Keyboard::Key::Numpad1:    return ImGuiKey_Keypad1;
        case sf::Keyboard::Key::Numpad2:    return ImGuiKey_Keypad2;
        case sf::Keyboard::Key::Numpad3:    return ImGuiKey_Keypad3;
        case sf::Keyboard::Key::Numpad4:    return ImGuiKey_Keypad4;
        case sf::Keyboard::Key::Numpad5:    return ImGuiKey_Keypad5;
        case sf::Keyboard::Key::Numpad6:    return ImGuiKey_Keypad6;
        case sf::Keyboard::Key::Numpad7:    return ImGuiKey_Keypad7;
        case sf::Keyboard::Key::Numpad8:    return ImGuiKey_Keypad8;
        case sf::Keyboard::Key::Numpad9:    return ImGuiKey_Keypad9;
        // case :                           return ImGuiKey_KeypadDecimal;
        case sf::Keyboard::Key::Divide:     return ImGuiKey_KeypadDivide;
        case sf::Keyboard::Key::Multiply:   return ImGuiKey_KeypadMultiply;
        case sf::Keyboard::Key::Subtract:   return ImGuiKey_KeypadSubtract;
        case sf::Keyboard::Key::Add:        return ImGuiKey_KeypadAdd;
        // case :                           return ImGuiKey_KeypadEnter;
        // case :                           return ImGuiKey_KeypadEqual;
        case sf::Keyboard::Key::LControl:   return ImGuiKey_LeftCtrl;
        case sf::Keyboard::Key::LShift:     return ImGuiKey_LeftShift;
        case sf::Keyboard::Key::LAlt:       return ImGuiKey_LeftAlt;
        case sf::Keyboard::Key::LSystem:    return ImGuiKey_LeftSuper;
        case sf::Keyboard::Key::RControl:   return ImGuiKey_RightCtrl;
        case sf::Keyboard::Key::RShift:     return ImGuiKey_RightShift;
        case sf::Keyboard::Key::RAlt:       return ImGuiKey_RightAlt;
        case sf::Keyboard::Key::RSystem:    return ImGuiKey_RightSuper;
        case sf::Keyboard::Key::Menu:       return ImGuiKey_Menu;
        case sf::Keyboard::Key::Num0:       return ImGuiKey_0;
        case sf::Keyboard::Key::Num1:       return ImGuiKey_1;
        case sf::Keyboard::Key::Num2:       return ImGuiKey_2;
        case sf::Keyboard::Key::Num3:       return ImGuiKey_3;
        case sf::Keyboard::Key::Num4:       return ImGuiKey_4;
        case sf::Keyboard::Key::Num5:       return ImGuiKey_5;
        case sf::Keyboard::Key::Num6:       return ImGuiKey_6;
        case sf::Keyboard::Key::Num7:       return ImGuiKey_7;
        case sf::Keyboard::Key::Num8:       return ImGuiKey_8;
        case sf::Keyboard::Key::Num9:       return ImGuiKey_9;
        case sf::Keyboard::Key::A:          return ImGuiKey_A;
        case sf::Keyboard::Key::B:          return ImGuiKey_B;
        case sf::Keyboard::Key::C:          return ImGuiKey_C;
        case sf::Keyboard::Key::D:          return ImGuiKey_D;
        case sf::Keyboard::Key::E:          return ImGuiKey_E;
        case sf::Keyboard::Key::F:          return ImGuiKey_F;
        case sf::Keyboard::Key::G:          return ImGuiKey_G;
        case sf::Keyboard::Key::H:          return ImGuiKey_H;
        case sf::Keyboard::Key::I:          return ImGuiKey_I;
        case sf::Keyboard::Key::J:          return ImGuiKey_J;
        case sf::Keyboard::Key::K:          return ImGuiKey_K;
        case sf::Keyboard::Key::L:          return ImGuiKey_L;
        case sf::Keyboard::Key::M:          return ImGuiKey_M;
        case sf::Keyboard::Key::N:          return ImGuiKey_N;
        case sf::Keyboard::Key::O:          return ImGuiKey_O;
        case sf::Keyboard::Key::P:          return ImGuiKey_P;
        case sf::Keyboard::Key::Q:          return ImGuiKey_Q;
        case sf::Keyboard::Key::R:          return ImGuiKey_R;
        case sf::Keyboard::Key::S:          return ImGuiKey_S;
        case sf::Keyboard::Key::T:          return ImGuiKey_T;
        case sf::Keyboard::Key::U:          return ImGuiKey_U;
        case sf::Keyboard::Key::V:          return ImGuiKey_V;
        case sf::Keyboard::Key::W:          return ImGuiKey_W;
        case sf::Keyboard::Key::X:          return ImGuiKey_X;
        case sf::Keyboard::Key::Y:          return ImGuiKey_Y;
        case sf::Keyboard::Key::Z:          return ImGuiKey_Z;
        case sf::Keyboard::Key::F1:         return ImGuiKey_F1;
        case sf::Keyboard::Key::F2:         return ImGuiKey_F2;
        case sf::Keyboard::Key::F3:         return ImGuiKey_F3;
        case sf::Keyboard::Key::F4:         return ImGuiKey_F4;
        case sf::Keyboard::Key::F5:         return ImGuiKey_F5;
        case sf::Keyboard::Key::F6:         return ImGuiKey_F6;
        case sf::Keyboard::Key::F7:         return ImGuiKey_F7;
        case sf::Keyboard::Key::F8:         return ImGuiKey_F8;
        case sf::Keyboard::Key::F9:         return ImGuiKey_F9;
        case sf::Keyboard::Key::F10:        return ImGuiKey_F10;
        case sf::Keyboard::Key::F11:        return ImGuiKey_F11;
        case sf::Keyboard::Key::F12:        return ImGuiKey_F12;
        default:
            break;
    }
    // clang-format on

    return ImGuiKey_None;
}

ImGuiKey keycodeToImGuiMod(sf::Keyboard::Key code)
{
    switch (code)
    {
        case sf::Keyboard::Key::LControl:
        case sf::Keyboard::Key::RControl:
            return ImGuiKey_ModCtrl;
        case sf::Keyboard::Key::LShift:
        case sf::Keyboard::Key::RShift:
            return ImGuiKey_ModShift;
        case sf::Keyboard::Key::LAlt:
        case sf::Keyboard::Key::RAlt:
            return ImGuiKey_ModAlt;
        case sf::Keyboard::Key::LSystem:
        case sf::Keyboard::Key::RSystem:
            return ImGuiKey_ModSuper;
        default:
            break;
    }
    return ImGuiKey_None;
}

} // end of anonymous namespace

namespace sf::ImGui
{
bool Init(sf::GraphicsContext& graphicsContext, sf::RenderWindow& window, bool loadDefaultFont)
{
    return Init(graphicsContext, window, window, loadDefaultFont);
}

bool Init(sf::GraphicsContext& graphicsContext, sf::Window& window, sf::RenderTarget& target, bool loadDefaultFont)
{
    return Init(graphicsContext, window, target.getSize().to<sf::Vector2f>(), loadDefaultFont);
}

bool Init(sf::GraphicsContext& graphicsContext, sf::Window& window, const sf::Vector2f& displaySize, bool loadDefaultFont)
{
    sWindowContexts.emplace_back(std::make_unique<ImGuiWindowContext>(&window));

    sCurrWindowCtx = sWindowContexts.back().get();
    ::ImGui::SetCurrentContext(sCurrWindowCtx->imContext);



    ImGuiIO& io = ::ImGui::GetIO();

    // tell ImGui which features we support
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendPlatformName = "imgui_impl_sfml";

    sCurrWindowCtx->joystickId = getConnectedJoystickId();

    initDefaultJoystickMapping();

    // init rendering
    io.DisplaySize = toImVec2(displaySize);

    // clipboard
    io.SetClipboardTextFn = setClipboardText;
    io.GetClipboardTextFn = getClipboardText;

    // load mouse cursors
    loadMouseCursor(ImGuiMouseCursor_Arrow, sf::Cursor::Type::Arrow);
    loadMouseCursor(ImGuiMouseCursor_TextInput, sf::Cursor::Type::Text);
    loadMouseCursor(ImGuiMouseCursor_ResizeAll, sf::Cursor::Type::SizeAll);
    loadMouseCursor(ImGuiMouseCursor_ResizeNS, sf::Cursor::Type::SizeVertical);
    loadMouseCursor(ImGuiMouseCursor_ResizeEW, sf::Cursor::Type::SizeHorizontal);
    loadMouseCursor(ImGuiMouseCursor_ResizeNESW, sf::Cursor::Type::SizeBottomLeftTopRight);
    loadMouseCursor(ImGuiMouseCursor_ResizeNWSE, sf::Cursor::Type::SizeTopLeftBottomRight);
    loadMouseCursor(ImGuiMouseCursor_Hand, sf::Cursor::Type::Hand);

    if (loadDefaultFont)
    {
        // this will load default font automatically
        // No need to call AddDefaultFont
        (void)UpdateFontTexture(graphicsContext);
    }

     ImGui_ImplOpenGL3_Init(nullptr);

    return true;
}

void SetCurrentWindow(const sf::Window& window)
{
    auto found = std::find_if(sWindowContexts.begin(),
                              sWindowContexts.end(),
                              [&](std::unique_ptr<ImGuiWindowContext>& ctx)
                              { return ctx->window->getNativeHandle() == window.getNativeHandle(); });
    SFML_BASE_ASSERT(found != sWindowContexts.end() &&
                     "Failed to find the window. Forgot to call Init for the "
                     "window?");
    sCurrWindowCtx = found->get();
    ::ImGui::SetCurrentContext(sCurrWindowCtx->imContext);
}

void ProcessEvent(const sf::Window& window, const sf::Event& event)
{
    SetCurrentWindow(window);
    SFML_BASE_ASSERT(sCurrWindowCtx && "No current window is set - forgot to call Init?");
    ImGuiIO& io = ::ImGui::GetIO();

    if (sCurrWindowCtx->windowHasFocus)
    {
        if (const auto* resized = event.getIf<sf::Event::Resized>())
        {
            io.DisplaySize = ImVec2(static_cast<float>(resized->size.x), static_cast<float>(resized->size.y));
        }
        else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>())
        {
            io.AddMousePosEvent(static_cast<float>(mouseMoved->position.x), static_cast<float>(mouseMoved->position.y));
            sCurrWindowCtx->mouseMoved = true;
        }
        else if (const auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            const int button = static_cast<int>(mouseButtonPressed->button);
            if (button >= 0 && button < 3)
            {
                sCurrWindowCtx->mousePressed[static_cast<int>(mouseButtonPressed->button)] = true;
                io.AddMouseButtonEvent(button, true);
            }
        }
        else if (const auto* mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>())
        {
            const int button = static_cast<int>(mouseButtonReleased->button);
            if (button >= 0 && button < 3)
                io.AddMouseButtonEvent(button, false);
        }
        else if (const auto* touchBegan = event.getIf<sf::Event::TouchBegan>())
        {
            sCurrWindowCtx->mouseMoved = false;
            const unsigned int button  = touchBegan->finger;
            if (button < 3)
                sCurrWindowCtx->touchDown[touchBegan->finger] = true;
        }
        else if (event.is<sf::Event::TouchEnded>())
        {
            sCurrWindowCtx->mouseMoved = false;
        }
        else if (const auto* mouseWheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (mouseWheelScrolled->wheel == sf::Mouse::Wheel::Vertical ||
                (mouseWheelScrolled->wheel == sf::Mouse::Wheel::Horizontal && io.KeyShift))
            {
                io.AddMouseWheelEvent(0, mouseWheelScrolled->delta);
            }
            else if (mouseWheelScrolled->wheel == sf::Mouse::Wheel::Horizontal)
            {
                io.AddMouseWheelEvent(mouseWheelScrolled->delta, 0);
            }
        }
        const auto handleKeyChanged = [&io](const auto& keyChanged, bool down)
        {
            const ImGuiKey mod = keycodeToImGuiMod(keyChanged.code);
            // The modifier booleans are not reliable when it's the modifier
            // itself that's being pressed. Detect these presses directly.
            if (mod != ImGuiKey_None)
            {
                io.AddKeyEvent(mod, down);
            }
            else
            {
                io.AddKeyEvent(ImGuiKey_ModCtrl, keyChanged.control);
                io.AddKeyEvent(ImGuiKey_ModShift, keyChanged.shift);
                io.AddKeyEvent(ImGuiKey_ModAlt, keyChanged.alt);
                io.AddKeyEvent(ImGuiKey_ModSuper, keyChanged.system);
            }

            const ImGuiKey key = keycodeToImGuiKey(keyChanged.code);
            io.AddKeyEvent(key, down);
            io.SetKeyEventNativeData(key, static_cast<int>(keyChanged.code), -1);
        };
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            handleKeyChanged(*keyPressed, true);
        }
        else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>())
        {
            handleKeyChanged(*keyReleased, false);
        }
        else if (const auto* textEntered = event.getIf<sf::Event::TextEntered>())
        {
            // Don't handle the event for unprintable characters
            if (textEntered->unicode >= ' ' && textEntered->unicode != 127)
                io.AddInputCharacter(textEntered->unicode);
        }
        else if (const auto* joystickConnected = event.getIf<sf::Event::JoystickConnected>())
        {
            if (sCurrWindowCtx->joystickId == nullJoystickId)
                sCurrWindowCtx->joystickId = joystickConnected->joystickId;
        }
        else if (const auto* joystickDisconnected = event.getIf<sf::Event::JoystickDisconnected>())
        {
            if (sCurrWindowCtx->joystickId == joystickDisconnected->joystickId)
                // used gamepad was disconnected
                sCurrWindowCtx->joystickId = getConnectedJoystickId();
        }
    }

    if (event.is<sf::Event::FocusLost>())
    {
        io.AddFocusEvent(false);
        sCurrWindowCtx->windowHasFocus = false;
    }
    else if (event.is<sf::Event::FocusGained>())
    {
        io.AddFocusEvent(true);
        sCurrWindowCtx->windowHasFocus = true;
    }
}

void Update(sf::RenderWindow& window, sf::Time dt)
{
    Update(window, window, dt);
}

void Update(sf::Window& window, sf::RenderTarget& target, sf::Time dt)
{
    SetCurrentWindow(window);
    SFML_BASE_ASSERT(sCurrWindowCtx);

    // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
    const ImGuiMouseCursor mouseCursor = ::ImGui::GetIO().MouseDrawCursor ? ImGuiMouseCursor_None : ::ImGui::GetMouseCursor();
    if (sCurrWindowCtx->lastCursor != mouseCursor)
    {
        sCurrWindowCtx->lastCursor = mouseCursor;
        updateMouseCursor(window);
    }

    if (!sCurrWindowCtx->mouseMoved)
    {
        if (sf::Touch::isDown(0))
            sCurrWindowCtx->touchPos = sf::Touch::getPosition(0, window);

        Update(sCurrWindowCtx->touchPos, target.getSize().to<sf::Vector2f>(), dt);
    }
    else
    {
        Update(sf::Mouse::getPosition(window), target.getSize().to<sf::Vector2f>(), dt);
    }
}

void Update(const sf::Vector2i& mousePos, const sf::Vector2f& displaySize, sf::Time dt)
{
    SFML_BASE_ASSERT(sCurrWindowCtx && "No current window is set - forgot to call Init?");

    ImGuiIO& io    = ::ImGui::GetIO();
    io.DisplaySize = toImVec2(displaySize);
    io.DeltaTime   = dt.asSeconds();

    if (sCurrWindowCtx->windowHasFocus)
    {
        if (io.WantSetMousePos)
        {
            const sf::Vector2i newMousePos(static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
            sf::Mouse::setPosition(newMousePos);
        }
        else
        {
            io.MousePos = ImVec2(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        }
        for (unsigned int i = 0; i < 3; i++)
        {
            io.MouseDown[i] = sCurrWindowCtx->touchDown[i] || sf::Touch::isDown(i) || sCurrWindowCtx->mousePressed[i] ||
                              sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));
            sCurrWindowCtx->mousePressed[i] = false;
            sCurrWindowCtx->touchDown[i]    = false;
        }
    }

#ifdef ANDROID
#ifdef USE_JNI
    if (io.WantTextInput && !s_currWindowCtx->wantTextInput)
    {
        openKeyboardIME();
        s_currWindowCtx->wantTextInput = true;
    }

    if (!io.WantTextInput && s_currWindowCtx->wantTextInput)
    {
        closeKeyboardIME();
        s_currWindowCtx->wantTextInput = false;
    }
#endif
#endif

    SFML_BASE_ASSERT(io.Fonts->Fonts.Size > 0); // You forgot to create and set up font
                                                // atlas (see createFontTexture)

    // gamepad navigation
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) && sCurrWindowCtx->joystickId != nullJoystickId)
    {
        updateJoystickButtonState(io);
        updateJoystickDPadState(io);
        updateJoystickAxisState(io);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ::ImGui::NewFrame();
}

void Render(sf::RenderWindow& window)
{
    SetCurrentWindow(window);
    Render(static_cast<sf::RenderTarget&>(window));
}

void Render(sf::RenderTarget& target)
{
    target.resetGLStates();

    ::ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());
}

void Render()
{
    ::ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());
}

void Shutdown(const sf::Window& window)
{
    const bool needReplacement = (sCurrWindowCtx->window->getNativeHandle() == window.getNativeHandle());

    // remove window's context
    auto found = std::find_if(sWindowContexts.begin(),
                              sWindowContexts.end(),
                              [&](std::unique_ptr<ImGuiWindowContext>& ctx)
                              { return ctx->window->getNativeHandle() == window.getNativeHandle(); });
    SFML_BASE_ASSERT(found != sWindowContexts.end() && "Window wasn't inited properly: forgot to call Init(window)?");
    sWindowContexts.erase(found); // s_currWindowCtx can become invalid here!

    // set current context to some window for convenience if needed
    if (needReplacement)
    {
        auto it = sWindowContexts.begin();
        if (it != sWindowContexts.end())
        {
            // set to some other window
            sCurrWindowCtx = it->get();
            ::ImGui::SetCurrentContext(sCurrWindowCtx->imContext);
        }
        else
        {
            // no alternatives...
            sCurrWindowCtx = nullptr;
            ::ImGui::SetCurrentContext(nullptr);
        }
    }
}

void Shutdown()
{
    sCurrWindowCtx = nullptr;
    ::ImGui::SetCurrentContext(nullptr);

    sWindowContexts.clear();
}

bool UpdateFontTexture(sf::GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);

    ImGuiIO&       io     = ::ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int            width  = 0;
    int            height = 0;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    auto newTexture = sf::Texture::create(graphicsContext, {static_cast<unsigned>(width), static_cast<unsigned>(height)});

    if (!newTexture.hasValue())
    {
        priv::err() << "Failed to create default ImGui font texture";
        return false;
    }

    newTexture->update(pixels);

    ImTextureID texID = convertGLTextureHandleToImTextureID(newTexture->getNativeHandle());
    io.Fonts->SetTexID(texID);

    sCurrWindowCtx->fontTexture = std::move(newTexture);

    return true;
}

sf::base::Optional<sf::Texture>& GetFontTexture()
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    return sCurrWindowCtx->fontTexture;
}

void SetActiveJoystickId(unsigned int joystickId)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    SFML_BASE_ASSERT(joystickId < sf::Joystick::Count);
    sCurrWindowCtx->joystickId = joystickId;
}

void SetJoystickDPadThreshold(float threshold)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    SFML_BASE_ASSERT(threshold >= 0.f && threshold <= 100.f);
    sCurrWindowCtx->dPadInfo.threshold = threshold;
}

void SetJoystickLStickThreshold(float threshold)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    SFML_BASE_ASSERT(threshold >= 0.f && threshold <= 100.f);
    sCurrWindowCtx->lStickInfo.threshold = threshold;
}

void SetJoystickRStickThreshold(float threshold)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    SFML_BASE_ASSERT(threshold >= 0.f && threshold <= 100.f);
    sCurrWindowCtx->rStickInfo.threshold = threshold;
}

void SetJoystickLTriggerThreshold(float threshold)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    SFML_BASE_ASSERT(threshold >= -100.f && threshold <= 100.f);
    sCurrWindowCtx->lTriggerInfo.threshold = threshold;
}

void SetJoystickRTriggerThreshold(float threshold)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    SFML_BASE_ASSERT(threshold >= -100.f && threshold <= 100.f);
    sCurrWindowCtx->rTriggerInfo.threshold = threshold;
}

void SetJoystickMapping(int key, unsigned int joystickButton)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    // This function now expects ImGuiKey_* values.
    // For partial backwards compatibility, also expect some ImGuiNavInput_* values.
    ImGuiKey finalKey{};
    switch (key)
    {
        case ImGuiNavInput_Activate:
            finalKey = ImGuiKey_GamepadFaceDown;
            break;
        case ImGuiNavInput_Cancel:
            finalKey = ImGuiKey_GamepadFaceRight;
            break;
        case ImGuiNavInput_Input:
            finalKey = ImGuiKey_GamepadFaceUp;
            break;
        case ImGuiNavInput_Menu:
            finalKey = ImGuiKey_GamepadFaceLeft;
            break;
        case ImGuiNavInput_FocusPrev:
        case ImGuiNavInput_TweakSlow:
            finalKey = ImGuiKey_GamepadL1;
            break;
        case ImGuiNavInput_FocusNext:
        case ImGuiNavInput_TweakFast:
            finalKey = ImGuiKey_GamepadR1;
            break;
        default:
            SFML_BASE_ASSERT(key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END);
            finalKey = static_cast<ImGuiKey>(key);
    }
    SFML_BASE_ASSERT(joystickButton < sf::Joystick::ButtonCount);
    sCurrWindowCtx->joystickMapping[joystickButton] = finalKey;
}

void SetDPadXAxis(sf::Joystick::Axis dPadXAxis, bool inverted)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->dPadInfo.xAxis     = dPadXAxis;
    sCurrWindowCtx->dPadInfo.xInverted = inverted;
}

void SetDPadYAxis(sf::Joystick::Axis dPadYAxis, bool inverted)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->dPadInfo.yAxis     = dPadYAxis;
    sCurrWindowCtx->dPadInfo.yInverted = inverted;
}

void SetLStickXAxis(sf::Joystick::Axis lStickXAxis, bool inverted)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->lStickInfo.xAxis     = lStickXAxis;
    sCurrWindowCtx->lStickInfo.xInverted = inverted;
}

void SetLStickYAxis(sf::Joystick::Axis lStickYAxis, bool inverted)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->lStickInfo.yAxis     = lStickYAxis;
    sCurrWindowCtx->lStickInfo.yInverted = inverted;
}

void SetRStickXAxis(sf::Joystick::Axis rStickXAxis, bool inverted)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->rStickInfo.xAxis     = rStickXAxis;
    sCurrWindowCtx->rStickInfo.xInverted = inverted;
}

void SetRStickYAxis(sf::Joystick::Axis rStickYAxis, bool inverted)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->rStickInfo.yAxis     = rStickYAxis;
    sCurrWindowCtx->rStickInfo.yInverted = inverted;
}

void SetLTriggerAxis(sf::Joystick::Axis lTriggerAxis)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->rTriggerInfo.axis = lTriggerAxis;
}

void SetRTriggerAxis(sf::Joystick::Axis rTriggerAxis)
{
    SFML_BASE_ASSERT(sCurrWindowCtx);
    sCurrWindowCtx->rTriggerInfo.axis = rTriggerAxis;
}


/////////////// Image Overloads for sf::Texture

void Image(const sf::Texture& texture, const sf::Color& tintColor, const sf::Color& borderColor)
{
    Image(texture, texture.getSize().to<sf::Vector2f>(), tintColor, borderColor);
}

void Image(const sf::Texture& texture, const sf::Vector2f& size, const sf::Color& tintColor, const sf::Color& borderColor)
{
    ImTextureID textureID = convertGLTextureHandleToImTextureID(texture.getNativeHandle());

    ::ImGui::Image(textureID, toImVec2(size), ImVec2(0, 0), ImVec2(1, 1), toImColor(tintColor), toImColor(borderColor));
}

/////////////// Image Overloads for sf::RenderTexture
void Image(const sf::RenderTexture& texture, const sf::Color& tintColor, const sf::Color& borderColor)
{
    Image(texture, texture.getSize().to<sf::Vector2f>(), tintColor, borderColor);
}

void Image(const sf::RenderTexture& texture, const sf::Vector2f& size, const sf::Color& tintColor, const sf::Color& borderColor)
{
    ImTextureID textureID = convertGLTextureHandleToImTextureID(texture.getTexture().getNativeHandle());

    ::ImGui::Image(textureID,
                   toImVec2(size),
                   ImVec2(0, 1),
                   ImVec2(1, 0), // flipped vertically,
                                 // because textures in
                                 // sf::RenderTexture are
                                 // stored this way
                   toImColor(tintColor),
                   toImColor(borderColor));
}

/////////////// Image Overloads for sf::Sprite

void Image(const sf::Sprite& sprite, const sf::Texture& texture, const sf::Color& tintColor, const sf::Color& borderColor)
{
    Image(sprite, texture, sprite.getGlobalBounds().size, tintColor, borderColor);
}

void Image(const sf::Sprite&   sprite,
           const sf::Texture&  texture,
           const sf::Vector2f& size,
           const sf::Color&    tintColor,
           const sf::Color&    borderColor)
{
    auto [uv0, uv1, textureID] = getSpriteTextureData(sprite, texture);
    ::ImGui::Image(textureID, toImVec2(size), uv0, uv1, toImColor(tintColor), toImColor(borderColor));
}

/////////////// Image Button Overloads for sf::Texture

bool ImageButton(const char*         id,
                 const sf::Texture&  texture,
                 const sf::Vector2f& size,
                 const sf::Color&    bgColor,
                 const sf::Color&    tintColor)
{
    ImTextureID textureID = convertGLTextureHandleToImTextureID(texture.getNativeHandle());

    return ::ImGui::ImageButton(id, textureID, toImVec2(size), ImVec2(0, 0), ImVec2(1, 1), toImColor(bgColor), toImColor(tintColor));
}

/////////////// Image Button Overloads for sf::RenderTexture

bool ImageButton(const char*              id,
                 const sf::RenderTexture& texture,
                 const sf::Vector2f&      size,
                 const sf::Color&         bgColor,
                 const sf::Color&         tintColor)
{
    ImTextureID textureID = convertGLTextureHandleToImTextureID(texture.getTexture().getNativeHandle());

    return ::ImGui::ImageButton(id,
                                textureID,
                                toImVec2(size),
                                ImVec2(0, 1),
                                ImVec2(1, 0), // flipped vertically, because textures in
                                              // sf::RenderTexture are stored this way
                                toImColor(bgColor),
                                toImColor(tintColor));
}

/////////////// Image Button Overloads for sf::Sprite

bool ImageButton(const char*         id,
                 const sf::Sprite&   sprite,
                 const sf::Texture&  texture,
                 const sf::Vector2f& size,
                 const sf::Color&    bgColor,
                 const sf::Color&    tintColor)
{
    auto [uv0, uv1, textureID] = getSpriteTextureData(sprite, texture);
    return ::ImGui::ImageButton(id, textureID, toImVec2(size), uv0, uv1, toImColor(bgColor), toImColor(tintColor));
}

/////////////// Draw_list Overloads

void DrawLine(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Color& color, float thickness)
{
    ImDrawList*  drawList = ::ImGui::GetWindowDrawList();
    const ImVec2 pos      = ::ImGui::GetCursorScreenPos();
    drawList->AddLine(ImVec2(a.x + pos.x, a.y + pos.y),
                      ImVec2(b.x + pos.x, b.y + pos.y),
                      ::ImGui::ColorConvertFloat4ToU32(toImColor(color)),
                      thickness);
}

void DrawRect(const sf::FloatRect& rect, const sf::Color& color, float rounding, int roundingCorners, float thickness)
{
    ImDrawList* drawList = ::ImGui::GetWindowDrawList();
    drawList->AddRect(getTopLeftAbsolute(rect),
                      getDownRightAbsolute(rect),
                      ::ImGui::ColorConvertFloat4ToU32(toImColor(color)),
                      rounding,
                      roundingCorners,
                      thickness);
}

void DrawRectFilled(const sf::FloatRect& rect, const sf::Color& color, float rounding, int roundingCorners)
{
    ImDrawList* drawList = ::ImGui::GetWindowDrawList();
    drawList->AddRectFilled(getTopLeftAbsolute(rect),
                            getDownRightAbsolute(rect),
                            ::ImGui::ColorConvertFloat4ToU32(toImColor(color)),
                            rounding,
                            roundingCorners);
}

} // namespace sf::ImGui
