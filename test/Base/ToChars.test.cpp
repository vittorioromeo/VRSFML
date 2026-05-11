#include "StringifyStringViewUtil.hpp"

#include "SFML/Base/ToChars.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"

#include <Doctest.hpp>

#include <limits>


#define CHECK_INTEGER_CONVERSION(value, expected_str)                                                    \
    do                                                                                                   \
    {                                                                                                    \
        char              buffer[64];                                                                    \
        const char* const last = buffer + sizeof(buffer);                                                \
        const char*       end  = sf::base::toChars(buffer, last, value);                                 \
                                                                                                         \
        CHECK(end != nullptr);                                                                           \
        CHECK(sf::base::StringView(buffer, static_cast<sf::base::SizeT>(end - buffer)) == expected_str); \
    } while (false)


#define CHECK_FLOAT_CONVERSION(value, expected_str, precision)                                           \
    do                                                                                                   \
    {                                                                                                    \
        char              buffer[64];                                                                    \
        const char* const last = buffer + sizeof(buffer);                                                \
        const char*       end  = sf::base::toChars(buffer, last, value, precision);                      \
                                                                                                         \
        CHECK(end != nullptr);                                                                           \
        CHECK(sf::base::StringView(buffer, static_cast<sf::base::SizeT>(end - buffer)) == expected_str); \
    } while (false)


TEST_CASE("[Base] ToChars.hpp")
{
    SECTION("Unsigned Integers")
    {
        CHECK_INTEGER_CONVERSION(0u, "0");
        CHECK_INTEGER_CONVERSION(123u, "123");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<unsigned int>::max(), "4294967295");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<sf::base::U64>::max(), "18446744073709551615");
    }

    SECTION("Signed Integers")
    {
        CHECK_INTEGER_CONVERSION(0, "0");
        CHECK_INTEGER_CONVERSION(456, "456");
        CHECK_INTEGER_CONVERSION(-789, "-789");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<int>::max(), "2147483647");

        // Special case: T_MIN requires correct unsigned handling
        CHECK_INTEGER_CONVERSION(std::numeric_limits<int>::min(), "-2147483648");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<sf::base::I64>::min(), "-9223372036854775808");
    }

    SECTION("Narrow signed types (must survive integer promotion of -uValue)")
    {
        // For types narrower than `int`, unary `-` on the unsigned counterpart
        // promotes to `int` first; without an explicit cast back, the result
        // would be a negative `int` and `unsignedToChars` would print nothing
        // after the leading '-'.
        CHECK_INTEGER_CONVERSION(static_cast<short>(-32'000), "-32000");
        CHECK_INTEGER_CONVERSION(static_cast<short>(32'000), "32000");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<short>::min(), "-32768");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<short>::max(), "32767");

        CHECK_INTEGER_CONVERSION(static_cast<signed char>(-100), "-100");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<signed char>::min(), "-128");
        CHECK_INTEGER_CONVERSION(std::numeric_limits<signed char>::max(), "127");
    }

    SECTION("Buffer Overrun Checks")
    {
        // Value "12345" requires 5 characters
        const int val = 12'345;
        char      buffer[10];

        // Buffer too small by one
        REQUIRE(sf::base::toChars(buffer, buffer + 4, val) == nullptr);

        // Buffer is exactly the right size
        const char* end = sf::base::toChars(buffer, buffer + 5, val);
        REQUIRE(end == buffer + 5);
        REQUIRE(sf::base::StringView(buffer, static_cast<sf::base::SizeT>(end - buffer)) == "12345");

        // Negative value "-987" requires 4 characters
        const int negVal = -987;

        // Buffer too small by one
        REQUIRE(sf::base::toChars(buffer, buffer + 3, negVal) == nullptr);

        // Buffer is exactly the right size
        end = sf::base::toChars(buffer, buffer + 4, negVal);
        REQUIRE(end == buffer + 4);
        REQUIRE(sf::base::StringView(buffer, static_cast<sf::base::SizeT>(end - buffer)) == "-987");

        // Zero with an empty buffer
        REQUIRE(sf::base::toChars(buffer, buffer, 0) == nullptr);
    }

    SECTION("Basic Conversions with default precision (6)")
    {
        CHECK_FLOAT_CONVERSION(0.0, "0.000000", 6);
        CHECK_FLOAT_CONVERSION(123.456, "123.456000", 6);
        CHECK_FLOAT_CONVERSION(-78.9, "-78.900000", 6);
        CHECK_FLOAT_CONVERSION(0.123456, "0.123456", 6);
        CHECK_FLOAT_CONVERSION(98765.0, "98765.000000", 6);
    }

    SECTION("Rounding behavior")
    {
        // Rounds up
        CHECK_FLOAT_CONVERSION(1.2345678, "1.234568", 6);
        // Rounds down
        CHECK_FLOAT_CONVERSION(1.1234561, "1.123456", 6);
        // Rounds up and carries over to integer part
        CHECK_FLOAT_CONVERSION(9.9999999, "10.000000", 6);
        CHECK_FLOAT_CONVERSION(-9.9999999, "-10.000000", 6);
    }

    SECTION("Custom Precision")
    {
        CHECK_FLOAT_CONVERSION(3.14159, "3.14", 2);
        CHECK_FLOAT_CONVERSION(1.234, "1.2340000000", 10);
        CHECK_FLOAT_CONVERSION(0.007, "0.01", 2); // Handles leading zeros in fraction
        CHECK_FLOAT_CONVERSION(-0.007, "-0.01", 2);

        // Precision 0 should not print a decimal point
        CHECK_FLOAT_CONVERSION(123.456, "123", 0);
        CHECK_FLOAT_CONVERSION(0.7, "1", 0); // rint(0.7) is 1
    }

    SECTION("Special values: NaN, infinity, negative zero")
    {
        // NaN: "nan", no sign (matches `std::to_chars` / IEEE-754)
        CHECK_FLOAT_CONVERSION(std::numeric_limits<double>::quiet_NaN(), "nan", 6);

        // +/- infinity
        CHECK_FLOAT_CONVERSION(std::numeric_limits<double>::infinity(), "inf", 6);
        CHECK_FLOAT_CONVERSION(-std::numeric_limits<double>::infinity(), "-inf", 6);

        // Negative zero preserves its sign (matches `std::to_chars`)
        CHECK_FLOAT_CONVERSION(-0.0, "-0.000000", 6);
        CHECK_FLOAT_CONVERSION(-0.0, "-0.00", 2);
        CHECK_FLOAT_CONVERSION(-0.0, "-0", 0);
    }

    SECTION("Banker's rounding (round-half-to-even)")
    {
        // `rint` uses the default IEEE-754 rounding mode (round-half-to-even),
        // matching `std::to_chars`. Half-cases round to the nearest even integer.
        CHECK_FLOAT_CONVERSION(0.5, "0", 0); // 0.5 → 0 (0 is even)
        CHECK_FLOAT_CONVERSION(1.5, "2", 0); // 1.5 → 2 (2 is even)
        CHECK_FLOAT_CONVERSION(2.5, "2", 0); // 2.5 → 2 (2 is even)
        CHECK_FLOAT_CONVERSION(3.5, "4", 0); // 3.5 → 4 (4 is even)
    }

    SECTION("Out-of-range values return nullptr")
    {
        char              buffer[64];
        const char* const last = buffer + sizeof(buffer);

        // Values beyond ~9.2e18 cannot be represented as `long long`.
        CHECK(sf::base::toChars(buffer, last, 1.0e20, 0) == nullptr);
        CHECK(sf::base::toChars(buffer, last, std::numeric_limits<double>::max(), 0) == nullptr);

        // At precision = 10, the multiplier is 10^10, so even 1e10 overflows.
        CHECK(sf::base::toChars(buffer, last, 1.0e10, 10) == nullptr);
    }

    SECTION("Buffer Overrun Checks")
    {
        // Value 12.345 with precision 3 is "12.345" (6 chars)
        const double val  = 12.345;
        const int    prec = 3;
        char         buffer[10];

        // Buffer too small by one
        REQUIRE(sf::base::toChars(buffer, buffer + 5, val, prec) == nullptr);

        // Buffer is exactly the right size
        const char* end = sf::base::toChars(buffer, buffer + 6, val, prec);
        REQUIRE(end == buffer + 6);
        REQUIRE(sf::base::StringView(buffer, static_cast<sf::base::SizeT>(end - buffer)) == "12.345");

        // Negative value -1.2 with precision 1 is "-1.2" (4 chars)
        const double negVal  = -1.2;
        const int    negPrec = 1;

        // Buffer too small by one
        REQUIRE(sf::base::toChars(buffer, buffer + 3, negVal, negPrec) == nullptr);

        // Buffer is exactly the right size
        end = sf::base::toChars(buffer, buffer + 4, negVal, negPrec);
        REQUIRE(end == buffer + 4);
        REQUIRE(sf::base::StringView(buffer, static_cast<sf::base::SizeT>(end - buffer)) == "-1.2");

        // Case where integer part fits but rest does not
        // "123.4" (5 chars), buffer for only "123" (3 chars)
        REQUIRE(sf::base::toChars(buffer, buffer + 3, 123.4, 1) == nullptr);
    }
}
