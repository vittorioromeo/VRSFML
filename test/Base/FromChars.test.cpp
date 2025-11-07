#include "SFML/Base/FromChars.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"

#include <Doctest.hpp>

#include <limits>


TEST_CASE("[Base] FromChars.hpp")
{
    SECTION("fromChars - Integral Types")
    {
        SECTION("Valid Signed Integers")
        {
            int         value  = 0;
            const char* str    = "12345";
            auto        result = sf::base::fromChars(str, str + 5, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 5);
            CHECK(value == 12'345);

            str    = "+678";
            result = sf::base::fromChars(str, str + 4, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 4);
            CHECK(value == 678);

            str    = "-987";
            result = sf::base::fromChars(str, str + 4, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 4);
            CHECK(value == -987);

            str    = "0";
            result = sf::base::fromChars(str, str + 1, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 1);
            CHECK(value == 0);
        }

        SECTION("Valid Unsigned Integers")
        {
            unsigned int value  = 0;
            const char*  str    = "12345";
            auto         result = sf::base::fromChars(str, str + 5, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 5);
            CHECK(value == 12'345);

            str    = "+678";
            result = sf::base::fromChars(str, str + 4, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 4);
            CHECK(value == 678);
        }

        SECTION("Partial Parsing")
        {
            int         value  = 0;
            const char* str    = "99bottles";
            const auto  result = sf::base::fromChars(str, str + 9, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 2); // Should point to 'b'
            CHECK(value == 99);
        }

        SECTION("Max/Min Values")
        {
            // Signed int max
            int              iValue  = 0;
            sf::base::String iMaxStr = sf::base::toString(std::numeric_limits<int>::max());
            auto             result  = sf::base::fromChars(iMaxStr.cStr(), iMaxStr.cStr() + iMaxStr.size(), iValue);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(iValue == std::numeric_limits<int>::max());

            // Unsigned long long max
            unsigned long long ullValue     = 0;
            sf::base::String   ullMaxString = sf::base::toString(std::numeric_limits<unsigned long long>::max());
            result = sf::base::fromChars(ullMaxString.cStr(), ullMaxString.cStr() + ullMaxString.size(), ullValue);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(ullValue == std::numeric_limits<unsigned long long>::max());
        }

        SECTION("Error: Invalid Argument")
        {
            int         value  = 1; // Should not be modified
            const char* str    = "";
            auto        result = sf::base::fromChars(str, str, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            str    = "+";
            result = sf::base::fromChars(str, str + 1, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            str    = "-";
            result = sf::base::fromChars(str, str + 1, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            str    = "abc";
            result = sf::base::fromChars(str, str + 3, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            // Negative sign for unsigned type
            unsigned int uValue = 1;
            str                 = "-123";
            result              = sf::base::fromChars(str, str + 4, uValue);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(uValue == 1);
        }

        SECTION("Error: Result Out Of Range")
        {
            // Overflow for signed int
            int              iValue       = 1; // Should not be modified
            sf::base::String iOverflowStr = sf::base::toString(std::numeric_limits<long long>::max());
            auto result = sf::base::fromChars(iOverflowStr.cStr(), iOverflowStr.cStr() + iOverflowStr.size(), iValue);
            CHECK(result.ec == sf::base::FromCharsError::ResultOutOfRange);
            CHECK(iValue == 1);

            // Overflow for uint8_t
            sf::base::U8 u8Value = 1;
            const char*  str     = "256";
            result               = sf::base::fromChars(str, str + 3, u8Value);
            CHECK(result.ec == sf::base::FromCharsError::ResultOutOfRange);
            CHECK(u8Value == 1);

            str    = "1000";
            result = sf::base::fromChars(str, str + 4, u8Value);
            CHECK(result.ec == sf::base::FromCharsError::ResultOutOfRange);
            CHECK(u8Value == 1);
        }
    }

    SECTION("fromChars - Floating-Point Types")
    {
        SECTION("Valid Floats")
        {
            double      value  = 0.0;
            const char* str    = "123.456";
            auto        result = sf::base::fromChars(str, str + 7, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 7);
            CHECK(value == doctest::Approx(123.456));

            str    = "-0.123";
            result = sf::base::fromChars(str, str + 6, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 6);
            CHECK(value == doctest::Approx(-0.123));

            str    = "+789.";
            result = sf::base::fromChars(str, str + 5, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 5);
            CHECK(value == doctest::Approx(789.0));

            str    = "500";
            result = sf::base::fromChars(str, str + 3, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 3);
            CHECK(value == doctest::Approx(500.0));

            float fValue = 0.0f;
            str          = ".25";
            result       = sf::base::fromChars(str, str + 3, fValue);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 3);
            CHECK(fValue == doctest::Approx(0.25));
        }

        SECTION("Zero")
        {
            double      value  = 1.0;
            const char* str    = "0.0";
            auto        result = sf::base::fromChars(str, str + 3, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(value == doctest::Approx(0.0));

            str    = "0";
            result = sf::base::fromChars(str, str + 1, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(value == doctest::Approx(0.0));
        }

        SECTION("Partial Parsing")
        {
            double      value  = 0.0;
            const char* str    = "3.14159andthensome";
            auto        result = sf::base::fromChars(str, str + 18, value);
            CHECK(result.ec == sf::base::FromCharsError::None);
            CHECK(result.ptr == str + 7); // Should point to 'a'
            CHECK(value == doctest::Approx(3.14159));
        }

        SECTION("Error: Invalid Argument")
        {
            double      value  = 1.0; // Should not be modified
            const char* str    = "";
            auto        result = sf::base::fromChars(str, str, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == doctest::Approx(1.0));

            str    = "+";
            result = sf::base::fromChars(str, str + 1, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == doctest::Approx(1.0));

            str    = "-";
            result = sf::base::fromChars(str, str + 1, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == doctest::Approx(1.0));

            str    = ".";
            result = sf::base::fromChars(str, str + 1, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == doctest::Approx(1.0));

            str    = "xyz";
            result = sf::base::fromChars(str, str + 3, value);
            CHECK(result.ec == sf::base::FromCharsError::InvalidArgument);
            CHECK(value == doctest::Approx(1.0));
        }
    }
}
