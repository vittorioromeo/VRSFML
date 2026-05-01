#include "SystemUtil.hpp"

#include "SFML/System/LifetimeDependant.hpp"
#include "SFML/System/LifetimeDependee.hpp"

#include "SFML/Base/Macros.hpp"

#include <Doctest.hpp>

#if defined(SFML_ENABLE_LIFETIME_TRACKING)

namespace
{
////////////////////////////////////////////////////////////
/// Dummy dependee (resource owner) and dependant (resource user)
/// for testing the lifetime tracking machinery in isolation.
////////////////////////////////////////////////////////////

struct DummyDependant;

struct DummyDependee
{
    int value{};

    SFML_DEFINE_LIFETIME_DEPENDEE(DummyDependee, DummyDependant);
};


struct DummyDependant
{
    const DummyDependee* dependee{};

    SFML_DEFINE_LIFETIME_DEPENDANT(DummyDependee);

    explicit DummyDependant(const DummyDependee& dep) : dependee(&dep)
    {
        SFML_UPDATE_LIFETIME_DEPENDANT(DummyDependee, DummyDependant, this, dependee);
    }

    ~DummyDependant()
    {
        SFML_LIFETIME_DEPENDANT_RETURN_IF_TESTING_ERROR(DummyDependee);
    }

    DummyDependant(const DummyDependant& rhs) : dependee(rhs.dependee)
    {
        SFML_UPDATE_LIFETIME_DEPENDANT(DummyDependee, DummyDependant, this, dependee);
    }

    DummyDependant& operator=(const DummyDependant& rhs)
    {
        if (&rhs == this)
            return *this;

        dependee = rhs.dependee;
        SFML_UPDATE_LIFETIME_DEPENDANT(DummyDependee, DummyDependant, this, dependee);
        return *this;
    }

    DummyDependant(DummyDependant&& rhs) noexcept : dependee(rhs.dependee)
    {
        SFML_UPDATE_LIFETIME_DEPENDANT(DummyDependee, DummyDependant, this, dependee);
    }

    DummyDependant& operator=(DummyDependant&& rhs) noexcept
    {
        dependee = rhs.dependee;
        SFML_UPDATE_LIFETIME_DEPENDANT(DummyDependee, DummyDependant, this, dependee);
        return *this;
    }
};

} // namespace


TEST_CASE("[System] Lifetime tracking")
{
    SECTION("Single dependant construction and destruction")
    {
        DummyDependee dep;
        {
            DummyDependant d(dep);
        }
        // dep destroyed with count == 0 -> OK
    }

    SECTION("Multiple dependants on same dependee")
    {
        DummyDependee dep;
        {
            DummyDependant a(dep);
            DummyDependant b(dep);
            DummyDependant c(dep);
        }
    }

    SECTION("Copy construction")
    {
        DummyDependee dep;
        {
            DummyDependant a(dep);
            DummyDependant b(a); // NOLINT(performance-unnecessary-copy-initialization)
            (void)b;
        }
    }

    SECTION("Copy assignment same dependee")
    {
        DummyDependee dep;
        {
            DummyDependant a(dep);
            DummyDependant b(dep);
            a = b;
        }
    }

    SECTION("Copy assignment different dependee")
    {
        DummyDependee dep1;
        DummyDependee dep2;
        {
            DummyDependant a(dep1);
            DummyDependant b(dep2);
            a = b;
            CHECK(a.dependee == &dep2);
        }
    }

    SECTION("Repeated copy assignment does not inflate count")
    {
        DummyDependee dep;
        {
            DummyDependant a(dep);
            DummyDependant b(dep);
            a = b;
            a = b;
            a = b;
            a = b;
            a = b;
        }
    }

    SECTION("Self copy assignment")
    {
        DummyDependee dep;
        {
            DummyDependant a(dep);
    #if defined(__clang__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wself-assign-overloaded"
    #endif
            a = a;
    #if defined(__clang__)
        #pragma clang diagnostic pop
    #endif
        }
    }

    SECTION("Move construction")
    {
        DummyDependee dep;
        {
            DummyDependant a(dep);
            DummyDependant b(SFML_BASE_MOVE(a));
            (void)b;
        }
    }

    SECTION("Move assignment same dependee")
    {
        DummyDependee dep;
        {
            DummyDependant a(dep);
            DummyDependant b(dep);
            a = SFML_BASE_MOVE(b);
        }
    }

    SECTION("Move assignment different dependee")
    {
        DummyDependee dep1;
        DummyDependee dep2;
        {
            DummyDependant a(dep1);
            DummyDependant b(dep2);
            a = SFML_BASE_MOVE(b);
            CHECK(a.dependee == &dep2);
        }
    }

    SECTION("Dependee destroyed before dependant triggers error")
    {
        const sf::priv::LifetimeDependee::TestingModeGuard guard{"DummyDependee"};
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));

        auto*          dep = new DummyDependee;
        DummyDependant d(*dep);

        delete dep;
        CHECK(guard.fatalErrorTriggered("DummyDependee"));
    }

    SECTION("Dependee destroyed with no dependants is fine")
    {
        const sf::priv::LifetimeDependee::TestingModeGuard guard{"DummyDependee"};
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));

        {
            DummyDependee dep;
            {
                DummyDependant d(dep);
            }
            // d destroyed, dep still alive -> count is 0
        }
        // dep destroyed with 0 dependants -> no error
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));
    }

    SECTION("Copy of dependant keeps dependee alive requirement")
    {
        const sf::priv::LifetimeDependee::TestingModeGuard guard{"DummyDependee"};
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));

        auto*          dep = new DummyDependee;
        DummyDependant a(*dep);
        DummyDependant b(a); // NOLINT(performance-unnecessary-copy-initialization)

        delete dep;
        CHECK(guard.fatalErrorTriggered("DummyDependee"));
    }

    SECTION("Switching dependee via copy assignment")
    {
        const sf::priv::LifetimeDependee::TestingModeGuard guard{"DummyDependee"};
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));

        DummyDependee dep1;
        DummyDependee dep2;
        {
            DummyDependant a(dep1);
            DummyDependant b(dep2);

            // a switches from dep1 to dep2
            a = b;

            // dep1 should now have 0 dependants, dep2 should have 2
        }
        // All dependants destroyed, both dependees at 0
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));
    }

    SECTION("Switching dependee via move assignment")
    {
        const sf::priv::LifetimeDependee::TestingModeGuard guard{"DummyDependee"};
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));

        DummyDependee dep1;
        DummyDependee dep2;
        {
            DummyDependant a(dep1);
            DummyDependant b(dep2);

            a = SFML_BASE_MOVE(b);
        }
        CHECK(!guard.fatalErrorTriggered("DummyDependee"));
    }
}

#endif // SFML_ENABLE_LIFETIME_TRACKING
