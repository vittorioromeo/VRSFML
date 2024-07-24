#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

#ifdef SFML_ENABLE_LIFETIME_TRACKING

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>


namespace sf::priv
{
class LifetimeDependee;

class SFML_SYSTEM_API LifetimeDependant
{
public:
    explicit LifetimeDependant(LifetimeDependee* dependee = nullptr) noexcept;
    ~LifetimeDependant();

    LifetimeDependant(const LifetimeDependant& rhs) noexcept;
    LifetimeDependant(LifetimeDependant&& rhs) noexcept;

    LifetimeDependant& operator=(const LifetimeDependant& rhs) noexcept;
    LifetimeDependant& operator=(LifetimeDependant&& rhs) noexcept;

    void update(LifetimeDependee* dependee) noexcept;

private:
    void addSelfAsDependant();
    void subSelfAsDependant();

    LifetimeDependee* m_dependee;
};

} // namespace sf::priv

// NOLINTBEGIN(bugprone-macro-parentheses)
#define SFML_DEFINE_LIFETIME_DEPENDANT(dependantType)                               \
    mutable ::sf::priv::LifetimeDependant m_sfPrivLifetimeDependant##dependantType; \
    using sfPrivSwallowSemicolon##dependantType = void

#define SFML_UPDATE_LIFETIME_DEPENDANT(dependantType, dependeeType, thisPtr, dependantMemberPtr) \
    thisPtr->m_sfPrivLifetimeDependant##dependantType.update(                                    \
        dependantMemberPtr == nullptr ? nullptr : &dependantMemberPtr->m_sfPrivLifetimeDependee##dependeeType)
// NOLINTEND(bugprone-macro-parentheses)

#else // SFML_ENABLE_LIFETIME_TRACKING

#define SFML_DEFINE_LIFETIME_DEPENDANT(dependantType) using sfPrivSwallowSemicolon##dependantType = void

#define SFML_UPDATE_LIFETIME_DEPENDANT(...) (void)0

#endif // SFML_ENABLE_LIFETIME_TRACKING
