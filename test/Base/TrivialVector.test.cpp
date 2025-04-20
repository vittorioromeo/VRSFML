#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsCopyAssignable.hpp"
#include "SFML/Base/Traits/IsCopyConstructible.hpp"
#include "SFML/Base/Traits/IsMoveAssignable.hpp"
#include "SFML/Base/Traits/IsMoveConstructible.hpp"
#include "SFML/Base/Traits/IsTrivial.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


namespace
{
TEST_CASE("[Base] Base/Vector.hpp")
{
    const auto asConst = [](auto& x) -> const auto& { return x; };

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIAL(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(int));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Vector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Vector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Vector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Vector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Vector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Vector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Vector<int>));

        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::base::Vector<int>));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::base::Vector<int>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::base::Vector<int>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::base::Vector<int>));
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

        sf::base::Vector<int> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.clear();
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        sf::base::Vector<int> tv2 = tv;
        DO_EMPTY_CHECKS(tv2);
        CHECK(tv2.capacity() == 0u);

        sf::base::Vector<int> tv3 = SFML_BASE_MOVE(tv);
        DO_EMPTY_CHECKS(tv3);
        CHECK(tv3.capacity() == 0u);

        sf::base::Vector<int> tv4;
        tv4 = tv;
        DO_EMPTY_CHECKS(tv4);
        CHECK(tv4.capacity() == 0u);

        sf::base::Vector<int> tv5;
        tv5 = SFML_BASE_MOVE(tv4);
        DO_EMPTY_CHECKS(tv5);
        CHECK(tv5.capacity() == 0u);
    }

    SECTION("Non-empty")
    {
        sf::base::Vector<int> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.reserve(1);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 0u);
        CHECK(tv.capacity() == 1u);

        tv.unsafeEmplaceBack(42);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() == 1u);
        CHECK(tv[0] == 42);

        tv.reserveMore(10);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() >= 11u);
        CHECK(tv[0] == 42);

        tv.resize(100);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 100u);
        CHECK(tv.capacity() >= 100u);
        CHECK(tv[0] == 42);

        for (sf::base::SizeT i = 1; i < 100; ++i)
            CHECK(tv[i] == 0);
    }

    SECTION("Non default constructible")
    {
        struct S
        {
            S(int x) : value(x)
            {
            }

            int value;
        };

        sf::base::Vector<S> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.reserve(1);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 0u);
        CHECK(tv.capacity() == 1u);

        tv.unsafeEmplaceBack(42);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() == 1u);
        CHECK(tv[0].value == 42);

        tv.reserveMore(10);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() >= 11u);
        CHECK(tv[0].value == 42);

        tv.reserve(100);
        for (sf::base::SizeT i = 1; i < 100; ++i)
            tv.unsafeEmplaceBack(0);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 100u);
        CHECK(tv.capacity() >= 100u);
        CHECK(tv[0].value == 42);

        for (sf::base::SizeT i = 1; i < 100; ++i)
            CHECK(tv[i].value == 0);
    }

    SECTION("Shrink to fit")
    {
        sf::base::Vector<int> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.reserve(255);

        for (int i = 0; i < 100; ++i)
            tv.unsafeEmplaceBack(5);

        CHECK(tv.size() == 100);
        CHECK(tv.capacity() > 100);

        tv.shrinkToFit();

        CHECK(tv.size() == 100);
        CHECK(tv.capacity() == 100);
    }
}

} // namespace
