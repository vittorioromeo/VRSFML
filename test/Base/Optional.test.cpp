#include "Tst/Tst.hpp"

#include "ZancleBase/Optional.hpp"

#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsMoveAssignable.hpp"
#include "ZancleBase/Trait/IsMoveConstructible.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyRelocatable.hpp"


namespace
{
namespace OptionalTest // to support unity builds
{
////////////////////////////////////////////////////////////
struct Trivial
{
};


////////////////////////////////////////////////////////////
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
        STATIC_CHECK(ZB_IS_TRIVIAL(Trivial));
        STATIC_CHECK(!ZB_IS_TRIVIAL(NonTrivial));

        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::Optional<NonTrivial>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPYABLE(zb::Optional<NonTrivial>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Optional<NonTrivial>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Optional<NonTrivial>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Optional<NonTrivial>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Optional<NonTrivial>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Optional<NonTrivial>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::Optional<Trivial>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_RELOCATABLE(zb::Optional<NonTrivial>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::Optional<NonTrivialButRelocatable>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(MoveOnly));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(MoveOnly));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(MoveOnly));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(MoveOnly));
        STATIC_CHECK(ZB_IS_MOVE_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(ZB_IS_MOVE_ASSIGNABLE(MoveOnly));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(MoveOnly));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(MoveOnly));

// Clang bug, see https://stackoverflow.com/questions/78885178
#ifndef __clang__
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(zb::Optional<MoveOnly>));
#endif
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(ZB_IS_MOVE_CONSTRUCTIBLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(ZB_IS_MOVE_ASSIGNABLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(zb::Optional<MoveOnly>));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(zb::Optional<MoveOnly>));
    }
}

} // namespace OptionalTest
} // namespace
