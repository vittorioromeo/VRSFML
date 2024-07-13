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
