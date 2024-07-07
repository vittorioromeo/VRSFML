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
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
namespace ContextImpl
{
// This per-thread variable holds the current context for each thread
thread_local sf::Context* currentContext(nullptr);
} // namespace ContextImpl
} // namespace

namespace sf
{
////////////////////////////////////////////////////////////
Context::Context(GraphicsContext& graphicsContext) :
m_graphicsContext(&graphicsContext),
m_context(priv::GlContext::create())
{
    if (!setActive(true))
        priv::err() << "Failed to set context as active during construction" << priv::errEndl;
}


////////////////////////////////////////////////////////////
Context::~Context()
{
    if (m_context && !setActive(false))
        priv::err() << "Failed to set context as inactive during destruction" << priv::errEndl;
}


////////////////////////////////////////////////////////////
Context::Context(Context&& rhs) noexcept : m_graphicsContext(rhs.m_graphicsContext), m_context(SFML_MOVE(rhs.m_context))
{
    if (&rhs == ContextImpl::currentContext)
        ContextImpl::currentContext = this;
}


////////////////////////////////////////////////////////////
Context& Context::operator=(Context&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    m_graphicsContext = rhs.m_graphicsContext;
    m_context         = SFML_MOVE(rhs.m_context);

    if (&rhs == ContextImpl::currentContext)
        ContextImpl::currentContext = this;

    return *this;
}


////////////////////////////////////////////////////////////
bool Context::setActive(bool active)
{
    if (!m_context->setActive(active))
        return false;

    if (active)
        ContextImpl::currentContext = this;
    else if (this == ContextImpl::currentContext)
        ContextImpl::currentContext = nullptr;

    return true;
}


////////////////////////////////////////////////////////////
const ContextSettings& Context::getSettings() const
{
    return m_context->getSettings();
}


////////////////////////////////////////////////////////////
const Context* Context::getActiveContext()
{
    using ContextImpl::currentContext;

    // We have to check that the last activated sf::Context is still active (a RenderTarget activation may have deactivated it)
    if (currentContext != nullptr && currentContext->m_context.get() == priv::GlContext::getActiveContext())
        return currentContext;

    return nullptr;
}


////////////////////////////////////////////////////////////
std::uint64_t Context::getActiveContextId()
{
    return priv::GlContext::getActiveContextId();
}


////////////////////////////////////////////////////////////
bool Context::hasActiveContext()
{
    return priv::GlContext::hasActiveContext();
}


////////////////////////////////////////////////////////////
bool Context::isExtensionAvailable(const char* name)
{
    return priv::GlContext::isExtensionAvailable(name);
}


////////////////////////////////////////////////////////////
GlFunctionPointer Context::getFunction(const char* name)
{
    return priv::GlContext::getFunction(name);
}


////////////////////////////////////////////////////////////
Context::Context(GraphicsContext& graphicsContext, const ContextSettings& settings, const Vector2u& size) :
m_graphicsContext(&graphicsContext),
m_context(priv::GlContext::create(settings, size))
{
    if (!setActive(true))
        priv::err() << "Failed to set context as active during construction" << priv::errEndl;
}

} // namespace sf
