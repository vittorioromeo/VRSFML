#include "SFML/Base/ChunkedVector.hpp"

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
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"

#include <Doctest.hpp>


namespace
{
namespace ChunkedVectorTest // for unity builds
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

    [[nodiscard]] bool operator==(const Obj& rhs) const
    {
        return value == rhs.value;
    }

    [[nodiscard]] bool operator!=(const Obj& rhs) const
    {
        return value != rhs.value;
    }
};

TEST_CASE("[Base] Base/ChunkedVector.hpp")
{
    const auto asConst = [](auto& x) -> const auto& { return x; };

    SECTION("Type traits")
    {
        using T = sf::base::ChunkedVector<int, 2>;

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(T));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(T));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(T));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(T));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(T));

        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(T));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(T));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(T));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(T));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(T));
    }

    SECTION("Empty")
    {
#define DO_EMPTY_CHECKS_CV(tv)                                   \
    CHECK_UNARY(((tv).begin() == (tv).end()));                   \
    CHECK_UNARY((asConst((tv)).begin() == asConst((tv)).end())); \
    CHECK_UNARY(((tv).cbegin() == (tv).cend()));                 \
    CHECK((tv).size() == 0u);                                    \
    CHECK((tv).capacity() == 0u);                                \
    CHECK((tv).empty());

        sf::base::ChunkedVector<int, 2> tv;
        DO_EMPTY_CHECKS_CV(tv);

        tv.clear();
        DO_EMPTY_CHECKS_CV(tv);

        sf::base::ChunkedVector<int, 2> tv2 = tv;
        DO_EMPTY_CHECKS_CV(tv2);

        sf::base::ChunkedVector<int, 2> tv3 = SFML_BASE_MOVE(tv);
        DO_EMPTY_CHECKS_CV(tv3);

        sf::base::ChunkedVector<int, 2> tv4;
        tv4 = tv2;
        DO_EMPTY_CHECKS_CV(tv4);

        sf::base::ChunkedVector<int, 2> tv5;
        tv5 = SFML_BASE_MOVE(tv4);
        DO_EMPTY_CHECKS_CV(tv5);
    }

    SECTION("Constructors and equality")
    {
        const int src[] = {1, 2, 3, 4, 5};

        const sf::base::ChunkedVector<int, 2> fromRange(src, src + 5);
        const sf::base::ChunkedVector<int, 2> fromList{1, 2, 3, 4, 5};
        const sf::base::ChunkedVector<int, 2> filled(5, 7);

        CHECK(fromRange.size() == 5u);
        CHECK(fromRange == fromList);
        CHECK(fromRange != filled);
        CHECK(filled.size() == 5u);

        for (sf::base::SizeT i = 0u; i < 5u; ++i)
            CHECK(filled[i] == 7);
    }

    SECTION("Push back, reserve, pointer stability, and shrinkToFit")
    {
        sf::base::ChunkedVector<int, 2> vec;

        for (int i = 0; i < 10; ++i)
            vec.pushBack(i);

        CHECK(vec.size() == 10u);
        CHECK(vec.capacity() == 12u);
        CHECK(vec.front() == 0);
        CHECK(vec.back() == 9);

        for (sf::base::SizeT i = 0u; i < vec.size(); ++i)
            CHECK(vec[i] == static_cast<int>(i));

        int* const stablePtr = &vec[3];
        vec.reserve(19u);

        CHECK(vec.capacity() == 20u);
        CHECK(stablePtr == &vec[3]);
        CHECK(*stablePtr == 3);

        vec.resize(5u);
        CHECK(vec.size() == 5u);
        CHECK(vec.capacity() == 20u);
        CHECK(vec.back() == 4);

        vec.shrinkToFit();
        CHECK(vec.capacity() == 8u);
        CHECK(stablePtr == &vec[3]); // element storage remains stable even when directory shrinks

        vec.clear();
        CHECK(vec.empty());
        CHECK(vec.capacity() == 8u);

        vec.shrinkToFit();
        CHECK(vec.capacity() == 0u);
    }

    SECTION("Resize, unsafe append helpers, and popBack")
    {
        sf::base::ChunkedVector<int, 2> vec;
        vec.resize(6u);

        CHECK(vec.size() == 6u);
        CHECK(vec.capacity() == 8u);

        for (sf::base::SizeT i = 0u; i < vec.size(); ++i)
            CHECK(vec[i] == 0);

        const int src[] = {10, 20, 30};

        vec.reserve(12u);
        vec.clear();
        vec.unsafeEmplaceBackRange(src, 3u);
        vec.unsafePushBackMultiple(40, 50);

        CHECK(vec == sf::base::ChunkedVector<int, 2>{10, 20, 30, 40, 50});

        vec.popBack();
        CHECK(vec == sf::base::ChunkedVector<int, 2>{10, 20, 30, 40});
    }

    SECTION("Callback iteration helpers")
    {
        sf::base::ChunkedVector<int, 2> vec;

        for (int i = 0; i < 10; ++i)
            vec.pushBack(i);

        int mutatedSum = 0;
        vec.forEach([&](int& x)
        {
            mutatedSum += x;
            x *= 2;
        });

        CHECK(mutatedSum == 45);

        sf::base::SizeT indexedCount = 0u;
        vec.forEachIndexed([&](const sf::base::SizeT i, int& x)
        {
            CHECK(x == static_cast<int>(i * 2u));
            ++indexedCount;
        });

        CHECK(indexedCount == vec.size());

        sf::base::SizeT blockCount   = 0u;
        sf::base::SizeT elementCount = 0u;
        int             blockSum     = 0;

        asConst(vec).forEachBlock([&](const int* begin, const int* end)
        {
            ++blockCount;
            elementCount += static_cast<sf::base::SizeT>(end - begin);

            for (const int* p = begin; p != end; ++p)
                blockSum += *p;
        });

        CHECK(blockCount == 3u);
        CHECK(elementCount == 10u);
        CHECK(blockSum == 90);

        CHECK(vec.findIf([](const int x) { return x == 12; }) == &vec[6]);
        CHECK(asConst(vec).findIf([](const int x) { return x == 18; }) == &asConst(vec)[9]);
        CHECK(vec.findIf([](const int x) { return x == 999; }) == nullptr);

        const int reduced = asConst(vec).reduce(0, [](int acc, const int x) { return acc + x; });
        CHECK(reduced == 90);
    }

    SECTION("Iterator compatibility")
    {
        sf::base::ChunkedVector<int, 2> vec{0, 1, 2, 3, 4, 5, 6};

        int rangeForSum = 0;
        for (const int x : vec)
            rangeForSum += x;

        CHECK(rangeForSum == 21);

        auto it = vec.begin();
        it += 5;

        CHECK(*it == 5);
        CHECK(it[-2] == 3);
        CHECK(it - vec.begin() == 5);
        CHECK(*(it - 1) == 4);
        CHECK(vec.end() - vec.begin() == 7);

        const auto cit = asConst(vec).begin() + 6;
        CHECK(*cit == 6);
    }

    SECTION("Non-trivial growth does not relocate existing elements")
    {
        resetCounters();

        {
            sf::base::ChunkedVector<Obj, 2> vec;
            vec.emplaceBack(1);
            vec.emplaceBack(2);
            vec.emplaceBack(3);

            Obj* const stablePtr = &vec[1];

            CHECK(intCtorCount == 3);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);

            vec.reserve(12u);

            CHECK(stablePtr == &vec[1]);
            CHECK(stablePtr->value == 2);
            CHECK(copyCtorCount == 0);
            CHECK(moveCtorCount == 0);
        }

        CHECK(dtorCount == 3);
    }

    SECTION("Non-trivial copy and move semantics")
    {
        resetCounters();

        sf::base::ChunkedVector<Obj, 2> source;
        source.emplaceBack(1);
        source.emplaceBack(2);
        source.emplaceBack(3);

        Obj* const stablePtr = &source[1];

        resetCounters();

        sf::base::ChunkedVector<Obj, 2> copy(source);
        sf::base::ChunkedVector<Obj, 2> assigned;
        assigned = source;

        CHECK(copyCtorCount == 6);
        CHECK(copyAssignCount == 0);
        CHECK(moveCtorCount == 0);
        CHECK(moveAssignCount == 0);
        CHECK_UNARY(copy == source);
        CHECK_UNARY(assigned == source);

        sf::base::ChunkedVector<Obj, 2> moved(SFML_BASE_MOVE(source));
        CHECK(source.empty());
        CHECK(stablePtr == &moved[1]);

        sf::base::ChunkedVector<Obj, 2> moveAssigned;
        moveAssigned = SFML_BASE_MOVE(copy);

        CHECK(copy.empty());
        CHECK(moveAssigned.size() == 3u);
        CHECK(moveAssigned[0].value == 1);
        CHECK(moveAssigned[1].value == 2);
        CHECK(moveAssigned[2].value == 3);
    }

    SECTION("Non-trivial resize, popBack, and clear destroy the right objects")
    {
        resetCounters();

        sf::base::ChunkedVector<Obj, 2> vec;
        vec.emplaceBack(1);
        vec.emplaceBack(2);
        vec.emplaceBack(3);
        vec.resize(5u);

        CHECK(intCtorCount == 3);
        CHECK(defaultCtorCount == 2);
        CHECK(dtorCount == 0);

        vec.resize(2u);
        CHECK(dtorCount == 3);

        vec.popBack();
        CHECK(dtorCount == 4);

        vec.clear();
        CHECK(dtorCount == 5);
        CHECK(vec.empty());
    }

    SECTION("Self-assignment")
    {
        sf::base::ChunkedVector<int, 2> vec{1, 2, 3, 4, 5};

        const auto* const stablePtr = &vec[2];

        auto& ref = vec;
        vec       = ref;
        CHECK(vec.size() == 5u);
        CHECK(vec[0] == 1);
        CHECK(vec[2] == 3);
        CHECK(vec[4] == 5);
        CHECK(stablePtr == &vec[2]);

        auto& moveRef = vec;
        vec           = SFML_BASE_MOVE(moveRef);
        CHECK(vec.size() == 5u);
        CHECK(vec[0] == 1);
        CHECK(vec[2] == 3);
        CHECK(vec[4] == 5);
    }

    SECTION("Swap")
    {
        sf::base::ChunkedVector<int, 2> a{1, 2, 3};
        sf::base::ChunkedVector<int, 2> b{10, 20};

        a.swap(b);
        CHECK(a.size() == 2u);
        CHECK(a[0] == 10);
        CHECK(a[1] == 20);
        CHECK(b.size() == 3u);
        CHECK(b[0] == 1);
        CHECK(b[1] == 2);
        CHECK(b[2] == 3);

        swap(a, b);
        CHECK(a == sf::base::ChunkedVector<int, 2>{1, 2, 3});
        CHECK(b == sf::base::ChunkedVector<int, 2>{10, 20});

        // Self-swap
        a.swap(a);
        CHECK(a == sf::base::ChunkedVector<int, 2>{1, 2, 3});
    }

    SECTION("Reserve smaller than current capacity is a no-op")
    {
        sf::base::ChunkedVector<int, 2> vec{1, 2, 3, 4, 5, 6};

        const auto        capBefore = vec.capacity();
        const auto* const stablePtr = &vec[3];

        vec.reserve(2u);
        CHECK(vec.capacity() == capBefore);
        CHECK(vec.size() == 6u);
        CHECK(stablePtr == &vec[3]);
    }

    SECTION("ShrinkToFit when already tight")
    {
        sf::base::ChunkedVector<int, 2> vec;

        // Push exactly one block worth of elements (blockSize == 4 for BlockShift=2)
        vec.pushBack(1);
        vec.pushBack(2);
        vec.pushBack(3);
        vec.pushBack(4);

        const auto capBefore = vec.capacity();
        CHECK(capBefore == 4u);

        vec.shrinkToFit();
        CHECK(vec.capacity() == capBefore);
        CHECK(vec.size() == 4u);

        for (sf::base::SizeT i = 0u; i < 4u; ++i)
            CHECK(vec[i] == static_cast<int>(i + 1));
    }

    SECTION("Block boundary crossing")
    {
        sf::base::ChunkedVector<int, 2> vec;

        // Push exactly blockSize elements (fills one block exactly)
        for (sf::base::SizeT i = 0u; i < 4u; ++i)
            vec.pushBack(static_cast<int>(i * 10u));

        CHECK(vec.size() == 4u);
        CHECK(vec.capacity() == 4u);

        // Push one more -- crosses into a second block
        vec.pushBack(40);
        CHECK(vec.size() == 5u);
        CHECK(vec.capacity() == 8u);

        for (sf::base::SizeT i = 0u; i < 5u; ++i)
            CHECK(vec[i] == static_cast<int>(i * 10u));

        // Verify pointer stability across the boundary
        const int* const ptrInBlock0 = &vec[3];
        const int* const ptrInBlock1 = &vec[4];

        vec.pushBack(50);
        vec.pushBack(60);
        vec.pushBack(70);

        CHECK(ptrInBlock0 == &vec[3]);
        CHECK(ptrInBlock1 == &vec[4]);
        CHECK(*ptrInBlock0 == 30);
        CHECK(*ptrInBlock1 == 40);
    }

    SECTION("Reduce on empty vector")
    {
        const sf::base::ChunkedVector<int, 2> vec;
        CHECK(vec.reduce(42, [](int acc, int x) { return acc + x; }) == 42);
    }

    SECTION("FindIf on empty vector")
    {
        sf::base::ChunkedVector<int, 2> vec;
        CHECK(vec.findIf([](int) { return true; }) == nullptr);
        CHECK(asConst(vec).findIf([](int) { return true; }) == nullptr);
    }
}

} // namespace ChunkedVectorTest
} // namespace
