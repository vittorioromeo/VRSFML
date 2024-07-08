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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Context.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GlContextTypeImpl.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <glad/gl.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>


namespace sf::priv
{
// This structure contains all the implementation data we
// don't want to expose through the visible interface
struct GlContext::Impl
{
    ////////////////////////////////////////////////////////////
    /// \brief Constructor
    ///
    ////////////////////////////////////////////////////////////
    Impl()
    {
        auto& weakUnsharedGlObjects = getWeakUnsharedGlObjects();
        unsharedGlObjects           = weakUnsharedGlObjects.lock();

        if (!unsharedGlObjects)
        {
            unsharedGlObjects = std::make_shared<UnsharedGlObjects>();

            weakUnsharedGlObjects = unsharedGlObjects;
        }
    }

    // Structure to track which unshared object belongs to which context
    struct UnsharedGlObject
    {
        std::uint64_t         contextId{};
        std::shared_ptr<void> object;
    };

    using UnsharedGlObjects = std::vector<UnsharedGlObject>;

    ////////////////////////////////////////////////////////////
    /// \brief Get weak_ptr to unshared objects
    ///
    /// \return weak_ptr to unshared objects
    ///
    ////////////////////////////////////////////////////////////
    static std::weak_ptr<UnsharedGlObjects>& getWeakUnsharedGlObjects()
    {
        static std::weak_ptr<UnsharedGlObjects> weakUnsharedGlObjects;
        return weakUnsharedGlObjects;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get mutex protecting unshared objects
    ///
    /// \return Mutex protecting unshared objects
    ///
    ////////////////////////////////////////////////////////////
    static std::mutex& getUnsharedGlObjectsMutex()
    {
        static std::mutex mutex;
        return mutex;
    }

    ///////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::shared_ptr<UnsharedGlObjects> unsharedGlObjects; //!< The current object's handle to unshared objects
    const std::uint64_t                id{
        []
        {
            static std::atomic<std::uint64_t> atomicId(1); // start at 1, zero is "no context"
            return atomicId.fetch_add(1);
        }()}; //!< Unique identifier, used for identifying contexts when managing unshareable OpenGL resources
};


////////////////////////////////////////////////////////////
void GlContext::registerUnsharedGlObject(void* objectSharedPtr)
{
    const std::lock_guard lock(Impl::getUnsharedGlObjectsMutex());

    if (const std::shared_ptr unsharedGlObjects = Impl::getWeakUnsharedGlObjects().lock())
        unsharedGlObjects->emplace_back(GraphicsContext::getActiveContextId(),
                                        SFML_MOVE(*static_cast<std::shared_ptr<void>*>(objectSharedPtr)));
}


////////////////////////////////////////////////////////////
void GlContext::unregisterUnsharedGlObject(void* objectSharedPtr)
{
    const std::lock_guard lock(Impl::getUnsharedGlObjectsMutex());

    if (const std::shared_ptr unsharedGlObjects = Impl::getWeakUnsharedGlObjects().lock())
    {
        const auto currentContextId = GraphicsContext::getActiveContextId();

        // Find the object in unshared objects and remove it if its associated context is currently active
        // This will trigger the destructor of the object since shared_ptr
        // in unshared objects should be the only one existing
        const auto iter = findIf(unsharedGlObjects->begin(),
                                 unsharedGlObjects->end(),
                                 [&](const Impl::UnsharedGlObject& obj) {
                                     return obj.contextId == currentContextId &&
                                            obj.object == *static_cast<std::shared_ptr<void>*>(objectSharedPtr);
                                 });

        if (iter != unsharedGlObjects->end())
            unsharedGlObjects->erase(iter);
    }
}


////////////////////////////////////////////////////////////
bool GlContext::isExtensionAvailable(GraphicsContext& graphicsContext, const char* name)
{
    return graphicsContext.isExtensionAvailable(name);
}


////////////////////////////////////////////////////////////
GlContext::~GlContext()
{
    auto& [id, ptr, transientCount] = GraphicsContext::CurrentContext::get();

    if (m_impl->id == id)
    {
        id  = 0;
        ptr = nullptr;
    }
}


////////////////////////////////////////////////////////////
const ContextSettings& GlContext::getSettings() const
{
    return m_settings;
}


////////////////////////////////////////////////////////////
bool GlContext::setActive(bool active)
{
    auto& [id, ptr, transientCount] = GraphicsContext::CurrentContext::get();

    // If this context is already the active one on this thread, don't do anything
    if (active && m_impl->id == id)
        return true;

    // If this context is not the active one on this thread, don't do anything
    if (!active && m_impl->id != id)
        return true;

    const auto activate = [&]
    {
        // Activate the context
        if (!makeCurrent(true))
        {
            err() << "makeCurrent(true) failure in GlContext::setActive" << errEndl;
            return false;
        }

        // Set it as the new current context for this thread
        id  = m_impl->id;
        ptr = this;

        return true;
    };

    const auto deactivate = [&]
    {
        // Deactivate the context
        if (!makeCurrent(false))
        {
            err() << "makeCurrent(false) failure in GlContext::setActive" << errEndl;
            return false;
        }

        id  = 0;
        ptr = nullptr;

        return true;
    };

    return active ? activate() : deactivate();
}


////////////////////////////////////////////////////////////
GlContext::GlContext(const ContextSettings& settings) : m_settings(settings)
{
}


////////////////////////////////////////////////////////////
int GlContext::evaluateFormat(
    unsigned int           bitsPerPixel,
    const ContextSettings& settings,
    int                    colorBits,
    int                    depthBits,
    int                    stencilBits,
    int                    antialiasing,
    bool                   accelerated,
    bool                   sRgb)
{
    int colorDiff        = static_cast<int>(bitsPerPixel) - colorBits;
    int depthDiff        = static_cast<int>(settings.depthBits) - depthBits;
    int stencilDiff      = static_cast<int>(settings.stencilBits) - stencilBits;
    int antialiasingDiff = static_cast<int>(settings.antialiasingLevel) - antialiasing;

    // Weight sub-scores so that better settings don't score equally as bad as worse settings
    colorDiff *= ((colorDiff > 0) ? 100000 : 1);
    depthDiff *= ((depthDiff > 0) ? 100000 : 1);
    stencilDiff *= ((stencilDiff > 0) ? 100000 : 1);
    antialiasingDiff *= ((antialiasingDiff > 0) ? 100000 : 1);

    // Aggregate the scores
    int score = std::abs(colorDiff) + std::abs(depthDiff) + std::abs(stencilDiff) + std::abs(antialiasingDiff);

    // If the user wants an sRGB capable format, try really hard to get one
    if (settings.sRgbCapable && !sRgb)
        score += 10000000;

    // Make sure we prefer hardware acceleration over features
    if (!accelerated)
        score += 100000000;

    return score;
}


////////////////////////////////////////////////////////////
void GlContext::cleanupUnsharedResources()
{
    const auto& [id, ptr, transientCount] = GraphicsContext::CurrentContext::get();

    // Save the current context so we can restore it later
    GlContext* contextToRestore = ptr;

    // If this context is already active there is no need to save it
    if (m_impl->id == id)
        contextToRestore = nullptr;

    // Make this context active so resources can be freed
    if (!setActive(true))
        err() << "Could not enable context in GlContext::cleanupUnsharedResources()" << errEndl;

    {
        const std::lock_guard lock(Impl::getUnsharedGlObjectsMutex());

        // Destroy the unshared objects contained in this context
        for (auto iter = m_impl->unsharedGlObjects->begin(); iter != m_impl->unsharedGlObjects->end();)
        {
            if (iter->contextId == m_impl->id)
            {
                iter = m_impl->unsharedGlObjects->erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    // Make the originally active context active again
    if (contextToRestore)
        if (!contextToRestore->setActive(true))
            err() << "Could not restore context in GlContext::cleanupUnsharedResources()" << errEndl;
}


////////////////////////////////////////////////////////////
bool GlContext::initialize(const ContextSettings& requestedSettings)
{
    // Activate the context
    if (!setActive(true))
        err() << "Error enabling context in GlContext::initalize()" << errEndl;

    // Retrieve the context version number
    int majorVersion = 0;
    int minorVersion = 0;

    // Try the new way first
    auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(ContextType::getFunction("glGetIntegerv"));
    auto glGetErrorFunc    = reinterpret_cast<glGetErrorFuncType>(ContextType::getFunction("glGetError"));
    auto glGetStringFunc   = reinterpret_cast<glGetStringFuncType>(ContextType::getFunction("glGetString"));
    auto glEnableFunc      = reinterpret_cast<glEnableFuncType>(ContextType::getFunction("glEnable"));
    auto glIsEnabledFunc   = reinterpret_cast<glIsEnabledFuncType>(ContextType::getFunction("glIsEnabled"));

    if (!glGetIntegervFunc || !glGetErrorFunc || !glGetStringFunc || !glEnableFunc || !glIsEnabledFunc)
    {
        err() << "Could not load necessary function to initialize OpenGL context" << errEndl;
        return false;
    }

    glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegervFunc(GL_MINOR_VERSION, &minorVersion);

    if (glGetErrorFunc() != GL_INVALID_ENUM)
    {
        m_settings.majorVersion = static_cast<unsigned int>(majorVersion);
        m_settings.minorVersion = static_cast<unsigned int>(minorVersion);
    }
    else
    {
        // Try the old way

        // If we can't get the version number, assume 1.1
        m_settings.majorVersion = 1;
        m_settings.minorVersion = 1;

        const char* version = reinterpret_cast<const char*>(glGetStringFunc(GL_VERSION));
        if (version)
        {
            // OpenGL ES Common Lite profile: The beginning of the returned string is "OpenGL ES-CL major.minor"
            // OpenGL ES Common profile:      The beginning of the returned string is "OpenGL ES-CM major.minor"
            // OpenGL ES Full profile:        The beginning of the returned string is "OpenGL ES major.minor"
            // Desktop OpenGL:                The beginning of the returned string is "major.minor"

            // Helper to parse OpenGL version strings
            static const auto parseVersionString =
                [](const char* versionString, const char* prefix, unsigned int& major, unsigned int& minor)
            {
                const std::size_t prefixLength = std::strlen(prefix);

                if ((std::strlen(versionString) >= (prefixLength + 3)) &&
                    (std::strncmp(versionString, prefix, prefixLength) == 0) && std::isdigit(versionString[prefixLength]) &&
                    (versionString[prefixLength + 1] == '.') && std::isdigit(versionString[prefixLength + 2]))
                {
                    major = static_cast<unsigned int>(versionString[prefixLength] - '0');
                    minor = static_cast<unsigned int>(versionString[prefixLength + 2] - '0');

                    return true;
                }

                return false;
            };

            if (!parseVersionString(version, "OpenGL ES-CL ", m_settings.majorVersion, m_settings.minorVersion) &&
                !parseVersionString(version, "OpenGL ES-CM ", m_settings.majorVersion, m_settings.minorVersion) &&
                !parseVersionString(version, "OpenGL ES ", m_settings.majorVersion, m_settings.minorVersion) &&
                !parseVersionString(version, "", m_settings.majorVersion, m_settings.minorVersion))
            {
                err() << "Unable to parse OpenGL version string: \"" << version << '"' << ", defaulting to 1.1" << errEndl;
            }
        }
        else
        {
            err() << "Unable to retrieve OpenGL version string, defaulting to 1.1" << errEndl;
        }
    }

    // 3.0 contexts only deprecate features, but do not remove them yet
    // 3.1 contexts remove features if ARB_compatibility is not present
    // 3.2+ contexts remove features only if a core profile is requested

    // If the context was created with wglCreateContext, it is guaranteed to be compatibility.
    // If a 3.0 context was created with wglCreateContextAttribsARB, it is guaranteed to be compatibility.
    // If a 3.1 context was created with wglCreateContextAttribsARB, the compatibility flag
    // is set only if ARB_compatibility is present
    // If a 3.2+ context was created with wglCreateContextAttribsARB, the compatibility flag
    // would have been set correctly already depending on whether ARB_create_context_profile is supported.

    // If the user requests a 3.0 context, it will be a compatibility context regardless of the requested profile.
    // If the user requests a 3.1 context and its creation was successful, the specification
    // states that it will not be a compatibility profile context regardless of the requested
    // profile unless ARB_compatibility is present.

    m_settings.attributeFlags = ContextSettings::Attribute::Default;

    if (m_settings.majorVersion >= 3)
    {
        // Retrieve the context flags
        int flags = 0;
        glGetIntegervFunc(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
            m_settings.attributeFlags |= ContextSettings::Attribute::Debug;

        if ((m_settings.majorVersion == 3) && (m_settings.minorVersion == 1))
        {
            m_settings.attributeFlags |= ContextSettings::Attribute::Core;

            auto glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(ContextType::getFunction("glGetStringi"));

            if (glGetStringiFunc)
            {
                int numExtensions = 0;
                glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions);

                for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
                {
                    const char* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i));

                    if (std::strstr(extensionString, "GL_ARB_compatibility"))
                    {
                        m_settings.attributeFlags &= ~ContextSettings::Attribute::Core;
                        break;
                    }
                }
            }
        }
        else if ((m_settings.majorVersion > 3) || (m_settings.minorVersion >= 2))
        {
            // Retrieve the context profile
            int profile = 0;
            glGetIntegervFunc(GL_CONTEXT_PROFILE_MASK, &profile);

            if (profile & GL_CONTEXT_CORE_PROFILE_BIT)
                m_settings.attributeFlags |= ContextSettings::Attribute::Core;
        }
    }

    // Enable anti-aliasing if requested by the user and supported
    if ((requestedSettings.antialiasingLevel > 0) && (m_settings.antialiasingLevel > 0))
    {
        glEnableFunc(GL_MULTISAMPLE);
    }
    else
    {
        m_settings.antialiasingLevel = 0;
    }

    // Enable sRGB if requested by the user and supported
    if (requestedSettings.sRgbCapable && m_settings.sRgbCapable)
    {
        glEnableFunc(GL_FRAMEBUFFER_SRGB);

        // Check to see if the enable was successful
        if (glIsEnabledFunc(GL_FRAMEBUFFER_SRGB) == GL_FALSE)
        {
            err() << "Warning: Failed to enable GL_FRAMEBUFFER_SRGB" << errEndl;
            m_settings.sRgbCapable = false;
        }
    }
    else
    {
        m_settings.sRgbCapable = false;
    }

    return true;
}


////////////////////////////////////////////////////////////
void GlContext::checkSettings(const ContextSettings& requestedSettings) const
{
    // Perform checks to inform the user if they are getting a context they might not have expected
    const int version = static_cast<int>(m_settings.majorVersion * 10u + m_settings.minorVersion);
    const int requestedVersion = static_cast<int>(requestedSettings.majorVersion * 10u + requestedSettings.minorVersion);

    if ((m_settings.attributeFlags != requestedSettings.attributeFlags) || (version < requestedVersion) ||
        (m_settings.stencilBits < requestedSettings.stencilBits) ||
        (m_settings.antialiasingLevel < requestedSettings.antialiasingLevel) ||
        (m_settings.depthBits < requestedSettings.depthBits) || (!m_settings.sRgbCapable && requestedSettings.sRgbCapable))
    {
        err() << "Warning: The created OpenGL context does not fully meet the settings that were requested" << '\n'
              << "Requested: version = " << requestedSettings.majorVersion << "." << requestedSettings.minorVersion
              << " ; depth bits = " << requestedSettings.depthBits << " ; stencil bits = " << requestedSettings.stencilBits
              << " ; AA level = " << requestedSettings.antialiasingLevel << std::boolalpha << " ; core = "
              << ((requestedSettings.attributeFlags & ContextSettings::Attribute::Core) != ContextSettings::Attribute{0u})
              << " ; debug = "
              << ((requestedSettings.attributeFlags & ContextSettings::Attribute::Debug) != ContextSettings::Attribute{0u})
              << " ; sRGB = " << requestedSettings.sRgbCapable << std::noboolalpha << '\n'
              << "Created: version = " << m_settings.majorVersion << "." << m_settings.minorVersion
              << " ; depth bits = " << m_settings.depthBits << " ; stencil bits = " << m_settings.stencilBits
              << " ; AA level = " << m_settings.antialiasingLevel << std::boolalpha << " ; core = "
              << ((m_settings.attributeFlags & ContextSettings::Attribute::Core) != ContextSettings::Attribute{0u})
              << " ; debug = "
              << ((m_settings.attributeFlags & ContextSettings::Attribute::Debug) != ContextSettings::Attribute{0u})
              << " ; sRGB = " << m_settings.sRgbCapable << std::noboolalpha << errEndl;
    }
}

} // namespace sf::priv
