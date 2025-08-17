#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#ifdef SFML_ENABLE_LIFETIME_TRACKING

    ////////////////////////////////////////////////////////////
    // Headers
    ////////////////////////////////////////////////////////////
    #include "SFML/System/LifetimeDependant.hpp"
    #include "SFML/System/LifetimeDependee.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
LifetimeDependant::LifetimeDependant(const char* const dependeeName, LifetimeDependee* dependee) noexcept :
    m_dependeeName(dependeeName),
    m_dependee(dependee)
{
    addSelfAsDependant();
}


////////////////////////////////////////////////////////////
LifetimeDependant::~LifetimeDependant()
{
    subSelfAsDependant();
}


////////////////////////////////////////////////////////////
LifetimeDependant::LifetimeDependant(const LifetimeDependant& rhs) noexcept :
    LifetimeDependant(rhs.m_dependeeName, rhs.m_dependee)
{
}


////////////////////////////////////////////////////////////
LifetimeDependant::LifetimeDependant(LifetimeDependant&& rhs) noexcept :
    m_dependeeName(rhs.m_dependeeName),
    m_dependee(rhs.m_dependee)
{
    rhs.m_dependee = nullptr;
}


////////////////////////////////////////////////////////////
LifetimeDependant& LifetimeDependant::operator=(const LifetimeDependant& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    m_dependeeName = rhs.m_dependeeName;
    m_dependee     = rhs.m_dependee;
    addSelfAsDependant();

    return *this;
}


////////////////////////////////////////////////////////////
LifetimeDependant& LifetimeDependant::operator=(LifetimeDependant&& rhs) noexcept
{
    m_dependeeName = rhs.m_dependeeName;
    m_dependee     = rhs.m_dependee;
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
    if (m_dependee != nullptr && !LifetimeDependee::TestingModeGuard::fatalErrorTriggered(m_dependeeName))
        m_dependee->addDependant();
}


////////////////////////////////////////////////////////////
void LifetimeDependant::subSelfAsDependant()
{
    if (m_dependee != nullptr && !LifetimeDependee::TestingModeGuard::fatalErrorTriggered(m_dependeeName))
        m_dependee->subDependant();
}

} // namespace sf::priv

#endif // SFML_ENABLE_LIFETIME_TRACKING
