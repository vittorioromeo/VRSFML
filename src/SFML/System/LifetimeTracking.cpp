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

#include <SFML/System/Err.hpp>
#include <SFML/System/LifetimeTracking.hpp>

#include <atomic>
#include <exception>
#include <new>
#include <ostream>
#include <string>

#include <cassert>
#include <cctype>


using AtomicUInt = std::atomic<unsigned int>;

static_assert(sizeof(AtomicUInt) == sizeof(unsigned int));
static_assert(alignof(AtomicUInt) == alignof(unsigned int));

namespace
{
std::atomic<bool> lifetimeTrackingTestingModfe{false};
std::atomic<bool> lifetimeTrackingFatalErrorTriggered{false};

[[gnu::always_inline]] inline AtomicUInt& asAtomicUInt(char* ptr)
{
    return *std::launder(reinterpret_cast<AtomicUInt*>(ptr));
}

} // namespace

namespace sf::priv
{
////////////////////////////////////////////////////////////
LifetimeDependee::TestingModeGuard::TestingModeGuard()
{
    lifetimeTrackingTestingModfe.store(true, std::memory_order_seq_cst);
}


////////////////////////////////////////////////////////////
LifetimeDependee::TestingModeGuard::~TestingModeGuard()
{
    lifetimeTrackingTestingModfe.store(false, std::memory_order_seq_cst);
    lifetimeTrackingFatalErrorTriggered.store(false, std::memory_order_seq_cst);
}


////////////////////////////////////////////////////////////
bool LifetimeDependee::TestingModeGuard::fatalErrorTriggered()
{
    return lifetimeTrackingFatalErrorTriggered.load(std::memory_order_seq_cst);
}


////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
LifetimeDependee::LifetimeDependee(const char* dependeeName, const char* dependantName) :
m_dependeeName(dependeeName),
m_dependantName(dependantName)
{
    new (m_dependantCount) AtomicUInt(0u);
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
m_dependeeName(rhs.m_dependeeName),
m_dependantName(rhs.m_dependantName)
{
    new (m_dependantCount) AtomicUInt(asAtomicUInt(rhs.m_dependantCount).load(std::memory_order_relaxed));

    // Intentionally not resetting `rhs.m_dependantCount` here, as we want to get a fatal error
    // if it wasn't `0u` when the move occurred.

    // We delay the check until the destructor to give a chance to the user to adjust the pointers
    // after the move if they so desire.
}


////////////////////////////////////////////////////////////
// A deep copy of a resource implies that lifetime tracking must being from scratch for that new copy.
LifetimeDependee& LifetimeDependee::operator=(const LifetimeDependee& rhs)
{
    if (&rhs == this)
        return *this;

    m_dependeeName  = rhs.m_dependeeName;
    m_dependantName = rhs.m_dependantName;
    asAtomicUInt(m_dependantCount).store(0u, std::memory_order_relaxed);

    return *this;
}


////////////////////////////////////////////////////////////
LifetimeDependee& LifetimeDependee::operator=(LifetimeDependee&& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    m_dependeeName  = rhs.m_dependeeName;
    m_dependantName = rhs.m_dependantName;
    asAtomicUInt(m_dependantCount)
        .store(asAtomicUInt(rhs.m_dependantCount).load(std::memory_order_relaxed), std::memory_order_relaxed);

    // See rationale in move constructor for not resetting `rhs.m_dependantCount`.

    return *this;
}


////////////////////////////////////////////////////////////
LifetimeDependee::~LifetimeDependee()
{
    const unsigned int finalCount = asAtomicUInt(m_dependantCount).load(std::memory_order_relaxed);
    asAtomicUInt(m_dependantCount).~AtomicUInt();

    if (finalCount == 0u)
        return;

    if (lifetimeTrackingTestingModfe)
    {
        lifetimeTrackingFatalErrorTriggered = true;
        return;
    }

    const auto toLowerStr = [](std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(std::tolower(c));

        return s;
    };

    const auto toTildes = [](std::string s)
    {
        for (char& c : s)
            c = '~';

        s[s.size() - 1] = '\0';
        return s;
    };

    const auto dependeeNameLower  = toLowerStr(m_dependeeName);
    const auto dependantNameLower = toLowerStr(m_dependantName);

    priv::err() << "FATAL ERROR: a " << dependeeNameLower << " object was destroyed while existing "
                << dependantNameLower << " objects depended on it.\n\n";

    priv::err() << "Please ensure that every " << dependeeNameLower << " object outlives all of the "
                << dependantNameLower << " objects associated with it, otherwise those " << dependantNameLower
                << "s will try to access the memory of the destroyed " << dependeeNameLower
                << ", causing undefined behavior (e.g., crashes, segfaults, or unexpected run-time behavior).\n\n";

    priv::err() << "One of the ways this issue can occur is when a " << dependeeNameLower
                << " object is created as a local variable in a function and passed to a " << dependantNameLower
                << " object. When the function has finished executing, the local " << dependeeNameLower
                << " object will be destroyed, and the " << dependantNameLower
                << " object associated with it will now be referring to invalid memory. Example:\n\n";

    // clang-format off
    priv::err() << "    sf::" << m_dependantName << " create" << m_dependantName << "()\n"
          << "    {\n"
          << "        " << "sf::" << m_dependeeName << " " << dependeeNameLower << "(/* ... */);\n"
          << "        " << "sf::" << m_dependantName << " " << dependantNameLower << "(" << dependeeNameLower << ", /* ... */);\n"
          << "        " << "\n"
          << "        " << "return " << dependantNameLower << ";\n"
          << "        " << "//     ^" << toTildes(dependantNameLower) << "\n"
          << "        " << "// ERROR: `" << dependeeNameLower << "` will be destroyed right after\n"
          << "        " << "//        `" << dependantNameLower << "` is returned from the function!\n"
          << "    }\n\n";
    // clang-format on

    priv::err() << "Another possible cause of this error is storing both a " << dependeeNameLower << " and a "
                << dependantNameLower
                << " together in a data structure (e.g., `class`, `struct`, container, pair, etc...), and then moving "
                   "that "
                   "data structure (i.e., returning it from a function, or using `std::move`) -- the internal "
                   "references "
                   "between the "
                << dependeeNameLower << " and " << dependantNameLower
                << " will not be updated, resulting in the same lifetime issue.\n\n";

    priv::err() << "In general, make sure that all your " << dependeeNameLower << " objects are destroyed *after* all the "
                << dependantNameLower << " objects depending on them to avoid these sort of issues." << std::endl;

    std::terminate();
}


////////////////////////////////////////////////////////////
void LifetimeDependee::addDependant()
{
    asAtomicUInt(m_dependantCount).fetch_add(1u, std::memory_order_relaxed);
}


////////////////////////////////////////////////////////////
void LifetimeDependee::subDependant()
{
    assert(asAtomicUInt(m_dependantCount).load(std::memory_order_relaxed) > 0u);
    asAtomicUInt(m_dependantCount).fetch_sub(1u, std::memory_order_relaxed);
}


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
LifetimeDependant::LifetimeDependant(LifetimeDependant&& rhs) noexcept : LifetimeDependant(rhs.m_dependee)
{
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
    return (*this = rhs);
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
