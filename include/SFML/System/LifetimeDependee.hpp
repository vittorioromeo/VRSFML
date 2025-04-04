// TODO P0: figure out way of detecting compilation mismatch

#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#ifdef SFML_ENABLE_LIFETIME_TRACKING

    ////////////////////////////////////////////////////////////
    // Headers
    ////////////////////////////////////////////////////////////
    #include "SFML/System/Export.hpp"


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
