#include "Tst/Tst.hpp"

#include "Zancle/System/GlobalAnchorPointMixin.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr za::Rect2f testRect{{53.f, 88.f}, {512.f, 5839.f}};


////////////////////////////////////////////////////////////
struct TestLayoutObject : za::GlobalAnchorPointMixin
{
    constexpr TestLayoutObject() = default;

    [[nodiscard]] constexpr za::Rect2f getLocalBounds() const
    {
        return {{0.f, 0.f}, {512.f, 5839.f}};
    }

    [[nodiscard]] constexpr za::Rect2f getGlobalBounds() const
    {
        const auto localBounds = getLocalBounds();
        return {position + localBounds.position, localBounds.size};
    }

    za::Vec2f position{42.f, 55.f};
};


////////////////////////////////////////////////////////////
[[nodiscard]] consteval bool doSetAnchorPointTest(za::Vec2f factors)
{
    constexpr za::Vec2f newPos{24.f, 24.f};

    TestLayoutObject testObject;
    testObject.setGlobalAnchorPoint(factors, newPos);
    return testObject.position == newPos - za::Vec2f{testRect.size.x * factors.x, testRect.size.y * factors.y};
}

TEST_CASE("[System] za::GlobalAnchorPointMixin")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::GlobalAnchorPointMixin));

        STATIC_CHECK(ZB_IS_TRIVIAL(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_AGGREGATE(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::GlobalAnchorPointMixin));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(za::GlobalAnchorPointMixin, za::GlobalAnchorPointMixin));
    }

    SECTION("getAnchorPoint")
    {
        constexpr TestLayoutObject testObject;

        STATIC_CHECK(testObject.getGlobalAnchorPoint({0.f, 0.f}) == testObject.getGlobalTopLeft());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({0.5f, 0.f}) == testObject.getGlobalTopCenter());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({1.f, 0.f}) == testObject.getGlobalTopRight());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({0.f, 0.5f}) == testObject.getGlobalCenterLeft());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({0.5f, 0.5f}) == testObject.getGlobalCenter());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({1.f, 0.5f}) == testObject.getGlobalCenterRight());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({0.f, 1.f}) == testObject.getGlobalBottomLeft());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({0.5f, 1.f}) == testObject.getGlobalBottomCenter());
        STATIC_CHECK(testObject.getGlobalAnchorPoint({1.f, 1.f}) == testObject.getGlobalBottomRight());
    }

    SECTION("setAnchorPoint")
    {
        STATIC_CHECK(doSetAnchorPointTest({0.f, 0.f}));
        STATIC_CHECK(doSetAnchorPointTest({0.5f, 0.f}));
        STATIC_CHECK(doSetAnchorPointTest({1.f, 0.f}));
        STATIC_CHECK(doSetAnchorPointTest({0.f, 0.5f}));
        STATIC_CHECK(doSetAnchorPointTest({0.5f, 0.5f}));
        STATIC_CHECK(doSetAnchorPointTest({1.f, 0.5f}));
        STATIC_CHECK(doSetAnchorPointTest({0.f, 1.f}));
        STATIC_CHECK(doSetAnchorPointTest({0.5f, 1.f}));
        STATIC_CHECK(doSetAnchorPointTest({1.f, 1.f}));
    }
}

} // namespace
