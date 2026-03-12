#include "SFML/System/GlobalAnchorPointMixin.hpp"

#include "SFML/System/Rect2.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Rect2f testRect{{53.f, 88.f}, {512.f, 5839.f}};


////////////////////////////////////////////////////////////
struct TestLayoutObject : sf::GlobalAnchorPointMixin<TestLayoutObject>
{
    constexpr TestLayoutObject() = default;

    [[nodiscard]] constexpr sf::Rect2f getLocalBounds() const
    {
        return {{0.f, 0.f}, {512.f, 5839.f}};
    }

    [[nodiscard]] constexpr sf::Rect2f getGlobalBounds() const
    {
        const auto localBounds = getLocalBounds();
        return {position + localBounds.position, localBounds.size};
    }

    sf::Vec2f position{42.f, 55.f};
};


////////////////////////////////////////////////////////////
[[nodiscard]] consteval bool doSetAnchorPointTest(sf::Vec2f factors)
{
    constexpr sf::Vec2f newPos{24.f, 24.f};

    TestLayoutObject testObject;
    testObject.setGlobalAnchorPoint(factors, newPos);
    return testObject.position == newPos - sf::Vec2f{testRect.size.x * factors.x, testRect.size.y * factors.y};
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::GlobalAnchorPointMixin")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::GlobalAnchorPointMixin<TestLayoutObject>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIAL(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::GlobalAnchorPointMixin<TestLayoutObject>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::GlobalAnchorPointMixin<TestLayoutObject>,
                                                       sf::GlobalAnchorPointMixin<TestLayoutObject>));
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
