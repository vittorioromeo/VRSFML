////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/System/InPlacePImpl.hpp>
#include <SFML/System/Optional.hpp>
#include <SFML/System/UniquePtr.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
class GlContext;
class WindowImpl;
} // namespace sf::priv

namespace sf
{
using GlFunctionPointer = void (*)();
struct ContextSettings;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
class [[nodiscard]] GraphicsContext
{
public:
    // TODO: private
    [[nodiscard]] explicit GraphicsContext();
    ~GraphicsContext();

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context, not associated to a window
    ///
    /// This function automatically chooses the specialized class
    /// to use according to the OS.
    ///
    /// \return Pointer to the created context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] priv::UniquePtr<priv::GlContext> createGlContext();

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
    [[nodiscard]] priv::UniquePtr<priv::GlContext> createGlContext(const ContextSettings&  settings,
                                                                   const priv::WindowImpl& owner,
                                                                   unsigned int            bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context that embeds its own rendering target
    ///
    /// This function automatically chooses the specialized class
    /// to use according to the OS.
    ///
    /// \param settings Creation parameters
    /// \param size     Back buffer width and height
    ///
    /// \return Pointer to the created context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] priv::UniquePtr<priv::GlContext> createGlContext(const ContextSettings& settings, const Vector2u& size);

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
    /// \brief Register an OpenGL object to be destroyed when its containing context is destroyed
    ///
    /// This is used for internal purposes in order to properly
    /// clean up OpenGL resources that cannot be shared bwteen
    /// contexts.
    ///
    /// \param object Object to be destroyed when its containing context is destroyed
    ///
    ////////////////////////////////////////////////////////////
    void registerUnsharedGlObject(void* objectSharedPtr);

    ////////////////////////////////////////////////////////////
    /// \brief Unregister an OpenGL object from its containing context
    ///
    /// \param object Object to be unregister
    ///
    ////////////////////////////////////////////////////////////
    void unregisterUnsharedGlObject(void* objectSharedPtr);

    ////////////////////////////////////////////////////////////
    /// \brief Notify unshared resources of context destruction
    ///
    ////////////////////////////////////////////////////////////
    void cleanupUnsharedResources();

    ////////////////////////////////////////////////////////////
    /// \brief Get the currently active context
    ///
    /// \return The currently active context or a null pointer if none is active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const priv::GlContext* getActiveThreadLocalGlContextPtr() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the currently active context's ID
    ///
    /// The context ID is used to identify contexts when
    /// managing unshareable OpenGL resources.
    ///
    /// \return The active context's ID or 0 if no context is currently active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint64_t getActiveThreadLocalGlContextId() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasActiveThreadLocalGlContext() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    void onGlContextDestroyed(priv::GlContext& glContext);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasAnyActiveGlContext() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActiveGlContextSharedContext() const;

private:
    struct Impl;
    priv::UniquePtr<Impl> m_impl;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::GraphicsContext
/// \ingroup graphics
///
/// TODO
///
////////////////////////////////////////////////////////////
