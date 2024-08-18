#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#if defined(SFML_SYSTEM_WINDOWS)
#include "SFML/Window/Win32/CursorImpl.hpp"
#elif !defined(SFML_USE_DRM) && (defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD) || \
                                 defined(SFML_SYSTEM_OPENBSD) || defined(SFML_SYSTEM_NETBSD))
#include "SFML/Window/Unix/CursorImpl.hpp"
#elif defined(SFML_SYSTEM_MACOS)
#include "SFML/Window/macOS/CursorImpl.hpp"
#elif defined(SFML_SYSTEM_EMSCRIPTEN) || defined(SFML_SYSTEM_ANDROID) || defined(SFML_SYSTEM_IOS) || defined(SFML_USE_DRM)
#include "SFML/Window/Stub/StubCursorImpl.hpp"
namespace sf::priv
{
class CursorImpl : public StubCursorImpl
{
    using StubCursorImpl::StubCursorImpl;
};
} // namespace sf::priv
#endif
