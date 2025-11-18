#include "StringifySfBaseStringUtil.hpp" // used
#include "StringifyStringViewUtil.hpp"   // used

#include "SFML/Base/ToString.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/ToString.hpp")
{
    SECTION("Integer types")
    {
        SUBCASE("Zero")
        {
            CHECK(sf::base::toString(0) == "0");
        }

        SUBCASE("Positive integers")
        {
            CHECK(sf::base::toString(123) == "123");
            CHECK(sf::base::toString(98'765) == "98765");
        }

        SUBCASE("Negative integers")
        {
            CHECK(sf::base::toString(-456) == "-456");
            CHECK(sf::base::toString(-1) == "-1");
        }

        SUBCASE("Integer limits")
        {
            CHECK(sf::base::toString(int{2'147'483'647}) == "2147483647");   // INT_MAX
            CHECK(sf::base::toString(int{-2'147'483'648}) == "-2147483648"); // INT_MIN

            CHECK(sf::base::toString(static_cast<long long>(9'223'372'036'854'775'807ll)) == "9223372036854775807"); // LLONG_MAX
        }

        SUBCASE("Unsigned integers")
        {
            CHECK(sf::base::toString(4'294'967'295u) == "4294967295"); // UINT_MAX
        }
    }

    SECTION("Floating-point types")
    {
        SUBCASE("Zero")
        {
            CHECK(sf::base::toString(0.f) == "0.000000");
            CHECK(sf::base::toString(0.0) == "0.000000");
        }

        SUBCASE("Positive floats")
        {
            CHECK(sf::base::toString(123.456f) == "123.456000");
            CHECK(sf::base::toString(0.123) == "0.123000");
        }

        SUBCASE("Negative floats")
        {
            CHECK(sf::base::toString(-78.9) == "-78.900000");
            CHECK(sf::base::toString(-0.001f) == "-0.001000");
        }

        SUBCASE("Integer-like floats")
        {
            CHECK(sf::base::toString(100.0) == "100.000000");
            CHECK(sf::base::toString(-50.f) == "-50.000000");
        }

        SUBCASE("Small fractional part requires padding")
        {
            CHECK(sf::base::toString(1.01) == "1.010000");
        }
    }
}
