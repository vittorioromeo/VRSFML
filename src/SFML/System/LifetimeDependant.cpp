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

#ifdef SFML_ENABLE_LIFETIME_TRACKING

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/System/LifetimeDependant.hpp>
#include <SFML/System/LifetimeDependee.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
LifetimeDependant::LifetimeDependant(LifetimeDependee* dependee) noexcept : m_dependee(dependee)
{
    addSelfAsDependant();
}


////////////////////////////////////////////////////////////
LifetimeDependant::~LifetimeDependant()
{
    subSelfAsDependant();
}


////////////////////////////////////////////////////////////
LifetimeDependant::LifetimeDependant(const LifetimeDependant& rhs) noexcept : LifetimeDependant(rhs.m_dependee)
{
}


////////////////////////////////////////////////////////////
LifetimeDependant::LifetimeDependant(LifetimeDependant&& rhs) noexcept : m_dependee(rhs.m_dependee)
{
    rhs.m_dependee = nullptr;
}


////////////////////////////////////////////////////////////
LifetimeDependant& LifetimeDependant::operator=(const LifetimeDependant& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    m_dependee = rhs.m_dependee;
    addSelfAsDependant();

    return *this;
}


////////////////////////////////////////////////////////////
LifetimeDependant& LifetimeDependant::operator=(LifetimeDependant&& rhs) noexcept
{
    m_dependee     = rhs.m_dependee;
    rhs.m_dependee = nullptr;

    return *this;
}


////////////////////////////////////////////////////////////
void LifetimeDependant::update(LifetimeDependee* dependee) noexcept
{
    subSelfAsDependant();
    m_dependee = dependee;
    addSelfAsDependant();
}


////////////////////////////////////////////////////////////
void LifetimeDependant::addSelfAsDependant()
{
    if (m_dependee != nullptr && !LifetimeDependee::TestingModeGuard::fatalErrorTriggered())
        m_dependee->addDependant();
}


////////////////////////////////////////////////////////////
void LifetimeDependant::subSelfAsDependant()
{
    if (m_dependee != nullptr && !LifetimeDependee::TestingModeGuard::fatalErrorTriggered())
        m_dependee->subDependant();
}

} // namespace sf::priv

#endif // SFML_ENABLE_LIFETIME_TRACKING
