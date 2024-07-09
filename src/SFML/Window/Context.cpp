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

#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
Context::Context(GraphicsContext& graphicsContext) :
m_graphicsContext(&graphicsContext),
m_context(graphicsContext.createGlContext())
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
}


////////////////////////////////////////////////////////////
Context& Context::operator=(Context&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    m_graphicsContext = rhs.m_graphicsContext;
    m_context         = SFML_MOVE(rhs.m_context);

    return *this;
}


////////////////////////////////////////////////////////////
bool Context::setActive(bool active)
{
    return m_context->setActive(active);
}


////////////////////////////////////////////////////////////
const ContextSettings& Context::getSettings() const
{
    return m_context->getSettings();
}


////////////////////////////////////////////////////////////
Context::Context(GraphicsContext& graphicsContext, const ContextSettings& settings, const Vector2u& size) :
m_graphicsContext(&graphicsContext),
m_context(graphicsContext.createGlContext(settings, size))
{
    if (!setActive(true))
        priv::err() << "Failed to set context as active during construction" << priv::errEndl;
}

} // namespace sf
