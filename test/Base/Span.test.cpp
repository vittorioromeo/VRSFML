#include "SFML/Base/Span.hpp"

#include "SFML/Base/Trait/IsAggregate.hpp"
#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Trait/IsStandardLayout.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/Span.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::Span<int>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::Span<int>)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::Span<int>));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::Span<int>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::Span<int>, sf::base::Span<int>));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::Span<int>));
    }

    SECTION("Structured bindings")
    {
        sf::base::Span<int> span{nullptr, 0u};

        auto [data, size] = span;

        CHECK(data == nullptr);
        CHECK(size == 0u);
    }

    SECTION("Range constructor from sf::base::Vector")
    {
        sf::base::Vector<int> vec;
        vec.pushBack(10);
        vec.pushBack(20);
        vec.pushBack(30);

        sf::base::Span<int> span{vec};

        CHECK(span.data() == vec.data());
        CHECK(span.size() == 3u);
        CHECK(span[0] == 10);
        CHECK(span[1] == 20);
        CHECK(span[2] == 30);
    }

    SECTION("Range constructor const-correctness")
    {
        sf::base::Vector<int> vec;
        vec.pushBack(1);
        vec.pushBack(2);

        const sf::base::Vector<int>& cvec = vec;

        sf::base::Span<const int> cspan{cvec};
        CHECK(cspan.data() == cvec.data());
        CHECK(cspan.size() == 2u);

        sf::base::Span<const int> cspan2{vec};
        CHECK(cspan2.size() == 2u);

        // Constructing a non-const Span from a non-const range is allowed.
        STATIC_CHECK(SFML_BASE_IS_CONSTRUCTIBLE(sf::base::Span<int>, sf::base::Vector<int>&));

        // Constructing a non-const Span from a const range must be rejected.
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::base::Span<int>, const sf::base::Vector<int>&));

        // Constructing a const Span from either is allowed.
        STATIC_CHECK(SFML_BASE_IS_CONSTRUCTIBLE(sf::base::Span<const int>, sf::base::Vector<int>&));
        STATIC_CHECK(SFML_BASE_IS_CONSTRUCTIBLE(sf::base::Span<const int>, const sf::base::Vector<int>&));

        // Types lacking .data()/.size() must not match the range constructor.
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::base::Span<int>, int));
    }
}
