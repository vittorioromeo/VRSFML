#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Base/EnumClassBitwiseOps.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Structure defining the settings of the OpenGL
///        context attached to a window
///
////////////////////////////////////////////////////////////
struct ContextSettings
{
    ////////////////////////////////////////////////////////////
    /// \brief Bit flags describing OpenGL context attributes
    ///
    /// These flags control how the OpenGL context is created.
    /// `Core` selects a core profile (no fixed-function or
    /// deprecated functionality), `Debug` enables OpenGL's
    /// debug output extension, and `Default` is the
    /// non-debug compatibility context. `Core` and the
    /// non-`Core` defaults are mutually exclusive.
    ///
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] Attribute : unsigned int
    {
        Default = 0,      //!< Non-debug, compatibility context (mutually exclusive with `Core`)
        Core    = 1 << 0, //!< Request a core profile context
        Debug   = 1 << 2, //!< Request a debug context (enables `GL_KHR_debug` / equivalent)

        DefaultAndDebug = Default | Debug, //!< Compatibility context with debug output
        CoreAndDebug    = Core | Debug,    //!< Core profile context with debug output
    };

#ifdef SFML_OPENGL_ES
    #ifdef SFML_DEBUG
    static inline constexpr auto defaultAttributeFlags = Attribute::DefaultAndDebug;
    #else
    static inline constexpr auto defaultAttributeFlags = Attribute::Default;
    #endif
#else
    #ifdef SFML_DEBUG
    static inline constexpr auto defaultAttributeFlags = Attribute::CoreAndDebug;
    #else
    static inline constexpr auto defaultAttributeFlags = Attribute::Core;
    #endif
#endif

#if defined(SFML_SYSTEM_EMSCRIPTEN)
    static inline constexpr auto defaultMajorVersion = 3u; // `3` is WebGL 2
    static inline constexpr auto defaultMinorVersion = 0u;
#elif defined(SFML_OPENGL_ES)
    static inline constexpr auto defaultMajorVersion = 3u;
    static inline constexpr auto defaultMinorVersion = 1u;
#else
    static inline constexpr auto defaultMajorVersion = 4u;
    static inline constexpr auto defaultMinorVersion = 1u;
#endif

#if defined(SFML_SYSTEM_EMSCRIPTEN)
    static inline constexpr auto defaultDepthBits   = 8u;
    static inline constexpr auto defaultStencilBits = 8u;
#else
    static inline constexpr auto defaultDepthBits   = 0u;
    static inline constexpr auto defaultStencilBits = 0u;
#endif

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the context uses a core profile
    ///
    /// \return `true` if `attributeFlags` contains `Attribute::Core`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] bool isCore() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the context has the debug flag enabled
    ///
    /// \return `true` if `attributeFlags` contains `Attribute::Debug`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] bool isDebug() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int depthBits{defaultDepthBits};           //!< Bits of the depth buffer (0 = no depth buffer)
    unsigned int stencilBits{defaultStencilBits};       //!< Bits of the stencil buffer (0 = no stencil buffer)
    unsigned int majorVersion{defaultMajorVersion};     //!< Major version of the OpenGL context to create
    unsigned int minorVersion{defaultMinorVersion};     //!< Minor version of the OpenGL context to create
    Attribute    attributeFlags{defaultAttributeFlags}; //!< Bitmask of context creation flags (core / debug / ...)
};


////////////////////////////////////////////////////////////
SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(ContextSettings::Attribute);


////////////////////////////////////////////////////////////
inline bool ContextSettings::isCore() const
{
    return (attributeFlags & Attribute::Core) != Attribute{0u};
}


////////////////////////////////////////////////////////////
inline bool ContextSettings::isDebug() const
{
    return (attributeFlags & Attribute::Debug) != Attribute{0u};
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::ContextSettings
/// \ingroup window
///
/// `ContextSettings` describes the OpenGL context attached
/// to a window: the depth and stencil buffer sizes, the
/// requested OpenGL version, and a small set of attribute
/// flags (core profile, debug context, ...).
///
/// VRSFML's graphics module is designed to run on a core
/// profile OpenGL (or OpenGL ES / WebGL2) context, so the
/// defaults already select an appropriate version and the
/// `Core` attribute. Most users only need to touch
/// `ContextSettings` when they are layering custom OpenGL
/// rendering on top of SFML and need to request specific
/// extensions or a specific version.
///
/// The `depthBits` and `stencilBits` members define the
/// number of bits per pixel requested for the depth and
/// stencil buffers respectively. A value of `0` means no
/// buffer of that kind is requested.
///
/// `majorVersion` and `minorVersion` define the version of
/// the OpenGL (or OpenGL ES / WebGL) context to create.
///
/// `attributeFlags` selects between core / compatibility
/// profile and toggles the debug context flag.
///
/// <b>Special Note for macOS:</b>
/// Apple only supports choosing between either a legacy context
/// (OpenGL 2.1) or a core context (OpenGL version depends on the
/// operating system version but is at least 3.2). Compatibility
/// contexts are not supported. Further information is available on the
/// <a href="https://developer.apple.com/opengl/capabilities/index.html">
/// OpenGL Capabilities Tables</a> page. macOS also currently does
/// not support debug contexts.
///
/// Please note that these values are only a hint. No failure
/// will be reported if one or more of these values are not
/// supported by the system; instead, SFML will try to find
/// the closest valid match. You can retrieve the settings
/// that the window actually used to create its context with
/// `sf::Window::getSettings()`.
///
////////////////////////////////////////////////////////////
