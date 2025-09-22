#include "SFML/Base/MiniPFR.hpp"

#include <Doctest.hpp>


namespace
{
////////////////////////////////////////////////////////////
struct S0
{
};

////////////////////////////////////////////////////////////
struct S1
{
    int i;
};

////////////////////////////////////////////////////////////
struct S2
{
    int   i;
    float f;
};

////////////////////////////////////////////////////////////
struct S3
{
    int   i;
    float f;
    char  c;
};

}; // namespace


TEST_CASE("[Base] Base/MiniPFR.hpp")
{
    SECTION("NumFields")
    {
        STATIC_CHECK(sf::base::minipfr::numFields<S0> == 0);
        STATIC_CHECK(sf::base::minipfr::numFields<S1> == 1);
        STATIC_CHECK(sf::base::minipfr::numFields<S2> == 2);
        STATIC_CHECK(sf::base::minipfr::numFields<S3> == 3);
    }

    SECTION("TieAsTuple")
    {
        {
            S0   obj{};
            auto tpl = sf::base::minipfr::tieAsTuple(obj);

            STATIC_CHECK(decltype(tpl)::memberCount == 0);
        }

        {
            S1   obj{42};
            auto tpl = sf::base::minipfr::tieAsTuple(obj);

            STATIC_CHECK(decltype(tpl)::memberCount == 1);

            CHECK(tpl.get<0>() == 42);

            tpl.get<0>() = 4;
            CHECK(obj.i == 4);
        }

        {
            S2   obj{42, 10.f};
            auto tpl = sf::base::minipfr::tieAsTuple(obj);

            STATIC_CHECK(decltype(tpl)::memberCount == 2);

            CHECK(tpl.get<0>() == 42);
            CHECK(tpl.get<1>() == 10.f);

            tpl.get<0>() = 4;
            CHECK(obj.i == 4);
        }

        {
            S3   obj{42, 10.f, 'x'};
            auto tpl = sf::base::minipfr::tieAsTuple(obj);

            STATIC_CHECK(decltype(tpl)::memberCount == 3);

            CHECK(tpl.get<0>() == 42);
            CHECK(tpl.get<1>() == 10.f);
            CHECK(tpl.get<2>() == 'x');

            tpl.get<0>() = 10;
            CHECK(obj.i == 10);
        }
    }

    SECTION("GetField")
    {
        {
            S1 obj{42};

            CHECK(sf::base::minipfr::getField<0>(obj) == 42);

            sf::base::minipfr::getField<0>(obj) = 4;
            CHECK(obj.i == 4);
        }

        {
            S2 obj{42, 10.f};

            CHECK(sf::base::minipfr::getField<0>(obj) == 42);
            CHECK(sf::base::minipfr::getField<1>(obj) == 10.f);

            sf::base::minipfr::getField<0>(obj) = 4;
            CHECK(obj.i == 4);
        }

        {
            S3 obj{42, 10.f, 'x'};

            CHECK(sf::base::minipfr::getField<0>(obj) == 42);
            CHECK(sf::base::minipfr::getField<1>(obj) == 10.f);
            CHECK(sf::base::minipfr::getField<2>(obj) == 'x');

            sf::base::minipfr::getField<0>(obj) = 10;
            CHECK(obj.i == 10);
        }
    }
}
