#include "Tst/Tst.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"


TEST_CASE("[Base] sf::base::ankerl::map::replace_key")
{
    using Map = ankerl::unordered_dense::map<sf::base::U64, sf::base::U64>;

    SECTION("Successful replacement returns {it, true}")
    {
        Map m;
        m.try_emplace(1u, 100u);
        m.try_emplace(2u, 200u);
        m.try_emplace(3u, 300u);

        auto* const it = m.find(2u);
        REQUIRE(it != m.end());

        const auto [newIt, ok] = m.replace_key(it, 42u);
        CHECK(ok);
        CHECK(newIt == it); // same iterator position
        CHECK(newIt->first == 42u);
        CHECK(newIt->second == 200u); // value preserved

        // Old key no longer present.
        CHECK(m.find(2u) == m.end());
        // New key is found.
        auto* const found = m.find(42u);
        REQUIRE(found != m.end());
        CHECK(found->second == 200u);

        // Other entries untouched.
        CHECK(m.find(1u)->second == 100u);
        CHECK(m.find(3u)->second == 300u);

        // Size unchanged.
        CHECK(m.size() == 3u);
    }

    SECTION("Replacement to an existing key fails and returns {iter-to-existing, false}")
    {
        Map m;
        m.try_emplace(1u, 100u);
        m.try_emplace(2u, 200u);

        auto* const it1 = m.find(1u);
        REQUIRE(it1 != m.end());

        const auto [retIt, ok] = m.replace_key(it1, 2u);
        CHECK(!ok);
        CHECK(retIt != m.end());
        CHECK(retIt->first == 2u);
        CHECK(retIt->second == 200u);

        // Both original keys still present, untouched.
        CHECK(m.find(1u) != m.end());
        CHECK(m.find(1u)->second == 100u);
        CHECK(m.find(2u)->second == 200u);
        CHECK(m.size() == 2u);
    }

    SECTION("Replace many keys preserves all values")
    {
        Map m;
        for (sf::base::U64 i = 0; i < 100u; ++i)
            m.try_emplace(i, i * 10u);

        // Shift every key by +1000.
        for (sf::base::U64 i = 0; i < 100u; ++i)
        {
            auto* const it = m.find(i);
            REQUIRE(it != m.end());
            const auto [newIt, ok] = m.replace_key(it, i + 1000u);
            CHECK(ok);
            CHECK(newIt->second == i * 10u);
        }

        CHECK(m.size() == 100u);

        // Verify all new keys present with correct values.
        for (sf::base::U64 i = 0; i < 100u; ++i)
        {
            auto* const it = m.find(i + 1000u);
            REQUIRE(it != m.end());
            CHECK(it->second == i * 10u);
        }

        // None of the old keys remain.
        for (sf::base::U64 i = 0; i < 100u; ++i)
            CHECK(m.find(i) == m.end());
    }

    SECTION("Replace with the same key is a no-op success")
    {
        Map m;
        m.try_emplace(5u, 500u);

        auto* const it = m.find(5u);
        REQUIRE(it != m.end());

        // replace_key(it, same_key) hits the "already exists" branch and
        // returns {it_to_existing, false}.
        const auto [retIt, ok] = m.replace_key(it, 5u);
        CHECK(!ok);
        CHECK(retIt->first == 5u);
        CHECK(retIt->second == 500u);
        CHECK(m.size() == 1u);
    }
}


TEST_CASE("[Base] sf::base::ankerl::segmented_vector::resize")
{
    using Vec = ankerl::unordered_dense::v4_8_1::segmented_vector<sf::base::U64>;

    SECTION("Grow with default value-init")
    {
        Vec v;
        v.resize(5u);
        CHECK(v.size() == 5u);
        for (sf::base::SizeT i = 0; i < 5u; ++i)
            CHECK(v[i] == 0u);
    }

    SECTION("Grow with explicit value")
    {
        Vec v;
        v.resize(4u, 42u);
        CHECK(v.size() == 4u);
        for (sf::base::SizeT i = 0; i < 4u; ++i)
            CHECK(v[i] == 42u);
    }

    SECTION("Shrink drops trailing elements")
    {
        Vec v;
        v.emplace_back(1u);
        v.emplace_back(2u);
        v.emplace_back(3u);
        v.resize(1u);
        CHECK(v.size() == 1u);
        CHECK(v[0] == 1u);
    }

    SECTION("Resize to current size is a no-op")
    {
        Vec v;
        v.emplace_back(7u);
        v.emplace_back(8u);
        v.resize(2u);
        CHECK(v.size() == 2u);
        CHECK(v[0] == 7u);
        CHECK(v[1] == 8u);
    }

    SECTION("Resize across segment boundary")
    {
        Vec v;
        // segment size is at least 1 element; force growth past one segment.
        v.resize(1000u, 99u);
        CHECK(v.size() == 1000u);
        CHECK(v[0] == 99u);
        CHECK(v[500] == 99u);
        CHECK(v[999] == 99u);

        v.resize(10u);
        CHECK(v.size() == 10u);
        CHECK(v[0] == 99u);
        CHECK(v[9] == 99u);
    }
}


TEST_CASE("[Base] sf::base::ankerl::segmented_vector iterator is random-access")
{
    using Vec = ankerl::unordered_dense::v4_8_1::segmented_vector<sf::base::U64>;

    Vec v;
    for (sf::base::U64 i = 0; i < 10u; ++i)
        v.emplace_back(i);

    auto it = v.begin();

    SECTION("operator++ / operator--")
    {
        ++it;
        CHECK(*it == 1u);
        it++;
        CHECK(*it == 2u);
        --it;
        CHECK(*it == 1u);
        it--;
        CHECK(*it == 0u);
    }

    SECTION("operator+= / operator-=")
    {
        it += 5;
        CHECK(*it == 5u);
        it -= 3;
        CHECK(*it == 2u);
    }

    SECTION("operator+ scalar / operator- scalar")
    {
        const auto it2 = it + 7;
        CHECK(*it2 == 7u);
        const auto it3 = it2 - 4;
        CHECK(*it3 == 3u);
    }

    SECTION("operator- (iterator difference)")
    {
        const auto a = v.begin();
        const auto b = a + 6;
        CHECK(b - a == 6);
    }

    SECTION("Ordering comparisons")
    {
        const auto a = v.begin();
        const auto b = a + 3;
        // Wrap each comparison in `static_cast<bool>` so the framework's expression
        // decomposer doesn't need to stringify iterator operands.
        CHECK(static_cast<bool>(a < b));
        CHECK(static_cast<bool>(b > a));
        CHECK(static_cast<bool>(a <= b));
        CHECK(static_cast<bool>(b >= a));
        CHECK(static_cast<bool>(a <= a));
        CHECK(static_cast<bool>(a >= a));
        CHECK(!static_cast<bool>(b < a));
        CHECK(!static_cast<bool>(a > b));
    }
}
