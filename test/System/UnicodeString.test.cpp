#include "SFML/System/UnicodeString.hpp"

#include "SFML/System/UnicodeStringUtfUtils.hpp"

#include "SFML/Base/Assert.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <StringifyStdStringUtil.hpp>

#include <sstream>
#include <string>


namespace
{
// Return either argument depending on whether wchar_t is 16 or 32 bits
// Lets us write tests that work on both Windows where wchar_t is 16 bits
// and elsewhere where it is 32. Otherwise the tests would only work on
// one OS or the other.
template <typename T>
auto selectAnsi(const std::basic_string<T>& stringWin, const std::basic_string<T>& stringUnix)
{
    SFML_BASE_ASSERT(stringWin != stringUnix && "Invalid to select between identical inputs");
#if defined(_WIN32) // TODO P1: fails under CLANG64 env
    return stringWin;
#else
    return stringUnix;
#endif
}

// Return either argument depending on whether wchar_t is 16 or 32 bits.
// Lets us write tests that work on both Windows (MSVC) where wchar_t is 16 bits
// and other platforms where it is 32 bits.
template <typename T>
auto selectWide(const std::basic_string<T>& string16, const std::basic_string<T>& string32)
{
    SFML_BASE_ASSERT(string16 != string32 && "Invalid to select between identical inputs");
    if constexpr (sizeof(wchar_t) == 2)
        return string16;
    else
        return string32;
}

auto toHex(const char32_t character)
{
    std::ostringstream stream;
    stream << "[\\x" << std::uppercase << std::hex << static_cast<sf::base::U32>(character) << ']';
    return stream.str();
}

} // namespace

// Specialize StringMaker for alternative std::basic_string<T> specializations
// std::string's string conversion cannot be specialized but all other string types get special treatment
// https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md#catchstringmaker-specialisation
namespace doctest
{
template <>
struct StringMaker<std::u8string>
{
    static doctest::String convert(const std::u8string& string)
    {
        std::ostringstream output;
        for (const auto character : string)
        {
            if (character >= 32 && character < 127)
                output << std::string(1, static_cast<char>(character));
            else
                output << toHex(static_cast<char32_t>(character));
        }

        const auto result = output.str();
        return {result.c_str(), static_cast<doctest::String::size_type>(result.size())};
    }
};
} // namespace doctest

TEST_CASE("[System] sf::UnicodeString")
{
    using namespace std::string_literals;

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::UnicodeString));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::UnicodeString));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::UnicodeString));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::UnicodeString));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const sf::UnicodeString string;
            CHECK(string.toAnsiString<std::string>().empty());
            CHECK(string.toWideString<std::wstring>().empty());
            CHECK(string.toUtf8<std::u8string>().empty());
            CHECK(string.toUtf16<std::u16string>().empty());
            CHECK(string.toUtf32<std::u32string>().empty());
            CHECK(string.getSize() == 0);
            CHECK(string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("ANSI character constructor")
        {
            const sf::UnicodeString string = 'a';
            CHECK(string.toAnsiString<std::string>() == "a"s);
            CHECK(string.toWideString<std::wstring>() == L"a"s);
            CHECK(string.toUtf8<std::u8string>() == std::u8string{'a'});
            CHECK(string.toUtf16<std::u16string>() == u"a"s);
            CHECK(string.toUtf32<std::u32string>() == U"a"s);
            CHECK(string.getSize() == 1);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("ANSI C string constructor")
        {
            const sf::UnicodeString string = "def";
            CHECK(string.toAnsiString<std::string>() == "def"s);
            CHECK(string.toWideString<std::wstring>() == L"def"s);
            CHECK(string.toUtf8<std::u8string>() == std::u8string{'d', 'e', 'f'});
            CHECK(string.toUtf16<std::u16string>() == u"def"s);
            CHECK(string.toUtf32<std::u32string>() == U"def"s);
            CHECK(string.getSize() == 3);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("ANSI string constructor")
        {
            const sf::UnicodeString string = "ghi"s;
            CHECK(string.toAnsiString<std::string>() == "ghi"s);
            CHECK(string.toWideString<std::wstring>() == L"ghi"s);
            CHECK(string.toUtf8<std::u8string>() == std::u8string{'g', 'h', 'i'});
            CHECK(string.toUtf16<std::u16string>() == u"ghi"s);
            CHECK(string.toUtf32<std::u32string>() == U"ghi"s);
            CHECK(string.getSize() == 3);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("Wide character constructor")
        {
            const sf::UnicodeString string = L'\xFA';
            CHECK(string.toAnsiString<std::string>() == selectAnsi("\xFA"s, "\0"s));
            CHECK(string.toWideString<std::wstring>() == L"\xFA"s);
            CHECK(string.toUtf8<std::u8string>() == std::u8string{0xC3, 0xBA});
            CHECK(string.toUtf16<std::u16string>() == u"\xFA"s);
            CHECK(string.toUtf32<std::u32string>() == U"\xFA"s);
            CHECK(string.getSize() == 1);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("Wide C string constructor")
        {
            const sf::UnicodeString string = L"j\xFAl";
            CHECK(string.toAnsiString<std::string>() == selectAnsi("j\xFAl"s, "j\0l"s));
            CHECK(string.toWideString<std::wstring>() == L"j\xFAl"s);
            CHECK(string.toUtf8<std::u8string>() == std::u8string{'j', 0xC3, 0xBA, 'l'});
            CHECK(string.toUtf16<std::u16string>() == u"j\xFAl"s);
            CHECK(string.toUtf32<std::u32string>() == U"j\xFAl"s);
            CHECK(string.getSize() == 3);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("Wide string constructor")
        {
            const sf::UnicodeString string = L"mno\xFA"s;
            CHECK(string.toAnsiString<std::string>() == selectAnsi("mno\xFA"s, "mno\0"s));
            CHECK(string.toWideString<std::wstring>() == L"mno\xFA"s);
            CHECK(string.toUtf8<std::u8string>() == std::u8string{'m', 'n', 'o', 0xC3, 0XBA});
            CHECK(string.toUtf16<std::u16string>() == u"mno\xFA"s);
            CHECK(string.toUtf32<std::u32string>() == U"mno\xFA"s);
            CHECK(string.getSize() == 4);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("UTF-32 character constructor")
        {
            const sf::UnicodeString string = U'\U0010AFAF';
            CHECK(string.toAnsiString<std::string>() == "\0"s);
            CHECK(string.toWideString<std::wstring>() == selectWide(L""s, L"\U0010AFAF"s));
            CHECK(string.toUtf8<std::u8string>() == std::u8string{0xF4, 0x8A, 0xBE, 0xAF});
            CHECK(string.toUtf16<std::u16string>() == u"\U0010AFAF"s);
            CHECK(string.toUtf32<std::u32string>() == U"\U0010AFAF"s);
            CHECK(string.getSize() == 1);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("UTF-32 C string constructor")
        {
            const sf::UnicodeString string = U"\U0010ABCDrs";
            CHECK(string.toAnsiString<std::string>() == "\0rs"s);
            CHECK(string.toWideString<std::wstring>() == selectWide(L"rs"s, L"\U0010ABCDrs"s));
            CHECK(string.toUtf8<std::u8string>() == std::u8string{0xF4, 0x8A, 0xAF, 0x8D, 'r', 's'});
            CHECK(string.toUtf16<std::u16string>() == u"\U0010ABCDrs"s);
            CHECK(string.toUtf32<std::u32string>() == U"\U0010ABCDrs"s);
            CHECK(string.getSize() == 3);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("UTF-32 string constructor")
        {
            const sf::UnicodeString string = U"tuv\U00104321"s;
            CHECK(string.toAnsiString<std::string>() == "tuv\0"s);
            CHECK(string.toWideString<std::wstring>() == selectWide(L"tuv"s, L"tuv\U00104321"s));
            CHECK(string.toUtf8<std::u8string>() == std::u8string{'t', 'u', 'v', 0xF4, 0x84, 0x8C, 0xA1});
            CHECK(string.toUtf16<std::u16string>() == u"tuv\U00104321"s);
            CHECK(string.toUtf32<std::u32string>() == U"tuv\U00104321"s);
            CHECK(string.getSize() == 4);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }
    }

    SECTION("fromUtf8()")
    {
        SECTION("Nominal")
        {
            constexpr sf::base::U8  characters[4]{'w', 'x', 'y', 'z'};
            const sf::UnicodeString string = sf::UnicodeStringUtfUtils::fromUtf8(characters, characters + 4);
            CHECK(string.toAnsiString<std::string>() == "wxyz"s);
            CHECK(string.toWideString<std::wstring>() == L"wxyz"s);
            CHECK(string.toUtf8<std::u8string>() == std::u8string{'w', 'x', 'y', 'z'});
            CHECK(string.toUtf16<std::u16string>() == u"wxyz"s);
            CHECK(string.toUtf32<std::u32string>() == U"wxyz"s);
            CHECK(string.getSize() == 4);
            CHECK(!string.isEmpty());
            CHECK(string.getData() != nullptr);
        }

        SECTION("Insufficient input")
        {
            constexpr sf::base::U8  characters[1]{251};
            const sf::UnicodeString string = sf::UnicodeStringUtfUtils::fromUtf8(characters, characters + 1);
            constexpr char32_t      defaultReplacementCharacter = 0;
            CHECK(string.getSize() == 1);
            CHECK(string[0] == defaultReplacementCharacter);
        }
    }

    SECTION("fromUtf16()")
    {
        constexpr char16_t      characters[4]{0xF1, 'x', 'y', 'z'};
        const sf::UnicodeString string = sf::UnicodeStringUtfUtils::fromUtf16(characters, characters + 4);
        CHECK(string.toAnsiString<std::string>() == selectAnsi("\xF1xyz"s, "\0xyz"s));
        CHECK(string.toWideString<std::wstring>() == L"\xF1xyz"s);
        CHECK(string.toUtf8<std::u8string>() == std::u8string{0xC3, 0xB1, 'x', 'y', 'z'});
        CHECK(string.toUtf16<std::u16string>() == u"\xF1xyz"s);
        CHECK(string.toUtf32<std::u32string>() == U"\xF1xyz"s);
        CHECK(string.getSize() == 4);
        CHECK(!string.isEmpty());
        CHECK(string.getData() != nullptr);
    }

    SECTION("fromUtf32()")
    {
        constexpr char32_t      characters[4]{'w', 0x10'43'21, 'y', 'z'};
        const sf::UnicodeString string = sf::UnicodeStringUtfUtils::fromUtf32(characters, characters + 4);
        CHECK(string.toAnsiString<std::string>() == "w\0yz"s);
        CHECK(string.toWideString<std::wstring>() == selectWide(L"wyz"s, L"w\U00104321yz"s));
        CHECK(string.toUtf8<std::u8string>() == std::u8string{'w', 0xF4, 0x84, 0x8C, 0xA1, 'y', 'z'});
        CHECK(string.toUtf16<std::u16string>() == u"w\U00104321yz"s);
        CHECK(string.toUtf32<std::u32string>() == U"w\U00104321yz"s);
        CHECK(string.getSize() == 4);
        CHECK(!string.isEmpty());
        CHECK(string.getData() != nullptr);
    }

    SECTION("clear()")
    {
        sf::UnicodeString string("you'll never guess what happens when you call clear()");
        string.clear();
        CHECK(string.isEmpty());
        CHECK(string.getSize() == 0);
    }

    SECTION("erase()")
    {
        sf::UnicodeString string("what if i want a shorter string?");
        string.erase(0, 8);
        string.erase(string.getSize() - 1, 1);
        CHECK(string == "i want a shorter string");
        CHECK(string.getSize() == 23);
    }

    SECTION("insert()")
    {
        sf::UnicodeString string("please insert text");
        string.insert(7, "don't ");
        CHECK(string == "please don't insert text");
        CHECK(string.getSize() == 24);
    }

    SECTION("find()")
    {
        const sf::UnicodeString string("a little bit of this and a little bit of that");
        CHECK(string.find("a little bit") == 0);
        CHECK(string.find("a little bit", 15) == 25);
        CHECK(string.find("a little bit", 1000) == sf::UnicodeString::InvalidPos);
        CHECK(string.find("no way you find this") == sf::UnicodeString::InvalidPos);
    }

    SECTION("replace()")
    {
        sf::UnicodeString string("sfml is the worst");
        string.replace(12, 5, "best!");
        CHECK(string == "sfml is the best!");
        string.replace("the", "THE");
        CHECK(string == "sfml is THE best!");
    }

    SECTION("substring()")
    {
        const sf::UnicodeString string("let's get some substrings");
        CHECK(string.substring(0) == "let's get some substrings");
        CHECK(string.substring(10) == "some substrings");
        CHECK(string.substring(10, 4) == "some");

// Exceptions not supported on Emscripten
#ifndef SFML_SYSTEM_EMSCRIPTEN
        CHECK_THROWS_AS((void)string.substring(1000), std::out_of_range);
        CHECK_THROWS_AS((void)string.substring(420, 69), std::out_of_range);
#endif
    }

    SECTION("begin() and end() const")
    {
        const sf::UnicodeString string("let's test the const iterators");
        CHECK(*string.begin() == 'l');
        CHECK(*(string.end() - 1) == 's');
        for (const auto character : string)
            CHECK(character != 0);
    }

    SECTION("begin() and end()")
    {
        sf::UnicodeString string("let's test the iterators");
        CHECK(*string.begin() == 'l');
        CHECK(*(string.end() - 1) == 's');
        for (auto& character : string)
            character = 'x';
        CHECK(string == "xxxxxxxxxxxxxxxxxxxxxxxx");
    }

    SECTION("Operators")
    {
        SECTION("operator+=")
        {
            sf::UnicodeString string;
            string += sf::UnicodeString("xyz");
            CHECK(string.toAnsiString<std::string>() == "xyz"s);
        }

        SECTION("operator[] const")
        {
            const sf::UnicodeString string("the quick brown fox");
            CHECK(string[0] == 't');
            CHECK(string[10] == 'b');
        }

        SECTION("operator[]")
        {
            sf::UnicodeString string("the quick brown fox");
            CHECK(string[0] == 't');
            string[1] = 'x';
            CHECK(string[1] == 'x');
        }

        SECTION("operator==")
        {
            CHECK(sf::UnicodeString() == sf::UnicodeString());
            CHECK_FALSE(sf::UnicodeString() == sf::UnicodeString(' '));
        }

        SECTION("operator!=")
        {
            CHECK(sf::UnicodeString() != sf::UnicodeString(' '));
            CHECK_FALSE(sf::UnicodeString() != sf::UnicodeString());
        }

        SECTION("operator<")
        {
            CHECK(sf::UnicodeString('a') < sf::UnicodeString('b'));
            CHECK_FALSE(sf::UnicodeString() < sf::UnicodeString());
        }

        SECTION("operator>")
        {
            CHECK(sf::UnicodeString('b') > sf::UnicodeString('a'));
            CHECK_FALSE(sf::UnicodeString() > sf::UnicodeString());
        }

        SECTION("operator<=")
        {
            CHECK(sf::UnicodeString() <= sf::UnicodeString());
            CHECK(sf::UnicodeString('a') <= sf::UnicodeString('b'));
        }

        SECTION("operator>=")
        {
            CHECK(sf::UnicodeString() >= sf::UnicodeString());
            CHECK(sf::UnicodeString('b') >= sf::UnicodeString('a'));
        }

        SECTION("operator+")
        {
            CHECK(sf::UnicodeString() + sf::UnicodeString() == sf::UnicodeString());
            CHECK(sf::UnicodeString("abc") + sf::UnicodeString("def") == sf::UnicodeString("abcdef"));
        }
    }
}
