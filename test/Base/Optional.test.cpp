#include "SFML/Base/Optional.hpp"

#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsMoveAssignable.hpp"
#include "SFML/Base/Trait/IsMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"

#include <Doctest.hpp>


namespace
{
namespace OptionalTest // to support unity builds
{
struct Trivial
{
};

struct NonTrivial
{
    // NOLINTNEXTLINE(modernize-use-equals-default)
    NonTrivial()
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    ~NonTrivial()
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    NonTrivial(const NonTrivial&)
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    NonTrivial(NonTrivial&&) noexcept
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    NonTrivial& operator=(const NonTrivial&)
    {
        return *this;
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    NonTrivial& operator=(NonTrivial&&) noexcept
    {
        return *this;
    }
};

struct MoveOnly
{
    MoveOnly()  = default;
    ~MoveOnly() = default;

    MoveOnly(const MoveOnly&)            = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    MoveOnly(MoveOnly&&) noexcept            = default;
    MoveOnly& operator=(MoveOnly&&) noexcept = default;
};


TEST_CASE("[Base] Base/Optional.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIAL(Trivial));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(NonTrivial));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Optional<Trivial>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Optional<NonTrivial>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Optional<Trivial>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Optional<NonTrivial>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Optional<Trivial>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Optional<NonTrivial>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Optional<Trivial>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Optional<NonTrivial>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Optional<Trivial>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Optional<NonTrivial>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Optional<Trivial>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Optional<NonTrivial>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Optional<Trivial>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Optional<NonTrivial>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(MoveOnly));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(MoveOnly));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(MoveOnly));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(MoveOnly));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(MoveOnly));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(MoveOnly));

// Clang bug, see https://stackoverflow.com/questions/78885178
#ifndef __clang__
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Optional<MoveOnly>));
#endif
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::base::Optional<MoveOnly>));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::base::Optional<MoveOnly>));
    }
}

} // namespace OptionalTest
} // namespace
