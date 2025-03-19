#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
class EglContext;
class GlContext;
class JoystickManager;
class SensorManager;
class WindowImpl;
} // namespace sf::priv


namespace sf
{
class GraphicsContext;
class RenderTarget;
class RenderTexture;
class Shader;
class Texture;
class VertexBuffer;
class Window;
struct ContextSettings;
struct Joystick;
struct Sensor;
struct WindowContextImpl;
using GlFunctionPointer = void (*)();
} // namespace sf

struct TestContext;


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_WINDOW_API WindowContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<WindowContext> create();

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowContext(base::PassKey<WindowContext>&&);
    [[nodiscard]] explicit WindowContext(base::PassKey<GraphicsContext>&&);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    ~WindowContext();

    // Deleted copy operations
    WindowContext(const WindowContext&)            = delete;
    WindowContext& operator=(const WindowContext&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    WindowContext(WindowContext&&) noexcept;

    // Deleted move assignment
    WindowContext& operator=(WindowContext&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P0: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isInstalled();

private:
    ////////////////////////////////////////////////////////////
    /// Friend declarations
    ///
    ////////////////////////////////////////////////////////////
    friend GraphicsContext;   // for `setActiveThreadLocalGlContext`
    friend Joystick;          // for `getJoystickManager`
    friend priv::EglContext;  // for `setActiveThreadLocalGlContext`
    friend priv::GlContext;   // for `onGlContextDestroyed` and `getActiveThreadLocalGlContextPtr`
    friend priv::WindowImpl;  // for `getJoystickManager`
    friend RenderTarget;      // for `getActiveThreadLocalGlContextId`
    friend RenderTexture;     // for `[un]registerUnsharedFrameBuffer`
    friend Sensor;            // for `getSensorManager`
    friend Shader;            // for `hasActiveThreadLocalGlContext` and `hasActiveThreadLocalOrSharedGlContext`
    friend TestContext;       // for `createGlContext`
    friend Texture;           // for `hasActiveThreadLocalGlContext` and `hasActiveThreadLocalOrSharedGlContext`
    friend VertexBuffer;      // for `hasActiveThreadLocalGlContext` and `hasActiveThreadLocalOrSharedGlContext`
    friend Window;            // for `createGlContext`
    friend WindowContextImpl; // for `UnsharedDeleteFn`

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context, not associated to a window
    ///
    /// This function automatically chooses the specialized class
    /// to use according to the OS.
    ///
    /// \return Pointer to the created context
    ///
    ////////////////////////////////////////////////////////////
    template <typename... GLContextArgs>
    [[nodiscard]] static base::UniquePtr<priv::GlContext> createGlContextImpl(const ContextSettings& contextSettings,
                                                                              GLContextArgs&&... args);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context, not associated to a window
    ///
    /// This function automatically chooses the specialized class
    /// to use according to the OS.
    ///
    /// \return Pointer to the created context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<priv::GlContext> createGlContext();

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context attached to a window
    ///
    /// This function automatically chooses the specialized class
    /// to use according to the OS.
    ///
    /// \param settings     Creation parameters
    /// \param owner        Pointer to the owner window
    /// \param bitsPerPixel Pixel depth (in bits per pixel)
    ///
    /// \return Pointer to the created context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<priv::GlContext> createGlContext(const ContextSettings&  contextSettings,
                                                                          const priv::WindowImpl& owner,
                                                                          unsigned int            bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    using UnsharedDeleteFn = void (*)(unsigned int);

    ////////////////////////////////////////////////////////////
    /// \brief Notify unshared resources of context destruction
    ///
    ////////////////////////////////////////////////////////////
    static void cleanupUnsharedFrameBuffers(priv::GlContext& glContext);

    ////////////////////////////////////////////////////////////
    /// \brief Get the currently active context's ID
    ///
    /// The context ID is used to identify contexts when
    /// managing unshareable OpenGL resources.
    ///
    /// \return The active context's ID or 0 if no context is currently active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getActiveThreadLocalGlContextId();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool hasActiveThreadLocalGlContext();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool hasActiveThreadLocalOrSharedGlContext();

    ////////////////////////////////////////////////////////////
    /// \brief Register an OpenGL object to be destroyed when its containing context is destroyed
    ///
    /// This is used for internal purposes in order to properly
    /// clean up OpenGL resources that cannot be shared between
    /// contexts.
    ///
    /// \param object Object to be destroyed when its containing context is destroyed
    ///
    ////////////////////////////////////////////////////////////
    static void registerUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId, UnsharedDeleteFn deleteFn);

    ////////////////////////////////////////////////////////////
    /// \brief Unregister an OpenGL object from its containing context
    ///
    /// \param object Object to be unregister
    ///
    ////////////////////////////////////////////////////////////
    static void unregisterUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    static void onGlContextDestroyed(priv::GlContext& glContext);

    ////////////////////////////////////////////////////////////
    /// \brief Check whether a given OpenGL extension is available
    ///
    /// \param name Name of the extension to check for
    ///
    /// \return `true` if available, false if unavailable
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isExtensionAvailable(const char* name);

    ////////////////////////////////////////////////////////////
    /// \brief Get the address of an OpenGL function
    ///
    /// On Windows when not using OpenGL ES, a context must be
    /// active for this function to succeed.
    ///
    /// \param name Name of the function to get the address of
    ///
    /// \return Address of the OpenGL function, 0 on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static GlFunctionPointer getFunction(const char* name);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool setActiveThreadLocalGlContextToSharedContext(bool active);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isActiveGlContextSharedContext();

    ////////////////////////////////////////////////////////////
    /// \brief Load OpenGL or OpenGL ES entry points using GLAD
    ///
    ////////////////////////////////////////////////////////////
    static void loadGLEntryPointsViaGLAD();

    ////////////////////////////////////////////////////////////
    /// \brief Return the currently active context or a null pointer if none is active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static const priv::GlContext* getActiveThreadLocalGlContextPtr();

    ////////////////////////////////////////////////////////////
    /// \brief Get the "global" joystick manager
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static priv::JoystickManager& getJoystickManager();

    ////////////////////////////////////////////////////////////
    /// \brief Get the "global" sensor manager
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static priv::SensorManager& getSensorManager();
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::WindowContext
/// \ingroup window
///
/// TODO P1: docs
///
////////////////////////////////////////////////////////////
