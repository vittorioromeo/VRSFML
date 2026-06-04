#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

#ifdef ZA_ENABLE_LIFETIME_TRACKING

    ////////////////////////////////////////////////////////////
    // Headers
    ////////////////////////////////////////////////////////////
    #include "Zancle/System/Export.hpp"


namespace za::priv
{
class LifetimeDependee;

class ZA_SYSTEM_API LifetimeDependant
{
public:
    explicit LifetimeDependant(const char* dependeeName, LifetimeDependee* dependee = nullptr) noexcept;
    ~LifetimeDependant();

    LifetimeDependant(const LifetimeDependant& rhs) noexcept;
    LifetimeDependant(LifetimeDependant&& rhs) noexcept;

    LifetimeDependant& operator=(const LifetimeDependant& rhs) noexcept;
    LifetimeDependant& operator=(LifetimeDependant&& rhs) noexcept;

    void update(LifetimeDependee* dependee) noexcept;

    [[nodiscard]] bool isTestingModeErrorTriggered() const noexcept;

private:
    void addSelfAsDependant();
    void subSelfAsDependant();

    const char*       m_dependeeName;
    LifetimeDependee* m_dependee;
};

} // namespace za::priv

    // NOLINTBEGIN(bugprone-macro-parentheses)
    #define ZA_DEFINE_LIFETIME_DEPENDANT(dependantType)                              \
        mutable ::za::priv::LifetimeDependant m_sfPrivLifetimeDependant##dependantType \
        {                                                                              \
            #dependantType                                                             \
        }

    #define ZA_UPDATE_LIFETIME_DEPENDANT(dependantType, dependeeType, thisPtr, dependantMemberPtr) \
        thisPtr->m_sfPrivLifetimeDependant##dependantType.update(                                    \
            dependantMemberPtr == nullptr ? nullptr : &dependantMemberPtr->m_sfPrivLifetimeDependee##dependeeType)

    #define ZA_LIFETIME_DEPENDANT_RETURN_IF_TESTING_ERROR(dependeeType)              \
        do                                                                             \
        {                                                                              \
            if (m_sfPrivLifetimeDependant##dependeeType.isTestingModeErrorTriggered()) \
                return;                                                                \
        } while (false)
// NOLINTEND(bugprone-macro-parentheses)

#else // ZA_ENABLE_LIFETIME_TRACKING

    #define ZA_DEFINE_LIFETIME_DEPENDANT(dependantType) static_assert(true)

    #define ZA_UPDATE_LIFETIME_DEPENDANT(...) (void)0

    #define ZA_LIFETIME_DEPENDANT_RETURN_IF_TESTING_ERROR(dependeeType) (void)0

#endif // ZA_ENABLE_LIFETIME_TRACKING
