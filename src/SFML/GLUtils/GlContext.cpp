// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GlContext.hpp"

#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GlContextTypeImpl.hpp"
#include "SFML/GLUtils/GlFuncTypesImpl.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Builtin/Strstr.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
GlContext::~GlContext()
{
    WindowContext::onGlContextDestroyed(*this);
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
GlContext::GlContext(unsigned int id, const ContextSettings& contextSettings) : m_settings(contextSettings), m_id{id}
{
}


////////////////////////////////////////////////////////////
bool GlContext::initialize(const GlContext& sharedGlContext, const ContextSettings& requestedSettings)
{
    SFML_BASE_ASSERT(WindowContext::getActiveThreadLocalGlContextPtr() == this);

    const auto& derivedSharedGlContext = static_cast<const DerivedGlContextType&>(sharedGlContext);

    // Try the new way first
    auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(
        derivedSharedGlContext.getFunction("glGetIntegerv"));

    auto glGetErrorFunc = reinterpret_cast<glGetErrorFuncType>(derivedSharedGlContext.getFunction("glGetError"));

    if (!glGetIntegervFunc || !glGetErrorFunc)
    {
        err() << "Could not load necessary function to initialize OpenGL context";
        return false;
    }

#if defined(SFML_SYSTEM_EMSCRIPTEN)

    // Hardcoded for WebGL 2.0
    m_settings.majorVersion   = 3;
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
        (m_settings.stencilBits < requestedSettings.stencilBits) || (m_settings.depthBits < requestedSettings.depthBits))
    {
        err() << "Warning: The created OpenGL context does not fully meet the settings that were requested" << '\n'
              << "Requested: version = " << requestedSettings.majorVersion << "." << requestedSettings.minorVersion
              << " ; depth bits = " << requestedSettings.depthBits << " ; stencil bits = " << requestedSettings.stencilBits
              << " ; core = " << boolToString(requestedSettings.isCore())
              << " ; debug = " << boolToString(requestedSettings.isDebug()) << '\n'
              << "Created: version = " << m_settings.majorVersion << "." << m_settings.minorVersion
              << " ; depth bits = " << m_settings.depthBits << " ; stencil bits = " << m_settings.stencilBits
              << " ; core = " << boolToString(m_settings.isCore()) << " ; debug = " << boolToString(m_settings.isDebug());
    }
}

} // namespace sf::priv
