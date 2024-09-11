#include "SFML/Base/TrivialVector.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsCopyAssignable.hpp"
#include "SFML/Base/Traits/IsCopyConstructible.hpp"
#include "SFML/Base/Traits/IsMoveAssignable.hpp"
#include "SFML/Base/Traits/IsMoveConstructible.hpp"
#include "SFML/Base/Traits/IsTrivial.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveConstructible.hpp"

#include <Doctest.hpp>

namespace
{
TEST_CASE("[Base] Base/TrivialVector.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIAL(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(int));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(int));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::TrivialVector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::TrivialVector<int>));

        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::base::TrivialVector<int>));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::base::TrivialVector<int>));
    }

    SECTION("Empty")
    {
#define DO_EMPTY_CHECKS(tv)                                                           \
    CHECK((tv).begin() == nullptr);                                                   \
    CHECK((tv).end() == nullptr);                                                     \
    CHECK((tv).data() == nullptr);                                                    \
                                                                                      \
    CHECK(static_cast<const sf::base::TrivialVector<int>&>((tv)).begin() == nullptr); \
    CHECK(static_cast<const sf::base::TrivialVector<int>&>((tv)).end() == nullptr);   \
    CHECK(static_cast<const sf::base::TrivialVector<int>&>((tv)).data() == nullptr);  \
                                                                                      \
    CHECK((tv).size() == 0u);                                                         \
    CHECK((tv).empty());

        sf::base::TrivialVector<int> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.clear();
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        sf::base::TrivialVector<int> tv2 = tv;
        DO_EMPTY_CHECKS(tv2);
        CHECK(tv2.capacity() == 0u);

        sf::base::TrivialVector<int> tv3 = SFML_BASE_MOVE(tv);
        DO_EMPTY_CHECKS(tv3);
        CHECK(tv3.capacity() == 0u);

        sf::base::TrivialVector<int> tv4;
        tv4 = tv;
        DO_EMPTY_CHECKS(tv4);
        CHECK(tv4.capacity() == 0u);

        sf::base::TrivialVector<int> tv5;
        tv5 = SFML_BASE_MOVE(tv4);
        DO_EMPTY_CHECKS(tv5);
        CHECK(tv5.capacity() == 0u);
    }

    SECTION("Non-empty")
    {
        sf::base::TrivialVector<int> tv;
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
}

} // namespace
