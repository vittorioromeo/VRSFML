#include "StringifyVectorUtil.hpp"   // IWYU pragma: keep
#include "StringifyZbStringUtil.hpp" // IWYU pragma: keep
#include "Tst/Tst.hpp"

#include "Zancle/Algorithm/Sort.hpp"

#include "Zancle/String/String.hpp"

#include "Zancle/Algorithm/IsSorted.hpp"

#include "Zancle/Container/Vector.hpp"


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

        za::insertionSort(values, values + 4, lessCmp);

        CHECK(values[0] == 0);
        CHECK(values[1] == 1);
        CHECK(values[2] == 2);
        CHECK(values[3] == 3);
    }

    SECTION("Quick Sort")
    {
        int values[]{3, 2, 1, 0};

        za::quickSort(values, values + 4, lessCmp);

        CHECK(values[0] == 0);
        CHECK(values[1] == 1);
        CHECK(values[2] == 2);
        CHECK(values[3] == 3);
    }
}

struct Person
{
    za::String name;
    int        age;

    bool operator<(const Person& other) const
    {
        return name < other.name;
    }
};

TEST_CASE("QuickSort (Default Comparator): Core functionality")
{
    SUBCASE("Sorting a general vector of integers")
    {
        za::Vector<int> vec      = {9, 0, 2, 7, 5, 3, 8, 1, 6, 4};
        za::Vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        za::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting a vector with duplicate elements")
    {
        za::Vector<int> vec      = {5, 2, 8, 2, 9, 5, 8, 1, 5};
        za::Vector<int> expected = {1, 2, 2, 5, 5, 5, 8, 8, 9};
        za::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == expected);
    }
}

TEST_CASE("QuickSort (Default Comparator): Edge cases")
{
    SUBCASE("Sorting an empty vector")
    {
        za::Vector<int> vec;
        za::quickSort(vec.begin(), vec.end());
        CHECK(vec.empty());
    }

    SUBCASE("Sorting a single-element vector")
    {
        za::Vector<int> vec = {42};
        za::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == za::Vector<int>{42});
    }

    SUBCASE("Sorting an already sorted vector")
    {
        za::Vector<int> vec = {1, 2, 3, 4, 5};
        za::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == za::Vector<int>{1, 2, 3, 4, 5});
    }

    SUBCASE("Sorting a reverse-sorted vector")
    {
        za::Vector<int> vec = {10, 9, 8, 7, 6};
        za::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == za::Vector<int>{6, 7, 8, 9, 10});
    }

    SUBCASE("Sorting a vector with all elements identical")
    {
        za::Vector<int> vec = {7, 7, 7, 7, 7};
        za::quickSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == za::Vector<int>{7, 7, 7, 7, 7});
    }
}

TEST_CASE("QuickSort (Custom Comparator): Functionality")
{
    SUBCASE("Sorting integers in descending order using std::greater")
    {
        za::Vector<int> vec      = {3, 1, 4, 1, 5, 9};
        za::Vector<int> expected = {9, 5, 4, 3, 1, 1};
        za::quickSort(vec.begin(), vec.end(), greaterCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting a custom struct by a member using a lambda")
    {
        za::Vector<Person> people = {{"Charlie", 35}, {"Alice", 30}, {"Bob", 25}};

        auto compareByAge = [](const Person& a, const Person& b) { return a.age < b.age; };

        za::quickSort(people.begin(), people.end(), compareByAge);
        CHECK(za::isSorted(people.begin(), people.end(), compareByAge));

        CHECK(people[0].name == "Bob");
        CHECK(people[1].name == "Alice");
        CHECK(people[2].name == "Charlie");
    }

    SUBCASE("Sorting a custom struct using its default operator<")
    {
        za::Vector<Person> people = {{"Charlie", 35}, {"Alice", 30}, {"Bob", 25}};

        za::quickSort(people.begin(), people.end(), lessCmp);

        CHECK(people[0].name == "Alice");
        CHECK(people[1].name == "Bob");
        CHECK(people[2].name == "Charlie");
    }
}

TEST_CASE("InsertionSort (with Comparator): Functionality")
{
    SUBCASE("Sorting a small, unsorted array ascending")
    {
        za::Vector<int> vec      = {5, 1, 4, 2, 8};
        za::Vector<int> expected = {1, 2, 4, 5, 8};
        za::insertionSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting a small, unsorted array descending")
    {
        za::Vector<int> vec      = {5, 1, 4, 2, 8};
        za::Vector<int> expected = {8, 5, 4, 2, 1};
        za::insertionSort(vec.begin(), vec.end(), greaterCmp);
        CHECK(vec == expected);
    }

    SUBCASE("Sorting an empty range")
    {
        za::Vector<int> vec;
        za::insertionSort(vec.begin(), vec.end(), lessCmp);
        CHECK(vec.empty());
    }
}
