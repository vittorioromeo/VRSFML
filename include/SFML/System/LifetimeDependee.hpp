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

class SFML_SYSTEM_API LifetimeDependee
{
public:
    struct SFML_SYSTEM_API TestingModeGuard
    {
        [[nodiscard]] TestingModeGuard();
        ~TestingModeGuard();

        TestingModeGuard(const TestingModeGuard&) = delete;
        TestingModeGuard(TestingModeGuard&&)      = delete;

        TestingModeGuard& operator=(const TestingModeGuard&) = delete;
        TestingModeGuard& operator=(TestingModeGuard&&)      = delete;

        [[nodiscard]] static bool fatalErrorTriggered();
    };

    explicit LifetimeDependee(const char* dependeeName, const char* dependantName);
    ~LifetimeDependee();

    LifetimeDependee(const LifetimeDependee& rhs);
    LifetimeDependee(LifetimeDependee&& rhs) noexcept;

    LifetimeDependee& operator=(const LifetimeDependee& rhs);
    LifetimeDependee& operator=(LifetimeDependee&& rhs) noexcept;

    void addDependant();
    void subDependant();

private:
    const char* m_dependeeName;                                        ///< Readable dependee type name
    const char* m_dependantName;                                       ///< Readable dependent type name
    alignas(unsigned int) char m_dependantCount[sizeof(unsigned int)]; ///< Current alive dependant count
};

} // namespace sf::priv

// NOLINTBEGIN(bugprone-macro-parentheses)
#define SFML_DEFINE_LIFETIME_DEPENDEE(dependeeType, dependantType)                                               \
    friend dependantType;                                                                                        \
    mutable ::sf::priv::LifetimeDependee m_sfPrivLifetimeDependee##dependantType{#dependeeType, #dependantType}; \
    using sfPrivSwallowSemicolon##dependantType##dependeeType = void
// NOLINTEND(bugprone-macro-parentheses)

#else // SFML_ENABLE_LIFETIME_TRACKING

#define SFML_DEFINE_LIFETIME_DEPENDEE(dependantType, dependeeType) \
    using sfPrivSwallowSemicolon##dependantType##dependeeType = void

#endif // SFML_ENABLE_LIFETIME_TRACKING