#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GlContext.hpp"
#include "SFML/Window/GlContextTypeImpl.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Builtins/Strstr.hpp"

#include <cctype>
#include <cstdlib>


namespace sf::priv
{
////////////////////////////////////////////////////////////
GlContext::~GlContext()
{
    m_windowContext.onGlContextDestroyed(*this);
}


////////////////////////////////////////////////////////////
const ContextSettings& GlContext::getSettings() const
{
    return m_settings;
}


////////////////////////////////////////////////////////////
unsigned int GlContext::getId() const
{
    return m_id;
}


////////////////////////////////////////////////////////////
GlContext::GlContext(WindowContext& windowContext, unsigned int id, const ContextSettings& contextSettings) :
m_settings(contextSettings),
m_windowContext(windowContext),
m_id{id}
{
}


////////////////////////////////////////////////////////////
int GlContext::evaluateFormat(
    unsigned int           bitsPerPixel,
    const ContextSettings& contextSettings,
    int                    colorBits,
    int                    depthBits,
    int                    stencilBits,
    int                    antiAliasing,
    bool                   accelerated,
    bool                   sRgb)
{
    // Weight sub-scores so that better contextSettings don't score equally as bad as worse contextSettings
    const auto adjustNegativeScore = [](int x) { return static_cast<unsigned int>(x * (x > 0 ? 100'000 : -1)); };

    const auto colorDiff   = adjustNegativeScore(static_cast<int>(bitsPerPixel) - colorBits);
    const auto depthDiff   = adjustNegativeScore(static_cast<int>(contextSettings.depthBits) - depthBits);
    const auto stencilDiff = adjustNegativeScore(static_cast<int>(contextSettings.stencilBits) - stencilBits);
    const auto antiAliasingDiff = adjustNegativeScore(static_cast<int>(contextSettings.antiAliasingLevel) - antiAliasing);

    // Aggregate the scores
    return static_cast<int>(
        colorDiff + depthDiff + stencilDiff + antiAliasingDiff +
        // If the user wants an sRGB capable format, try really hard to get one
        ((contextSettings.sRgbCapable && !sRgb) ? 10'000'000 : 0) +
        // Make sure we prefer hardware acceleration over features
        (!accelerated ? 100'000'000 : 0));
}


////////////////////////////////////////////////////////////
bool GlContext::initialize(const GlContext& sharedGlContext, const ContextSettings& requestedSettings)
{
    SFML_BASE_ASSERT(m_windowContext.getActiveThreadLocalGlContextPtr() == this);

    const auto& derivedSharedGlContext = static_cast<const DerivedGlContextType&>(sharedGlContext);

    // Try the new way first
    auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(
        derivedSharedGlContext.getFunction("glGetIntegerv"));
    auto glGetErrorFunc  = reinterpret_cast<glGetErrorFuncType>(derivedSharedGlContext.getFunction("glGetError"));
    auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(derivedSharedGlContext.getFunction("glGetString"));
    auto glEnableFunc    = reinterpret_cast<glEnableFuncType>(derivedSharedGlContext.getFunction("glEnable"));
    auto glIsEnabledFunc = reinterpret_cast<glIsEnabledFuncType>(derivedSharedGlContext.getFunction("glIsEnabled"));

    if (!glGetIntegervFunc || !glGetErrorFunc || !glGetStringFunc || !glEnableFunc || !glIsEnabledFunc)
    {
        err() << "Could not load necessary function to initialize OpenGL context";
        return false;
    }

#if defined(SFML_SYSTEM_EMSCRIPTEN)

    // Hardcoded for WebGL 2.0
    m_settings.majorVersion   = 2;
    m_settings.minorVersion   = 0;
    m_settings.attributeFlags = requestedSettings.attributeFlags;

#else

    // Retrieve the context version number
    int majorVersion = 0;
    glCheckIgnoreWithFunc(glGetErrorFunc, glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion));
    m_settings.majorVersion = static_cast<unsigned int>(majorVersion);

    if (m_settings.majorVersion < 3)
    {
        err() << "Context major version below 3.x is not supported";
        return false;
    }

    int minorVersion = 0;
    glCheckIgnoreWithFunc(glGetErrorFunc, glGetIntegervFunc(GL_MINOR_VERSION, &minorVersion));
    m_settings.minorVersion = static_cast<unsigned int>(minorVersion);

    m_settings.attributeFlags = requestedSettings.attributeFlags;

    // Retrieve the context flags
    int flags = 0;
    glCheckIgnoreWithFunc(glGetErrorFunc, glGetIntegervFunc(GL_CONTEXT_FLAGS, &flags));

    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        m_settings.attributeFlags |= ContextSettings::Attribute::Debug;

    if ((m_settings.majorVersion == 3) && (m_settings.minorVersion == 1))
    {
        // OpenGL ES likely hits this path

        m_settings.attributeFlags |= ContextSettings::Attribute::Core;

        if (auto glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(derivedSharedGlContext.getFunction("glGetStr"
                                                                                                              "ingi")))
        {
            int numExtensions = 0;
            glCheckIgnoreWithFunc(glGetErrorFunc, glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions));

            for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
            {
                const auto* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i));

                if (SFML_BASE_STRSTR(extensionString, "GL_ARB_compatibility"))
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
        glCheckIgnoreWithFunc(glGetErrorFunc, glGetIntegervFunc(GL_CONTEXT_PROFILE_MASK, &profile));

        if (profile & GL_CONTEXT_CORE_PROFILE_BIT)
            m_settings.attributeFlags |= ContextSettings::Attribute::Core;
    }

#endif

    // Enable anti-aliasing if requested by the user and supported
    if ((requestedSettings.antiAliasingLevel > 0) && (m_settings.antiAliasingLevel > 0))
    {
        glCheckIgnoreWithFunc(glGetErrorFunc, glEnableFunc(GL_MULTISAMPLE));
    }
    else
    {
        m_settings.antiAliasingLevel = 0;
    }

    // Enable sRGB if requested by the user and supported
    if (requestedSettings.sRgbCapable && m_settings.sRgbCapable)
    {
        glCheckIgnoreWithFunc(glGetErrorFunc, glEnableFunc(GL_FRAMEBUFFER_SRGB));

        // Check to see if the enable was successful
        if (glCheckIgnoreWithFunc(glGetErrorFunc, glIsEnabledFunc(GL_FRAMEBUFFER_SRGB)) == GL_FALSE)
        {
            err() << "Warning: Failed to enable GL_FRAMEBUFFER_SRGB";
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
    const auto boolToString = [](bool b) { return b ? "true" : "false"; };

    // Perform checks to inform the user if they are getting a context they might not have expected
    const int version = static_cast<int>(m_settings.majorVersion * 10u + m_settings.minorVersion);
    const int requestedVersion = static_cast<int>(requestedSettings.majorVersion * 10u + requestedSettings.minorVersion);

    if ((m_settings.attributeFlags != requestedSettings.attributeFlags) || (version < requestedVersion) ||
        (m_settings.stencilBits < requestedSettings.stencilBits) ||
        (m_settings.antiAliasingLevel < requestedSettings.antiAliasingLevel) ||
        (m_settings.depthBits < requestedSettings.depthBits) || (!m_settings.sRgbCapable && requestedSettings.sRgbCapable))
    {
        err() << "Warning: The created OpenGL context does not fully meet the settings that were requested" << '\n'
              << "Requested: version = " << requestedSettings.majorVersion << "." << requestedSettings.minorVersion
              << " ; depth bits = " << requestedSettings.depthBits << " ; stencil bits = " << requestedSettings.stencilBits
              << " ; AA level = " << requestedSettings.antiAliasingLevel << " ; core = "
              << boolToString((requestedSettings.attributeFlags & ContextSettings::Attribute::Core) !=
                              ContextSettings::Attribute{0u})
              << " ; debug = "
              << boolToString((requestedSettings.attributeFlags & ContextSettings::Attribute::Debug) !=
                              ContextSettings::Attribute{0u})
              << " ; sRGB = " << requestedSettings.sRgbCapable << '\n'
              << "Created: version = " << m_settings.majorVersion << "." << m_settings.minorVersion
              << " ; depth bits = " << m_settings.depthBits << " ; stencil bits = " << m_settings.stencilBits
              << " ; AA level = " << m_settings.antiAliasingLevel << " ; core = "
              << boolToString((m_settings.attributeFlags & ContextSettings::Attribute::Core) !=
                              ContextSettings::Attribute{0u})
              << " ; debug = "
              << boolToString((m_settings.attributeFlags & ContextSettings::Attribute::Debug) !=
                              ContextSettings::Attribute{0u})
              << " ; sRGB = " << m_settings.sRgbCapable;
    }
}

} // namespace sf::priv
