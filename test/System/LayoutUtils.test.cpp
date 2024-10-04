#include "SFML/System/LayoutUtils.hpp"

#include "SFML/System/Rect.hpp"

#include <Doctest.hpp>


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::FloatRect testRect{{53.f, 88.f}, {512.f, 5839.f}};


////////////////////////////////////////////////////////////
struct TestLayoutObject
{
    constexpr TestLayoutObject() = default;

    [[nodiscard]] constexpr sf::FloatRect getLocalBounds() const
    {
        return {{0.f, 0.f}, {512.f, 5839.f}};
    }

    [[nodiscard]] constexpr sf::FloatRect getGlobalBounds() const
    {
        const auto localBounds = getLocalBounds();
        return {position + localBounds.position, localBounds.size};
    }

    sf::Vector2f position{42.f, 55.f};
};


////////////////////////////////////////////////////////////
consteval bool doSetAnchorPointTest(sf::Vector2f factors)
{
    constexpr sf::Vector2f newPos{24.f, 24.f};

    TestLayoutObject testObject;
    sf::LayoutUtils::setAnchorPoint(testObject, factors, newPos);
    return testObject.position == newPos - sf::Vector2f{testRect.size.x * factors.x, testRect.size.y * factors.y};
};


////////////////////////////////////////////////////////////
TEST_CASE("[System] sf::LayoutUtils::LayoutUtils")
{
    SECTION("getAnchorPoint")
    {
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {0.f, 0.f}) == sf::LayoutUtils::getTopLeft(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {0.5f, 0.f}) == sf::LayoutUtils::getTopCenter(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {1.f, 0.f}) == sf::LayoutUtils::getTopRight(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {0.f, 0.5f}) == sf::LayoutUtils::getCenterLeft(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {0.5f, 0.5f}) == sf::LayoutUtils::getCenter(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {1.f, 0.5f}) == sf::LayoutUtils::getCenterRight(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {0.f, 1.f}) == sf::LayoutUtils::getBottomLeft(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {0.5f, 1.f}) == sf::LayoutUtils::getBottomCenter(testRect));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testRect, {1.f, 1.f}) == sf::LayoutUtils::getBottomRight(testRect));
    }

    SECTION("getAnchorPoint")
    {
        constexpr TestLayoutObject testObject;

        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {0.f, 0.f}) == sf::LayoutUtils::getTopLeft(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {0.5f, 0.f}) == sf::LayoutUtils::getTopCenter(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {1.f, 0.f}) == sf::LayoutUtils::getTopRight(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {0.f, 0.5f}) == sf::LayoutUtils::getCenterLeft(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {0.5f, 0.5f}) == sf::LayoutUtils::getCenter(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {1.f, 0.5f}) ==
                     sf::LayoutUtils::getCenterRight(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {0.f, 1.f}) == sf::LayoutUtils::getBottomLeft(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {0.5f, 1.f}) ==
                     sf::LayoutUtils::getBottomCenter(testObject));
        STATIC_CHECK(sf::LayoutUtils::getAnchorPoint(testObject, {1.f, 1.f}) == sf::LayoutUtils::getBottomRight(testObject));
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
