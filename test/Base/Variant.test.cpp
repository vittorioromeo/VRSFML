#include "SFML/Base/Variant.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>


namespace
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

} // namespace


TEST_CASE("[Base] Base/Variant.hpp")
{
    SECTION("Type traits")
    {
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
