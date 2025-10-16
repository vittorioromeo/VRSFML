#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsMoveAssignable.hpp"
#include "SFML/Base/Trait/IsMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


namespace
{
namespace NonTrivialVectorTest // for unity builds
{
int defaultCtorCount = 0;
int intCtorCount     = 0;
int copyCtorCount    = 0;
int moveCtorCount    = 0;
int dtorCount        = 0;
int copyAssignCount  = 0;
int moveAssignCount  = 0;

void resetCounters()
{
    defaultCtorCount = 0;
    intCtorCount     = 0;
    copyCtorCount    = 0;
    moveCtorCount    = 0;
    dtorCount        = 0;
    copyAssignCount  = 0;
    moveAssignCount  = 0;
}

struct Obj
{
    int value = 0;

    Obj()
    {
        ++defaultCtorCount;
    }

    Obj(int x) : value(x)
    {
        ++intCtorCount;
    }

    Obj(const Obj& rhs) : value(rhs.value)
    {
        ++copyCtorCount;
    }

    Obj(Obj&& rhs) noexcept : value(rhs.value)
    {
        ++moveCtorCount;

        rhs.value = 0;
    }

    ~Obj()
    {
        ++dtorCount;
    }

    Obj& operator=(const Obj& rhs)
    {
        if (this == &rhs)
            return *this;

        value = rhs.value;

        ++copyAssignCount;
        return *this;
    }

    Obj& operator=(Obj&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        value     = rhs.value;
        rhs.value = 0;

        ++moveAssignCount;
        return *this;
    }
};

TEST_CASE("[Base] Base/Vector.hpp")
{
    const auto asConst = [](auto& x) -> const auto& { return x; };

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(Obj));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(Obj));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(Obj));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(Obj));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(Obj));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(Obj));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(Obj));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Vector<Obj>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Vector<Obj>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Vector<Obj>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Vector<Obj>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Vector<Obj>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Vector<Obj>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Vector<Obj>));

        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::base::Vector<Obj>));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::base::Vector<Obj>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::base::Vector<Obj>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::base::Vector<Obj>));
    }

    SECTION("Empty")
    {
#define DO_EMPTY_CHECKS(tv)                  \
    CHECK((tv).begin() == nullptr);          \
    CHECK((tv).end() == nullptr);            \
    CHECK((tv).data() == nullptr);           \
                                             \
    CHECK(asConst((tv)).begin() == nullptr); \
    CHECK(asConst((tv)).end() == nullptr);   \
    CHECK(asConst((tv)).data() == nullptr);  \
                                             \
    CHECK((tv).size() == 0u);                \
    CHECK((tv).empty());

        resetCounters();

        sf::base::Vector<Obj> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.clear();
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        sf::base::Vector<Obj> tv2 = tv;
        DO_EMPTY_CHECKS(tv2);
        CHECK(tv2.capacity() == 0u);

        sf::base::Vector<Obj> tv3 = SFML_BASE_MOVE(tv);
        DO_EMPTY_CHECKS(tv3);
        CHECK(tv3.capacity() == 0u);

        sf::base::Vector<Obj> tv4;
        tv4 = tv;
        DO_EMPTY_CHECKS(tv4);
        CHECK(tv4.capacity() == 0u);

        sf::base::Vector<Obj> tv5;
        tv5 = SFML_BASE_MOVE(tv4);
        DO_EMPTY_CHECKS(tv5);
        CHECK(tv5.capacity() == 0u);

        CHECK(defaultCtorCount == 0);
        CHECK(intCtorCount == 0);
        CHECK(copyCtorCount == 0);
        CHECK(moveCtorCount == 0);
        CHECK(dtorCount == 0);
        CHECK(copyAssignCount == 0);
        CHECK(moveAssignCount == 0);
    }

    SECTION("Non-empty")
    {
        resetCounters();

        {
            sf::base::Vector<Obj> tv;
            DO_EMPTY_CHECKS(tv);
            CHECK(tv.capacity() == 0u);

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 0);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);
            CHECK(dtorCount == 0);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            tv.reserve(1);
            CHECK(tv.data() != nullptr);
            CHECK(tv.begin() == tv.data());
            CHECK(tv.end() == tv.data() + tv.size());
            CHECK(tv.size() == 0u);
            CHECK(tv.capacity() == 1u);

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 0);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);
            CHECK(dtorCount == 0);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            tv.unsafeEmplaceBack(42);
            CHECK(tv.data() != nullptr);
            CHECK(tv.begin() == tv.data());
            CHECK(tv.end() == tv.data() + tv.size());
            CHECK(tv.size() == 1u);
            CHECK(tv.capacity() == 1u);
            CHECK(tv[0].value == 42);

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 1);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);
            CHECK(dtorCount == 0);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            tv.reserveMore(10);

            CHECK(tv.data() != nullptr);
            CHECK(tv.begin() == tv.data());
            CHECK(tv.end() == tv.data() + tv.size());
            CHECK(tv.size() == 1u);
            CHECK(tv.capacity() >= 11u);
            CHECK(tv[0].value == 42);

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 1);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 1);
            CHECK(dtorCount == 1);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            tv.resize(100);

            CHECK(defaultCtorCount == 99);
            CHECK(intCtorCount == 1);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 2);
            CHECK(dtorCount == 2);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            CHECK(tv.data() != nullptr);
            CHECK(tv.begin() == tv.data());
            CHECK(tv.end() == tv.data() + tv.size());
            CHECK(tv.size() == 100u);
            CHECK(tv.capacity() >= 100u);
            CHECK(tv[0].value == 42);

            for (sf::base::SizeT i = 1; i < 100; ++i)
                CHECK(tv[i].value == 0);
        }

        CHECK(defaultCtorCount == 99);
        CHECK(intCtorCount == 1);
        CHECK(copyCtorCount == 0);
        CHECK(moveCtorCount == 2);
        CHECK(dtorCount == 102);
        CHECK(copyAssignCount == 0);
        CHECK(moveAssignCount == 0);
    }

    SECTION("Shrink to fit")
    {
        resetCounters();

        {
            sf::base::Vector<Obj> tv;
            DO_EMPTY_CHECKS(tv);
            CHECK(tv.capacity() == 0u);

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 0);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);
            CHECK(dtorCount == 0);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            tv.reserve(255);

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 0);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);
            CHECK(dtorCount == 0);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            for (int i = 0; i < 100; ++i)
                tv.unsafeEmplaceBack(5);

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 100);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);
            CHECK(dtorCount == 0);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            CHECK(tv.size() == 100);
            CHECK(tv.capacity() > 100);

            tv.shrinkToFit();

            CHECK(defaultCtorCount == 0);
            CHECK(intCtorCount == 100);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 100);
            CHECK(dtorCount == 100);
            CHECK(copyAssignCount == 0);
            CHECK(moveAssignCount == 0);

            CHECK(tv.size() == 100);
            CHECK(tv.capacity() == 100);
        }

        CHECK(defaultCtorCount == 0);
        CHECK(intCtorCount == 100);
        CHECK(copyCtorCount == 0);
        CHECK(moveCtorCount == 100);
        CHECK(dtorCount == 200);
        CHECK(copyAssignCount == 0);
        CHECK(moveAssignCount == 0);
    }


    SECTION("erase")
    {
        resetCounters();
        sf::base::Vector<Obj> tv;

        // Populate the vector: 10, 20, 30, 40, 50
        tv.emplaceBack(10);
        tv.emplaceBack(20);
        tv.emplaceBack(30);
        tv.emplaceBack(40);
        tv.emplaceBack(50);
        REQUIRE(tv.size() == 5);

        // We expect 5 int constructions from emplaceBack creating temporaries,
        // and 5 move constructions into the vector (assuming some reallocations might occur).
        // Let's reset counters here to focus purely on erase operations.
        resetCounters();

        // --- Erase from the middle (element 30 at index 2) ---
        Obj* itRet = tv.erase(tv.begin() + 2);
        CHECK(tv.size() == 4);
        // Check return value points to the element after the erased one (40)
        REQUIRE(itRet != tv.end());
        CHECK(itRet->value == 40);
        CHECK(itRet == tv.begin() + 2); // 40 is now at index 2
        // Check element values: 10, 20, 40, 50
        CHECK(tv[0].value == 10);
        CHECK(tv[1].value == 20);
        CHECK(tv[2].value == 40);
        CHECK(tv[3].value == 50);
        // Check counters: Expect 2 move assignments (40->30's spot, 50->40's spot)
        // Expect 1 destructor call (for the element at the new end, originally 50)
        CHECK(moveAssignCount == 2);
        CHECK(dtorCount == 1);
        CHECK(defaultCtorCount == 0);
        CHECK(intCtorCount == 0);
        CHECK(copyCtorCount == 0);
        CHECK(moveCtorCount == 0);
        CHECK(copyAssignCount == 0);

        // --- Erase the first element (element 10 at index 0) ---
        resetCounters();
        itRet = tv.erase(tv.begin());
        CHECK(tv.size() == 3);
        // Check return value points to the element after the erased one (20)
        REQUIRE(itRet != tv.end());
        CHECK(itRet->value == 20);
        CHECK(itRet == tv.begin()); // 20 is now at index 0
        // Check element values: 20, 40, 50
        CHECK(tv[0].value == 20);
        CHECK(tv[1].value == 40);
        CHECK(tv[2].value == 50);
        // Check counters: Expect 3 move assignments (20->10, 40->20, 50->40)
        // Expect 1 destructor call (for the element at the new end, originally 50)
        CHECK(moveAssignCount == 3);
        CHECK(dtorCount == 1);
        CHECK(defaultCtorCount == 0);
        CHECK(intCtorCount == 0);
        CHECK(copyCtorCount == 0);
        CHECK(moveCtorCount == 0);
        CHECK(copyAssignCount == 0);


        // --- Erase the last element (element 50 at index 2) ---
        resetCounters();
        itRet = tv.erase(tv.end() - 1); // Equivalent to tv.begin() + 2
        CHECK(tv.size() == 2);
        // Check return value points to end() since last element was erased
        CHECK(itRet == tv.end());
        // Check element values: 20, 40
        CHECK(tv[0].value == 20);
        CHECK(tv[1].value == 40);
        // Check counters: Expect 0 move assignments (no shifting)
        // Expect 1 destructor call (for the erased element 50)
        CHECK(moveAssignCount == 0);
        CHECK(dtorCount == 1);
        CHECK(defaultCtorCount == 0);
        CHECK(intCtorCount == 0);
        CHECK(copyCtorCount == 0);
        CHECK(moveCtorCount == 0);
        CHECK(copyAssignCount == 0);


        // --- Erase remaining elements ---
        // Erase last (40)
        resetCounters();
        itRet = tv.erase(tv.end() - 1);
        CHECK(tv.size() == 1);
        CHECK(itRet == tv.end());
        CHECK(tv[0].value == 20);
        CHECK(moveAssignCount == 0);
        CHECK(dtorCount == 1);

        // Erase first/last (20)
        resetCounters();
        itRet = tv.erase(tv.begin());
        CHECK(tv.size() == 0);
        CHECK(itRet == tv.end()); // Erasing last element returns end()
        CHECK(tv.empty());
        CHECK(moveAssignCount == 0);
        CHECK(dtorCount == 1);


        // --- Edge case: Erase from single-element vector ---
        resetCounters();
        sf::base::Vector<Obj> tvSingle;
        tvSingle.emplaceBack(100); // int:1, move:0 (initially no alloc)
        REQUIRE(tvSingle.size() == 1);
        resetCounters();

        itRet = tvSingle.erase(tvSingle.begin());
        CHECK(tvSingle.empty());
        CHECK(itRet == tvSingle.end());
        CHECK(moveAssignCount == 0);
        CHECK(dtorCount == 1);
    }
}

} // namespace NonTrivialVectorTest
} // namespace
