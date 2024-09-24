#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
class GlContext;
class JoystickManager;
class WindowImpl;
} // namespace sf::priv

namespace sf
{
class Window;
struct ContextSettings;
using GlFunctionPointer = void (*)();
} // namespace sf

struct TestContext;


namespace sf
{
////////////////////////////////////////////////////////////
class [[nodiscard]] WindowContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowContext();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    ~WindowContext();

    ////////////////////////////////////////////////////////////
    /// \brief Check whether a given OpenGL extension is available
    ///
    /// \param name Name of the extension to check for
    ///
    /// \return True if available, false if unavailable
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isExtensionAvailable(const char* name) const;

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
    [[nodiscard]] GlFunctionPointer getFunction(const char* name) const;

    ////////////////////////////////////////////////////////////
    /// \brief Notify unshared resources of context destruction
    ///
    ////////////////////////////////////////////////////////////
    void cleanupUnsharedFrameBuffers(priv::GlContext& glContext);

    ////////////////////////////////////////////////////////////
    /// \brief Get the currently active context's ID
    ///
    /// The context ID is used to identify contexts when
    /// managing unshareable OpenGL resources.
    ///
    /// \return The active context's ID or 0 if no context is currently active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getActiveThreadLocalGlContextId() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasActiveThreadLocalGlContext() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasActiveThreadLocalOrSharedGlContext() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    using GLLoadFn = GlFunctionPointer (*)(const char*);
    [[nodiscard]] GLLoadFn getGLLoadFn() const;

protected:
    ////////////////////////////////////////////////////////////
    /// Friend declarations
    ///
    ////////////////////////////////////////////////////////////
    friend priv::GlContext;
    friend Window;
    friend priv::WindowImpl;

    friend TestContext;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void ensureExtensionsInit() const;

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
    [[nodiscard]] base::UniquePtr<priv::GlContext> createGlContextImpl(const ContextSettings& contextSettings,
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
    [[nodiscard]] base::UniquePtr<priv::GlContext> createGlContext();

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
    [[nodiscard]] base::UniquePtr<priv::GlContext> createGlContext(const ContextSettings&  contextSettings,
                                                                   const priv::WindowImpl& owner,
                                                                   unsigned int            bitsPerPixel);


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    using UnsharedDeleteFn = void (*)(unsigned int);

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
    void registerUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId, UnsharedDeleteFn deleteFn);

    ////////////////////////////////////////////////////////////
    /// \brief Unregister an OpenGL object from its containing context
    ///
    /// \param object Object to be unregister
    ///
    ////////////////////////////////////////////////////////////
    void unregisterUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void onGlContextDestroyed(priv::GlContext& glContext);

public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setActiveThreadLocalGlContextToSharedContext(bool active);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActiveGlContextSharedContext() const;

    ////////////////////////////////////////////////////////////
    /// \brief Load OpenGL or OpenGL ES entry points using GLAD
    ///
    ////////////////////////////////////////////////////////////
    void loadGLEntryPointsViaGLAD() const;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Get the currently active context
    ///
    /// \return The currently active context or a null pointer if none is active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const priv::GlContext* getActiveThreadLocalGlContextPtr() const;

public: // TODO P0: to passkey
    ////////////////////////////////////////////////////////////
    /// \brief Get the "global" joystick manager
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] priv::JoystickManager& getJoystickManager();

    ////////////////////////////////////////////////////////////
    /// \brief Get the "global" joystick manager
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const priv::JoystickManager& getJoystickManager() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::WindowContext
/// \ingroup window
///
/// TODO P1: docs
///
////////////////////////////////////////////////////////////
