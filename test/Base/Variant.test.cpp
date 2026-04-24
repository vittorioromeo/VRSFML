#include "SFML/Base/Variant.hpp"

#include "SFML/Base/Trait/IsAggregate.hpp"
#include "SFML/Base/Trait/IsStandardLayout.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"

#include <Doctest.hpp>


namespace
{
namespace VariantTest // for unity builds
{
////////////////////////////////////////////////////////////
struct NonTrivial
{
    static inline int si{};

    NonTrivial(const NonTrivial&) : i(si)
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    ~NonTrivial()
    {
    }

    int& i; // NOLINT(cppcoreguidelines-use-default-member-init, modernize-use-default-member-init)
};


////////////////////////////////////////////////////////////
struct NonTrivialButRelocatable
{
    enum : bool
    {
        enableTrivialRelocation = true
    };

    static inline int si{};

    NonTrivialButRelocatable(const NonTrivialButRelocatable&) : i(si)
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    ~NonTrivialButRelocatable()
    {
    }

    int& i; // NOLINT(cppcoreguidelines-use-default-member-init, modernize-use-default-member-init)
};


////////////////////////////////////////////////////////////
// Counts special-member invocations so tests can assert which path
// the variant's assignment operators took (destroy+construct vs.
// same-alternative assignment fast path).
struct Tracker
{
    static inline int copyCtor   = 0;
    static inline int moveCtor   = 0;
    static inline int copyAssign = 0;
    static inline int moveAssign = 0;
    static inline int dtor       = 0;

    int tag = 0;

    Tracker() noexcept = default;
    explicit Tracker(int t) noexcept : tag(t)
    {
    }
    Tracker(const Tracker& other) noexcept : tag(other.tag)
    {
        ++copyCtor;
    }
    Tracker(Tracker&& other) noexcept : tag(other.tag)
    {
        ++moveCtor;
    }
    Tracker& operator=(const Tracker& other) noexcept
    {
        tag = other.tag;
        ++copyAssign;
        return *this;
    }
    Tracker& operator=(Tracker&& other) noexcept
    {
        tag = other.tag;
        ++moveAssign;
        return *this;
    }
    ~Tracker() noexcept
    {
        ++dtor;
    }

    static void reset() noexcept
    {
        copyCtor = moveCtor = copyAssign = moveAssign = dtor = 0;
    }
};


////////////////////////////////////////////////////////////
struct OtherAlt
{
    int x = 0;
};

} // namespace VariantTest
} // namespace


TEST_CASE("[Base] Base/Variant.hpp")
{
    SECTION("Type traits")
    {
        using namespace VariantTest;

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Variant<int, char>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Variant<int, char>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Variant<int, char>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Variant<int, char>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Variant<int, char>));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Variant<int, char>));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::base::Variant<int, char>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Variant<int, char>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Variant<int, char>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Variant<int, char>, sf::base::Variant<int, char>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Variant<int, char>));


        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Variant<NonTrivial, char>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Variant<NonTrivial, char>));
        STATIC_CHECK(
            !SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Variant<NonTrivial, char>, sf::base::Variant<NonTrivial, char>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Variant<NonTrivial, char>));


        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Variant<NonTrivialButRelocatable, char>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Variant<NonTrivialButRelocatable, char>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Variant<NonTrivialButRelocatable, char>,
                                                        sf::base::Variant<NonTrivialButRelocatable, char>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Variant<NonTrivialButRelocatable, char>));
    }

    SECTION("Copy-assign fast path (same alternative)")
    {
        using namespace VariantTest;
        using V = sf::base::Variant<Tracker, OtherAlt>;

        V a{Tracker{1}};
        V b{Tracker{2}};

        Tracker::reset();
        a = b;

        CHECK(Tracker::copyAssign == 1);
        CHECK(Tracker::copyCtor == 0);
        CHECK(Tracker::dtor == 0);
        CHECK(a.is<Tracker>());
        CHECK(a.as<Tracker>().tag == 2);
    }

    SECTION("Move-assign fast path (same alternative)")
    {
        using namespace VariantTest;
        using V = sf::base::Variant<Tracker, OtherAlt>;

        V a{Tracker{1}};
        V b{Tracker{2}};

        Tracker::reset();
        a = static_cast<V&&>(b);

        CHECK(Tracker::moveAssign == 1);
        CHECK(Tracker::moveCtor == 0);
        CHECK(Tracker::dtor == 0);
        CHECK(a.is<Tracker>());
        CHECK(a.as<Tracker>().tag == 2);
    }

    SECTION("Copy-assign across different alternatives still destroys + constructs")
    {
        using namespace VariantTest;
        using V = sf::base::Variant<Tracker, OtherAlt>;

        V a{Tracker{1}};
        V b{OtherAlt{5}};

        Tracker::reset();
        a = b;

        CHECK(Tracker::dtor == 1);
        CHECK(Tracker::copyAssign == 0);
        CHECK(a.is<OtherAlt>());
        CHECK(a.as<OtherAlt>().x == 5);
    }

    SECTION("operator=(T&&) is aliasing-safe when x refers to the active alternative")
    {
        using namespace VariantTest;
        using V = sf::base::Variant<Tracker, OtherAlt>;

        V v{Tracker{42}};

        Tracker::reset();
        // `x` binds to the currently-held value. The same-alternative fast
        // path avoids destroying storage that `x` aliases.
        v = v.as<Tracker>();

        CHECK(v.is<Tracker>());
        CHECK(v.as<Tracker>().tag == 42);
        CHECK(Tracker::copyAssign == 1);
        CHECK(Tracker::dtor == 0);
    }

    SECTION("operator=(T&&) changes alternative on type mismatch")
    {
        using namespace VariantTest;
        using V = sf::base::Variant<Tracker, OtherAlt>;

        V v{Tracker{1}};

        Tracker::reset();
        v = OtherAlt{7};

        CHECK(Tracker::dtor == 1);
        CHECK(v.is<OtherAlt>());
        CHECK(v.as<OtherAlt>().x == 7);
    }
}
