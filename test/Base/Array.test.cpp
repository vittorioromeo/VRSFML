#include "SFML/Base/Array.hpp"

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
namespace ArrayTest // for unity builds
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

} // namespace ArrayTest
} // namespace


TEST_CASE("[Base] Base/Array.hpp")
{
    SECTION("Type traits")
    {
        using namespace ArrayTest;

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Array<int, 5>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Array<int, 5>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Array<int, 5>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Array<int, 5>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIAL(sf::base::Array<int, 5>)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Array<int, 5>));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::base::Array<int, 5>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Array<int, 5>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Array<int, 5>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Array<int, 5>, sf::base::Array<int, 5>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Array<int, 5>));


        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Array<NonTrivial, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Array<NonTrivial, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Array<NonTrivial, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Array<NonTrivial, 5>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Array<NonTrivial, 5>)); // because of member initializers
        STATIC_CHECK(!SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Array<NonTrivial, 5>));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::base::Array<NonTrivial, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Array<NonTrivial, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Array<NonTrivial, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Array<NonTrivial, 5>, sf::base::Array<NonTrivial, 5>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Array<NonTrivial, 5>));


        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Array<NonTrivialButRelocatable, 5>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Array<NonTrivialButRelocatable, 5>)); // because of member initializers
        STATIC_CHECK(!SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::base::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Array<NonTrivialButRelocatable, 5>,
                                                        sf::base::Array<NonTrivialButRelocatable, 5>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Array<NonTrivialButRelocatable, 5>));
    }
}
