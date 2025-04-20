#include "SFML/Base/Swap.hpp"

#include <Doctest.hpp>

#include <vector>


TEST_CASE("[Base] Base/Swap.hpp")
{
    SECTION("Swap")
    {
        int a = 10;
        int b = 20;
        sf::base::swap(a, b);
        CHECK(a == 20);
        CHECK(b == 10);

        std::vector<int> v1 = {1, 2};
        std::vector<int> v2 = {3, 4, 5};
        sf::base::swap(v1, v2);
        CHECK((v1 == std::vector<int>{3, 4, 5}));
        CHECK((v2 == std::vector<int>{1, 2}));
    }

    SECTION("IterSwap")
    {
        int values[] = {10, 20, 30, 40};
        sf::base::iterSwap(values, values + 2); // Swap values[0] and values[2]
        CHECK(values[0] == 30);
        CHECK(values[1] == 20);
        CHECK(values[2] == 10);
        CHECK(values[3] == 40);

        std::vector<int> v = {5, 15, 25};
        sf::base::iterSwap(v.begin(), v.begin() + 1);
        CHECK(v[0] == 15);
        CHECK(v[1] == 5);
        CHECK(v[2] == 25);
    }

    SECTION("SwapRanges")
    {
        int arr1[] = {1, 2, 3, 4, 5};
        int arr2[] = {10, 20, 30, 40, 50};

        // Swap the first 3 elements
        auto* resultIter = sf::base::swapRanges(arr1, arr1 + 3, arr2);

        CHECK(resultIter == arr2 + 3);

        CHECK(arr1[0] == 10);
        CHECK(arr1[1] == 20);
        CHECK(arr1[2] == 30);
        CHECK(arr1[3] == 4); // Unchanged
        CHECK(arr1[4] == 5); // Unchanged

        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);
        CHECK(arr2[2] == 3);
        CHECK(arr2[3] == 40); // Unchanged
        CHECK(arr2[4] == 50); // Unchanged

        std::vector<int> v1 = {100, 200, 300, 400};
        std::vector<int> v2 = {500, 600, 700, 800};

        // Swap the middle 2 elements
        resultIter = sf::base::swapRanges(v1.begin() + 1, v1.begin() + 3, v2.begin() + 1).base();

        CHECK((resultIter == (v2.begin() + 3).base()));

        CHECK((v1 == std::vector<int>{100, 600, 700, 400}));
        CHECK((v2 == std::vector<int>{500, 200, 300, 800}));
    }
}
