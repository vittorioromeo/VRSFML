#include "Tst/Tst.hpp"

#include "Zancle/String/FromChars.hpp"

#include "Zancle/String/String.hpp"
#include "Zancle/String/ToString.hpp"

#include "Zancle/Base/IntTypes.hpp"

#include <limits>


TEST_CASE("[Base] FromChars.hpp")
{
    SECTION("fromChars - Integral Types")
    {
        SECTION("Valid Signed Integers")
        {
            int         value  = 0;
            const char* str    = "12345";
            auto        result = za::fromChars(str, str + 5, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 5);
            CHECK(value == 12'345);

            str    = "+678";
            result = za::fromChars(str, str + 4, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 4);
            CHECK(value == 678);

            str    = "-987";
            result = za::fromChars(str, str + 4, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 4);
            CHECK(value == -987);

            str    = "0";
            result = za::fromChars(str, str + 1, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 1);
            CHECK(value == 0);
        }

        SECTION("Valid Unsigned Integers")
        {
            unsigned int value  = 0;
            const char*  str    = "12345";
            auto         result = za::fromChars(str, str + 5, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 5);
            CHECK(value == 12'345);

            str    = "+678";
            result = za::fromChars(str, str + 4, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 4);
            CHECK(value == 678);
        }

        SECTION("Partial Parsing")
        {
            int         value  = 0;
            const char* str    = "99bottles";
            const auto  result = za::fromChars(str, str + 9, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 2); // Should point to 'b'
            CHECK(value == 99);
        }

        SECTION("Max/Min Values")
        {
            // Signed int max
            int        iValue  = 0;
            za::String iMaxStr = za::toString(std::numeric_limits<int>::max());
            auto       result  = za::fromChars(iMaxStr.cStr(), iMaxStr.cStr() + iMaxStr.size(), iValue);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(iValue == std::numeric_limits<int>::max());

            // Signed int min
            za::String iMinStr = za::toString(std::numeric_limits<int>::min());
            result             = za::fromChars(iMinStr.cStr(), iMinStr.cStr() + iMinStr.size(), iValue);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(iValue == std::numeric_limits<int>::min());

            // Signed long long min
            long long  llValue  = 0;
            za::String llMinStr = za::toString(std::numeric_limits<long long>::min());
            result              = za::fromChars(llMinStr.cStr(), llMinStr.cStr() + llMinStr.size(), llValue);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(llValue == std::numeric_limits<long long>::min());

            // Unsigned long long max
            unsigned long long ullValue     = 0;
            za::String         ullMaxString = za::toString(std::numeric_limits<unsigned long long>::max());
            result = za::fromChars(ullMaxString.cStr(), ullMaxString.cStr() + ullMaxString.size(), ullValue);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(ullValue == std::numeric_limits<unsigned long long>::max());
        }

        SECTION("Error: Invalid Argument")
        {
            int         value  = 1; // Should not be modified
            const char* str    = "";
            auto        result = za::fromChars(str, str, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            str    = "+";
            result = za::fromChars(str, str + 1, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            str    = "-";
            result = za::fromChars(str, str + 1, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            str    = "abc";
            result = za::fromChars(str, str + 3, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == 1);

            // Negative sign for unsigned type
            unsigned int uValue = 1;
            str                 = "-123";
            result              = za::fromChars(str, str + 4, uValue);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(uValue == 1);
        }

        SECTION("Error: Result Out Of Range")
        {
            // Overflow for signed int
            int        iValue       = 1; // Should not be modified
            za::String iOverflowStr = za::toString(std::numeric_limits<long long>::max());
            auto       result = za::fromChars(iOverflowStr.cStr(), iOverflowStr.cStr() + iOverflowStr.size(), iValue);
            CHECK(result.ec == za::FromCharsError::ResultOutOfRange);
            CHECK(iValue == 1);

            // Overflow for uint8_t
            za::U8      u8Value = 1;
            const char* str     = "256";
            result              = za::fromChars(str, str + 3, u8Value);
            CHECK(result.ec == za::FromCharsError::ResultOutOfRange);
            CHECK(u8Value == 1);

            str    = "1000";
            result = za::fromChars(str, str + 4, u8Value);
            CHECK(result.ec == za::FromCharsError::ResultOutOfRange);
            CHECK(u8Value == 1);
        }
    }

    SECTION("fromChars - Floating-Point Types")
    {
        SECTION("Valid Floats")
        {
            double      value  = 0.0;
            const char* str    = "123.456";
            auto        result = za::fromChars(str, str + 7, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 7);
            CHECK(value == tst::Approx(123.456));

            str    = "-0.123";
            result = za::fromChars(str, str + 6, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 6);
            CHECK(value == tst::Approx(-0.123));

            str    = "+789.";
            result = za::fromChars(str, str + 5, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 5);
            CHECK(value == tst::Approx(789.0));

            str    = "500";
            result = za::fromChars(str, str + 3, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 3);
            CHECK(value == tst::Approx(500.0));

            float fValue = 0.0f;
            str          = ".25";
            result       = za::fromChars(str, str + 3, fValue);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 3);
            CHECK(fValue == tst::Approx(0.25));
        }

        SECTION("Zero")
        {
            double      value  = 1.0;
            const char* str    = "0.0";
            auto        result = za::fromChars(str, str + 3, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(value == tst::Approx(0.0));

            str    = "0";
            result = za::fromChars(str, str + 1, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(value == tst::Approx(0.0));
        }

        SECTION("Partial Parsing")
        {
            double      value  = 0.0;
            const char* str    = "3.14159andthensome";
            auto        result = za::fromChars(str, str + 18, value);
            CHECK(result.ec == za::FromCharsError::None);
            CHECK(result.ptr == str + 7); // Should point to 'a'
            CHECK(value == tst::Approx(3.14159));
        }

        SECTION("Error: Invalid Argument")
        {
            double      value  = 1.0; // Should not be modified
            const char* str    = "";
            auto        result = za::fromChars(str, str, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == tst::Approx(1.0));

            str    = "+";
            result = za::fromChars(str, str + 1, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == tst::Approx(1.0));

            str    = "-";
            result = za::fromChars(str, str + 1, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == tst::Approx(1.0));

            str    = ".";
            result = za::fromChars(str, str + 1, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == tst::Approx(1.0));

            str    = "xyz";
            result = za::fromChars(str, str + 3, value);
            CHECK(result.ec == za::FromCharsError::InvalidArgument);
            CHECK(value == tst::Approx(1.0));
        }
    }
}
