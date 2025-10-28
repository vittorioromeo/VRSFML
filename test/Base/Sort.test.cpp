#include "SFML/Base/Algorithm/Sort.hpp"

#include "StringifyStringUtil.hpp" // used
#include "StringifyVectorUtil.hpp" // used

#include "SFML/Base/Algorithm/IsSorted.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>

#include <string>


namespace
{
const auto lessCmp    = [](const auto& a, const auto& b) { return a < b; };
const auto greaterCmp = [](const auto& a, const auto& b) { return a > b; };
} // namespace


TEST_CASE("[Base] Base/Sort.hpp")
{
    SECTION("Insertion Sort")
    {
        int values[]{3, 2, 1, 0};

        sf::base::insertionSort(values, values + 4, lessCmp);

        CHECK(values[0] == 0);
        CHECK(values[1] == 1);
        CHECK(values[2] == 2);
        CHECK(values[3] == 3);
    }

    SECTION("Quick Sort")
    {
        int values[]{3, 2, 1, 0};

        sf::base::quickSort(values, values + 4, lessCmp);

        CHECK(values[0] == 0);
        CHECK(values[1] == 1);
        CHECK(values[2] == 2);
        CHECK(values[3] == 3);
    }
}

struct Person
{
    std::string name;
    int         age;

    bool operator<(const Person& other) const
    {
        return name < other.name;
    }
};

TEST_CASE("QuickSort (Default Comparator): Core functionality")
{
    SUBCASE("Sorting a general vector of integers")
    {
        sf::base::Vector<int> vec      = {9, 0, 2, 7, 5, 3, 8, 1, 6, 4};
        sf::base::Vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        sf::base::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting a vector with duplicate elements")
    {
        sf::base::Vector<int> vec      = {5, 2, 8, 2, 9, 5, 8, 1, 5};
        sf::base::Vector<int> expected = {1, 2, 2, 5, 5, 5, 8, 8, 9};
        sf::base::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == expected);
    }
}

TEST_CASE("QuickSort (Default Comparator): Edge cases")
{
    SUBCASE("Sorting an empty vector")
    {
        sf::base::Vector<int> vec;
        sf::base::quickSort(vec.begin(), vec.end());
        CHECK(vec.empty());
    }

    SUBCASE("Sorting a single-element vector")
    {
        sf::base::Vector<int> vec = {42};
        sf::base::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == sf::base::Vector<int>{42});
    }

    SUBCASE("Sorting an already sorted vector")
    {
        sf::base::Vector<int> vec = {1, 2, 3, 4, 5};
        sf::base::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == sf::base::Vector<int>{1, 2, 3, 4, 5});
    }

    SUBCASE("Sorting a reverse-sorted vector")
    {
        sf::base::Vector<int> vec = {10, 9, 8, 7, 6};
        sf::base::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == sf::base::Vector<int>{6, 7, 8, 9, 10});
    }

    SUBCASE("Sorting a vector with all elements identical")
    {
        sf::base::Vector<int> vec = {7, 7, 7, 7, 7};
        sf::base::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == sf::base::Vector<int>{7, 7, 7, 7, 7});
    }
}

TEST_CASE("QuickSort (Custom Comparator): Functionality")
{
    SUBCASE("Sorting integers in descending order using std::greater")
    {
        sf::base::Vector<int> vec      = {3, 1, 4, 1, 5, 9};
        sf::base::Vector<int> expected = {9, 5, 4, 3, 1, 1};
        sf::base::quickSort(vec.begin(), vec.end(), greaterCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting a custom struct by a member using a lambda")
    {
        sf::base::Vector<Person> people = {{"Charlie", 35}, {"Alice", 30}, {"Bob", 25}};

        auto compareByAge = [](const Person& a, const Person& b) { return a.age < b.age; };

        sf::base::quickSort(people.begin(), people.end(), compareByAge);
        CHECK(sf::base::isSorted(people.begin(), people.end(), compareByAge));

        CHECK(people[0].name == "Bob");
        CHECK(people[1].name == "Alice");
        CHECK(people[2].name == "Charlie");
    }

    SUBCASE("Sorting a custom struct using its default operator<")
    {
        sf::base::Vector<Person> people = {{"Charlie", 35}, {"Alice", 30}, {"Bob", 25}};

        sf::base::quickSort(people.begin(), people.end(), lessCmp);

        CHECK(people[0].name == "Alice");
        CHECK(people[1].name == "Bob");
        CHECK(people[2].name == "Charlie");
    }
}

TEST_CASE("InsertionSort (with Comparator): Functionality")
{
    SUBCASE("Sorting a small, unsorted array ascending")
    {
        sf::base::Vector<int> vec      = {5, 1, 4, 2, 8};
        sf::base::Vector<int> expected = {1, 2, 4, 5, 8};
        sf::base::insertionSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting a small, unsorted array descending")
    {
        sf::base::Vector<int> vec      = {5, 1, 4, 2, 8};
        sf::base::Vector<int> expected = {8, 5, 4, 2, 1};
        sf::base::insertionSort(vec.begin(), vec.end(), greaterCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting an empty range")
    {
        sf::base::Vector<int> vec;
        sf::base::insertionSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec.empty());
    }
}
