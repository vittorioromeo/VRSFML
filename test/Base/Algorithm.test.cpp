#include "SFML/Base/Algorithm/AdjacentFind.hpp"
#include "SFML/Base/Algorithm/AllOf.hpp"
#include "SFML/Base/Algorithm/AnyOf.hpp"
#include "SFML/Base/Algorithm/Copy.hpp"
#include "SFML/Base/Algorithm/Count.hpp"
#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/Algorithm/IsSorted.hpp"
#include "SFML/Base/Algorithm/Remove.hpp"
#include "SFML/Base/Algorithm/SwapAndPop.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/Algorithm/*.hpp")
{
    SECTION("Copy")
    {
        const int values[]{0, 1, 2, 3};
        int       target[4];

        CHECK(sf::base::copy(values, values + 4, target) == target + 4);

        CHECK(target[0] == 0);
        CHECK(target[1] == 1);
        CHECK(target[2] == 2);
        CHECK(target[3] == 3);
    }

    SECTION("Find/FindIf/AnyOf")
    {
        const int values[]{0, 1, 2, 3, 4, 5, 6, 7};

        CHECK(sf::base::find(values, values + 8, 4) == &values[4]);
        CHECK(sf::base::find(values, values + 8, 5) == &values[5]);
        CHECK(sf::base::find(values, values + 8, 400) == values + 8);

        CHECK(sf::base::findIf(values, values + 8, [](const int x) { return x == 4; }) == &values[4]);
        CHECK(sf::base::findIf(values, values + 8, [](const int x) { return x == 5; }) == &values[5]);
        CHECK(sf::base::findIf(values, values + 8, [](const int x) { return x == 400; }) == values + 8);

        CHECK(sf::base::anyOf(values, values + 8, [](const int x) { return x == 4; }));
        CHECK(sf::base::anyOf(values, values + 8, [](const int x) { return x == 5; }));
        CHECK(!sf::base::anyOf(values, values + 8, [](const int x) { return x == 400; }));
    }

    SECTION("Count")
    {
        const bool bools[]{true, false, true, true, false};
        CHECK(sf::base::count(bools, bools + 5, true) == 3);

        const int ints[]{1, 0, 5, 0, -1, 0, 7};
        CHECK(sf::base::count(ints, ints + 7, 0) == 3);
    }

    SECTION("CountIf")
    {
        const int values[]{0, 1, 2, 3, 4, 5, 6, 7};

        const auto isEven      = [](int x) { return x % 2 == 0; };
        const auto isGt5       = [](int x) { return x > 5; };
        const auto alwaysFalse = [](int) { return false; };

        CHECK(sf::base::countIf(values, values + 8, isEven) == 4);
        CHECK(sf::base::countIf(values, values + 8, isGt5) == 2); // 6, 7
        CHECK(sf::base::countIf(values, values + 8, alwaysFalse) == 0);

        CHECK(sf::base::countIf(values, values + 0, isEven) == 0);
    }

    SECTION("AllOf")
    {
        const int values[]{0, 1, 2, 3, 4, 5, 6, 7};

        const auto isLt10     = [](int x) { return x < 10; };
        const auto isEven     = [](int x) { return x % 2 == 0; };
        const auto alwaysTrue = [](int) { return true; };

        CHECK(sf::base::allOf(values, values + 8, isLt10));
        CHECK(!sf::base::allOf(values, values + 8, isEven));

        const int evenValues[]{2, 4, 6, 8};
        CHECK(sf::base::allOf(evenValues, evenValues + 4, isEven));

        CHECK(sf::base::allOf(evenValues, evenValues + 0, isEven));     // Vacuously true
        CHECK(sf::base::allOf(evenValues, evenValues + 0, alwaysTrue)); // Vacuously true
    }


    SECTION("RemoveIf")
    {
        sf::base::Vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};

        const auto isEven = [](int x) { return x % 2 == 0; };

        auto* newEnd = sf::base::removeIf(v.begin(), v.end(), isEven);

        // Check the returned iterator position
        CHECK((newEnd == v.begin() + 4));

        // Check the elements that should remain are at the beginning
        CHECK(v[0] == 1);
        CHECK(v[1] == 3);
        CHECK(v[2] == 5);
        CHECK(v[3] == 7);
        // Elements from v[4] onwards are moved-from/unspecified but valid

        // Check with no elements to remove
        sf::base::Vector<int> vecOdd{1, 3, 5, 7};
        auto*                 newEndOdd = sf::base::removeIf(vecOdd.begin(), vecOdd.end(), isEven);
        CHECK((newEndOdd == vecOdd.end()));
        CHECK((vecOdd == sf::base::Vector<int>{1, 3, 5, 7}));

        // Check with all elements to remove
        sf::base::Vector<int> vecEven{2, 4, 6, 8};
        auto*                 newEndEven = sf::base::removeIf(vecEven.begin(), vecEven.end(), isEven);
        CHECK((newEndEven == vecEven.begin()));
    }

    SECTION("VectorEraseIf")
    {
        sf::base::Vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
        auto                  isEven = [](int x) { return x % 2 == 0; };

        sf::base::SizeT removedCount = sf::base::vectorEraseIf(v, isEven);

        CHECK(removedCount == 4);
        CHECK(v.size() == 4);
        CHECK((v == sf::base::Vector<int>{1, 3, 5, 7}));

        // Check with no elements removed
        removedCount = sf::base::vectorEraseIf(v, isEven);
        CHECK(removedCount == 0);
        CHECK(v.size() == 4);
        CHECK((v == sf::base::Vector<int>{1, 3, 5, 7}));

        // Check removing all elements
        auto isOdd   = [](int x) { return x % 2 != 0; };
        removedCount = sf::base::vectorEraseIf(v, isOdd);
        CHECK(removedCount == 4);
        CHECK(v.empty());

        // Check empty vector
        removedCount = sf::base::vectorEraseIf(v, isOdd);
        CHECK(removedCount == 0);
        CHECK(v.empty());
    }

    SECTION("IsSorted")
    {
        const auto less    = [](int a, int b) { return a < b; };
        const auto greater = [](int a, int b) { return a > b; };

        const int sorted[] = {1, 2, 2, 3, 4, 5};
        CHECK(sf::base::isSorted(sorted, sorted + 6, less));

        const int unsorted[] = {1, 3, 2, 4, 5};
        CHECK(!sf::base::isSorted(unsorted, unsorted + 5, less));

        const int reverseSorted[] = {5, 4, 3, 2, 1};
        CHECK(!sf::base::isSorted(reverseSorted, reverseSorted + 5, less));
        CHECK(sf::base::isSorted(reverseSorted, reverseSorted + 5, greater)); // Check with different comparer

        const int single[] = {10};
        CHECK(sf::base::isSorted(single, single + 1, less)); // Single element is sorted

        CHECK(sf::base::isSorted(unsorted, unsorted + 0, less)); // Empty range is sorted
    }

    SECTION("VectorSwapAndPopIf")
    {
        sf::base::Vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
        auto                  isEven = [](int x) { return x % 2 == 0; };

        sf::base::SizeT removedCount = sf::base::vectorSwapAndPopIf(v, isEven);

        CHECK(removedCount == 4);
        CHECK(v.size() == 4);
        CHECK((v == sf::base::Vector<int>{1, 5, 3, 7}));

        // Check with no elements removed
        removedCount = sf::base::vectorSwapAndPopIf(v, isEven);
        CHECK(removedCount == 0);
        CHECK(v.size() == 4);
        CHECK((v == sf::base::Vector<int>{1, 5, 3, 7}));

        // Check removing all elements
        auto isOdd   = [](int x) { return x % 2 != 0; };
        removedCount = sf::base::vectorSwapAndPopIf(v, isOdd);
        CHECK(removedCount == 4);
        CHECK(v.empty());

        // Check empty vector
        removedCount = sf::base::vectorSwapAndPopIf(v, isOdd);
        CHECK(removedCount == 0);
        CHECK(v.empty());
    }


    SECTION("AdjacentFind (Default): Core functionality")
    {
        SUBCASE("Pair found at the beginning")
        {
            int   vec[] = {5, 5, 2, 3, 4};
            auto* it    = sf::base::adjacentFind(vec, vec + 5);
            REQUIRE(it == vec);
            CHECK(*it == 5);
        }

        SUBCASE("Pair found in the middle")
        {
            int   vec[] = {1, 2, 8, 8, 3};
            auto* it    = sf::base::adjacentFind(vec, vec + 5);
            REQUIRE(it == vec + 2);
            CHECK(*it == 8);
        }

        SUBCASE("Pair found at the end")
        {
            int   vec[] = {1, 2, 3, 9, 9};
            auto* it    = sf::base::adjacentFind(vec, vec + 5);
            REQUIRE(it == vec + 3);
            CHECK(*it == 9);
        }

        SUBCASE("Multiple pairs exist, finds the first one")
        {
            int   vec[] = {1, 2, 2, 3, 4, 4};
            auto* it    = sf::base::adjacentFind(vec, vec + 6);
            REQUIRE(it == vec + 1);
            CHECK(*it == 2);
        }
    }

    SECTION("AdjacentFind (Default): Edge cases")
    {
        SUBCASE("No adjacent pair found")
        {
            int   vec[] = {1, 2, 3, 4, 5, 4, 3, 2, 1};
            auto* it    = sf::base::adjacentFind(vec, vec + 9);
            CHECK(it == vec + 9);
        }

        SUBCASE("Empty range")
        {
            int   vec[1]{};
            auto* it = sf::base::adjacentFind(vec, vec + 0);
            CHECK(it == vec + 0);
        }

        SUBCASE("Single element range")
        {
            int   vec[] = {100};
            auto* it    = sf::base::adjacentFind(vec, vec + 1);
            CHECK(it == vec + 1);
        }
    }

    SECTION("Adjacent find")
    {
        SUBCASE("Find where second element is greater than first")
        {
            int  vec[5]     = {5, 2, 3, 1, 8};
            auto greaterCmp = [](int a, int b) { return b > a; };

            auto* it = sf::base::adjacentFind(vec, vec + 5, greaterCmp);

            // The first pair where b > a is (2, 3)
            REQUIRE(it == vec + 1);
            CHECK(*it == 2);
            CHECK(*(it + 1) == 3);
        }

        SUBCASE("Predicate is never satisfied")
        {
            int  vec[5]     = {10, 8, 6, 4, 2};
            auto greaterCmp = [](int a, int b) { return b > a; };

            auto* it = sf::base::adjacentFind(vec, vec + 5, greaterCmp);
            CHECK(it == vec + 5);
        }
    }
}
