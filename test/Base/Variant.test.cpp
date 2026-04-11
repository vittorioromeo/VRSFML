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
}
