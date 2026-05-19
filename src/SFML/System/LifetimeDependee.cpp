// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

#ifdef SFML_ENABLE_LIFETIME_TRACKING

    ////////////////////////////////////////////////////////////
    // Headers
    ////////////////////////////////////////////////////////////
    #include "SFML/System/LifetimeDependee.hpp"

    #include "SFML/System/Atomic.hpp"
    #include "SFML/System/Err.hpp"

    #include "SFML/Base/Abort.hpp"
    #include "SFML/Base/Assert.hpp"
    #include "SFML/Base/Builtin/Strcmp.hpp"
    #include "SFML/Base/Launder.hpp"
    #include "SFML/Base/PlacementNew.hpp"
    #include "SFML/Base/StackTrace.hpp"
    #include "SFML/Base/String.hpp"

    #include <cctype>


using AtomicUInt = sf::Atomic<unsigned int>;

static_assert(sizeof(AtomicUInt) == sizeof(unsigned int));
static_assert(alignof(AtomicUInt) == alignof(unsigned int));

namespace
{
constinit sf::Atomic<bool>        lifetimeTrackingTestingMode{false};
constinit sf::Atomic<bool>        lifetimeTrackingFatalErrorTriggered{false};
constinit sf::Atomic<const char*> lifetimeTrackingTestingDependeeName{nullptr};

[[gnu::always_inline, gnu::const]] inline AtomicUInt& asAtomicUInt(char* ptr)
{
    return *SFML_BASE_LAUNDER_CAST(AtomicUInt*, ptr);
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
LifetimeDependee::TestingModeGuard::TestingModeGuard(const char* const dependeeName)
{
    lifetimeTrackingTestingMode.storeSeqCst(true);
    lifetimeTrackingTestingDependeeName.storeSeqCst(dependeeName);
}


////////////////////////////////////////////////////////////
LifetimeDependee::TestingModeGuard::~TestingModeGuard()
{
    lifetimeTrackingTestingMode.storeSeqCst(false);
    lifetimeTrackingFatalErrorTriggered.storeSeqCst(false);
    lifetimeTrackingTestingDependeeName.storeSeqCst(nullptr);
}


////////////////////////////////////////////////////////////
bool LifetimeDependee::TestingModeGuard::fatalErrorTriggered(const char* const dependeeName)
{
    return lifetimeTrackingFatalErrorTriggered.loadSeqCst() &&
           SFML_BASE_STRCMP(lifetimeTrackingTestingDependeeName.loadSeqCst(), dependeeName) == 0;
}


////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
LifetimeDependee::LifetimeDependee(const char* dependeeName, const char* dependantName) :
    m_dependeeName(dependeeName),
    m_dependantName(dependantName)
{
    SFML_BASE_PLACEMENT_NEW(m_dependantCount) AtomicUInt(0u);
}


////////////////////////////////////////////////////////////
// A deep copy of a resource implies that lifetime tracking must being from scratch for that new copy.
LifetimeDependee::LifetimeDependee(const LifetimeDependee& rhs) :
    LifetimeDependee(rhs.m_dependeeName, rhs.m_dependantName)
{
}


////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
LifetimeDependee::LifetimeDependee(LifetimeDependee&& rhs) noexcept :
    LifetimeDependee(rhs.m_dependeeName, rhs.m_dependantName)
{
    // Intentionally not resetting `rhs.m_dependantCount` here, as we want to get a fatal error
    // if it wasn't `0u` when the move occurred.

    // We delay the check until the destructor to give a chance to the user to adjust the pointers
    // after the move if they so desire.
}


////////////////////////////////////////////////////////////
LifetimeDependee& LifetimeDependee::operator=(const LifetimeDependee& rhs)
{
    if (&rhs == this)
        return *this;

    m_dependeeName  = rhs.m_dependeeName;
    m_dependantName = rhs.m_dependantName;

    return *this;
}


////////////////////////////////////////////////////////////
LifetimeDependee& LifetimeDependee::operator=(LifetimeDependee&& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    m_dependeeName  = rhs.m_dependeeName;
    m_dependantName = rhs.m_dependantName;

    return *this;
}


////////////////////////////////////////////////////////////
LifetimeDependee::~LifetimeDependee()
{
    const unsigned int finalCount = asAtomicUInt(m_dependantCount).loadRelaxed();
    asAtomicUInt(m_dependantCount).~AtomicUInt();

    if (finalCount == 0u)
        return;

    const auto toLowerStr = [](base::String s)
    {
        for (char& c : s)
            c = static_cast<char>(std::tolower(c));

        return s;
    };

    const auto toTildes = [](base::String s)
    {
        for (char& c : s)
            c = '~';

        s[s.size() - 1] = '\0';
        return s;
    };

    const auto dependeeNameLower  = toLowerStr(m_dependeeName);
    const auto dependantNameLower = toLowerStr(m_dependantName);

    if (lifetimeTrackingTestingMode.loadSeqCst())
    {
        sf::priv::
            errMsg("LIFETIME TEST GUARD ERROR: a {} object was destroyed while existing {} objects depended on it.",
                   dependeeNameLower,
                   dependantNameLower);

        lifetimeTrackingFatalErrorTriggered.storeSeqCst(true);
        return;
    }

    priv::errMsgMulti("FATAL ERROR: a {} object was destroyed while existing {} objects depended on it.\n\n",
                      dependeeNameLower,
                      dependantNameLower);

    priv::errMsgMulti(
        "Please ensure that every {} object outlives all of the {} objects associated with it, otherwise those {}s "
        "will try to access the memory of the destroyed {}, causing undefined behavior (e.g., crashes, segfaults, or "
        "unexpected run-time behavior).\n\n",
        dependeeNameLower,
        dependantNameLower,
        dependantNameLower,
        dependeeNameLower);

    priv::errMsgMulti(
        "One of the ways this issue can occur is when a {} object is created as a local variable in a function and "
        "passed to a {} object. When the function has finished executing, the local {} object will be destroyed, and "
        "the {} object associated with it will now be referring to invalid memory. Example:\n\n",
        dependeeNameLower,
        dependantNameLower,
        dependeeNameLower,
        dependantNameLower);

    // clang-format off
    priv::errMsgMulti("    sf::{} create{}()\n    {{\n        sf::{} {}(/* ... */);\n        sf::{} {}({}, /* ... */);\n        \n        return {};\n        //     ^{}\n        // ERROR: `{}` will be destroyed right after\n        //        `{}` is returned from the function!\n    }}\n\n", m_dependantName, m_dependantName, m_dependeeName, dependeeNameLower, m_dependantName, dependantNameLower, dependeeNameLower, dependantNameLower, toTildes(dependantNameLower), dependeeNameLower, dependantNameLower);
    // clang-format on

    priv::errMsgMulti(
        "Another possible cause of this error is storing both a {} and a {} together in a data structure (e.g., "
        "`class`, `struct`, container, pair, etc...), and then moving that data structure (i.e., returning it from a "
        "function, or using `std::move`) -- the internal references between the {} and {} will not be updated, "
        "resulting in the same lifetime issue.\n\n",
        dependeeNameLower,
        dependantNameLower,
        dependeeNameLower,
        dependantNameLower);

    priv::errMsgMulti(
        "In general, make sure that all your {} objects are destroyed *after* all the {} objects depending on them to "
        "avoid these sort of issues.",
        dependeeNameLower,
        dependantNameLower);

    priv::errMsg("{}", '\n');

    base::priv::printStackTrace();
    base::abort();
}


////////////////////////////////////////////////////////////
void LifetimeDependee::addDependant()
{
    asAtomicUInt(m_dependantCount).fetchAddRelaxed(1u);
}


////////////////////////////////////////////////////////////
void LifetimeDependee::subDependant()
{
    SFML_BASE_ASSERT(asAtomicUInt(m_dependantCount).loadRelaxed() > 0u);
    asAtomicUInt(m_dependantCount).fetchSubRelaxed(1u);
}

} // namespace sf::priv

#endif // SFML_ENABLE_LIFETIME_TRACKING
