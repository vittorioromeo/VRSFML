#include "StringifyStdStringUtil.hpp"     // IWYU pragma: keep
#include "StringifyStdStringViewUtil.hpp" // IWYU pragma: keep
#include "Tst/Tst.hpp"

#include "Zancle/System/Utf.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/BackInserter.hpp"

#include <locale>
#include <string_view>


namespace
{
////////////////////////////////////////////////////////////
// Return either argument depending on whether wchar_t is 16 or 32 bits
// Lets us write tests that work on both Windows where wchar_t is 16 bits
// and elsewhere where it is 32. Otherwise the tests would only work on
// one OS or the other.
template <typename T>
auto select(const std::basic_string_view<T>& string16, const std::basic_string_view<T>& string32)
{
    ZB_ASSERT(string16 != string32 && "Invalid to select between identical inputs");
    if constexpr (sizeof(wchar_t) == 2)
        return string16;
    else
        return string32;
}

////////////////////////////////////////////////////////////
const auto& getFacet()
{
    return std::use_facet<std::ctype<wchar_t>>(std::locale{});
}

} // namespace

using namespace std::string_view_literals;

// Create C++17-compatible aliases for std::u8string{_view}
using u8string      = std::basic_string<decltype(u8' ')>;
using u8string_view = std::basic_string_view<decltype(u8' ')>;

// NOLINTBEGIN(readability-qualified-auto)

TEST_CASE("[System] za::Utf8")
{
    static constexpr auto utf8 = u8"SFML 🐌"sv;

    SECTION("decode")
    {
        std::u32string output;
        for (auto begin = utf8.cbegin(); begin < utf8.cend();)
        {
            char32_t character = 0;
            begin              = za::Utf8::decode(begin, utf8.cend(), character, 0);
            output.push_back(character);
        }
        CHECK(output == U"SFML 🐌"sv);
    }

    SECTION("encode")
    {
        u8string output;

        SECTION("Default replacement character")
        {
            za::Utf8::encode(U' ', zb::BackInserter(output), 0);
            CHECK(output == u8" "sv);
            za::Utf8::encode(U'🐌', zb::BackInserter(output), 0);
            CHECK(output == u8" 🐌"sv);
            za::Utf8::encode(0xFF'FF'FF'FF, zb::BackInserter(output), 0);
            CHECK(output == u8" 🐌"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf8::encode(U' ', zb::BackInserter(output), '?');
            CHECK(output == u8" "sv);
            za::Utf8::encode(U'🐌', zb::BackInserter(output), '?');
            CHECK(output == u8" 🐌"sv);
            za::Utf8::encode(0xFF'FF'FF'FF, zb::BackInserter(output), '?');
            CHECK(output == u8" 🐌?"sv);
        }
    }

    SECTION("next")
    {
        auto next = utf8.cbegin();
        CHECK(*next == u8'S');
        next = za::Utf8::next(next, utf8.cend());
        CHECK(*next == u8'F');
        next = za::Utf8::next(next, utf8.cend());
        CHECK(*next == u8'M');
        next = za::Utf8::next(next, utf8.cend());
        CHECK(*next == u8'L');
        next = za::Utf8::next(next, utf8.cend());
        CHECK(*next == u8' ');
        next = za::Utf8::next(next, utf8.cend());
        CHECK(u8string_view(&*next, 4) == u8"🐌"sv);
        next = za::Utf8::next(next, utf8.cend());
        CHECK((next == utf8.cend()));
    }

    SECTION("count")
    {
        REQUIRE(utf8.size() == 9);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cend()) == 6);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 9) == 6);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 8) == 6);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 7) == 6);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 6) == 6);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 5) == 5);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 4) == 4);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 3) == 3);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 2) == 2);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin() + 1) == 1);
        CHECK(za::Utf8::count(utf8.cbegin(), utf8.cbegin()) == 0);
    }

    SECTION("fromAnsi")
    {
        static constexpr auto ansi = "abcdefg"sv;
        u8string              output;
        za::Utf8::fromAnsi(ansi.cbegin(), ansi.cend(), zb::BackInserter(output), getFacet());
        CHECK(output == u8"abcdefg"sv);
    }

    SECTION("fromWide")
    {
        static constexpr auto wide = L"abçdéfgń"sv;
        u8string              output;
        za::Utf8::fromWide(wide.cbegin(), wide.cend(), zb::BackInserter(output));
        CHECK(output == u8"abçdéfgń"sv);
    }

    SECTION("fromLatin1")
    {
        static constexpr auto latin1 =
            "\xA1"
            "ab\xE7"
            "d\xE9!"sv;
        u8string output;
        za::Utf8::fromLatin1(latin1.cbegin(), latin1.cend(), zb::BackInserter(output));
        CHECK(output == u8"¡abçdé!"sv);
    }

    SECTION("toAnsi")
    {
        std::string output;

        SECTION("Default replacement character")
        {
            za::Utf8::toAnsi(utf8.cbegin(), utf8.cend(), zb::BackInserter(output), 0, getFacet());
            CHECK(output == "SFML \0"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf8::toAnsi(utf8.cbegin(), utf8.cend(), zb::BackInserter(output), '_', getFacet());
            CHECK(output == "SFML _"sv);
        }
    }

    SECTION("toWide")
    {
        std::wstring output;

        SECTION("Default replacement character")
        {
            za::Utf8::toWide(utf8.cbegin(), utf8.cend(), zb::BackInserter(output), 0);
            CHECK(output == select(L"SFML "sv, L"SFML 🐌"sv));
        }

        SECTION("Custom replacement character")
        {
            za::Utf8::toWide(utf8.cbegin(), utf8.cend(), zb::BackInserter(output), L'_');
            CHECK(output == select(L"SFML _"sv, L"SFML 🐌"sv));
        }
    }

    SECTION("toLatin1")
    {
        std::string output;

        SECTION("Default replacement character")
        {
            za::Utf8::toLatin1(utf8.cbegin(), utf8.cend(), zb::BackInserter(output), 0);
            CHECK(output == "SFML \0"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf8::toLatin1(utf8.cbegin(), utf8.cend(), zb::BackInserter(output), '_');
            CHECK(output == "SFML _"sv);
        }
    }

    SECTION("toUtf8")
    {
        u8string output;
        za::Utf8::toUtf8(utf8.cbegin(), utf8.cend(), zb::BackInserter(output));
        CHECK(output == utf8);
    }

    SECTION("toUtf16")
    {
        std::u16string output;
        za::Utf8::toUtf16(utf8.cbegin(), utf8.cend(), zb::BackInserter(output));
        CHECK(output == u"SFML 🐌"sv);
    }

    SECTION("toUtf32")
    {
        std::u32string output;
        za::Utf8::toUtf32(utf8.cbegin(), utf8.cend(), zb::BackInserter(output));
        CHECK(output == U"SFML 🐌"sv);
    }
}

TEST_CASE("[System] za::Utf16")
{
    static constexpr auto utf16 = u"SFML 🐌"sv;

    SECTION("decode")
    {
        std::u32string output;
        for (auto begin = utf16.cbegin(); begin < utf16.cend();)
        {
            char32_t character = 0;
            begin              = za::Utf16::decode(begin, utf16.cend(), character, 0);
            output.push_back(character);
        }
        CHECK(output == U"SFML 🐌"sv);
    }

    SECTION("encode")
    {
        std::u16string output;

        SECTION("Default replacement character")
        {
            za::Utf16::encode(U' ', zb::BackInserter(output), 0);
            CHECK(output == u" "sv);
            za::Utf16::encode(U'🐌', zb::BackInserter(output), 0);
            CHECK(output == u" 🐌"sv);
            za::Utf16::encode(0xFF'FF'FF'FF, zb::BackInserter(output), 0);
            CHECK(output == u" 🐌"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf16::encode(U' ', zb::BackInserter(output), '?');
            CHECK(output == u" "sv);
            za::Utf16::encode(U'🐌', zb::BackInserter(output), '?');
            CHECK(output == u" 🐌"sv);
            za::Utf16::encode(0xFF'FF'FF'FF, zb::BackInserter(output), '?');
            CHECK(output == u" 🐌?"sv);
        }
    }

    SECTION("next")
    {
        auto next = utf16.cbegin();
        CHECK(*next == u'S');
        next = za::Utf16::next(next, utf16.cend());
        CHECK(*next == u'F');
        next = za::Utf16::next(next, utf16.cend());
        CHECK(*next == u'M');
        next = za::Utf16::next(next, utf16.cend());
        CHECK(*next == u'L');
        next = za::Utf16::next(next, utf16.cend());
        CHECK(*next == u' ');
        next = za::Utf16::next(next, utf16.cend());
        CHECK(std::u16string_view(&*next, 2) == u"🐌"sv);
        next = za::Utf16::next(next, utf16.cend());
        CHECK((next == utf16.cend()));
    }

    SECTION("count")
    {
        REQUIRE(utf16.size() == 7);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cend()) == 6);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin() + 7) == 6);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin() + 6) == 6);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin() + 5) == 5);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin() + 4) == 4);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin() + 3) == 3);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin() + 2) == 2);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin() + 1) == 1);
        CHECK(za::Utf16::count(utf16.cbegin(), utf16.cbegin()) == 0);
    }

    SECTION("fromAnsi")
    {
        static constexpr auto ansi = "abcdefg"sv;
        std::u16string        output;
        za::Utf16::fromAnsi(ansi.cbegin(), ansi.cend(), zb::BackInserter(output), getFacet());
        CHECK(output == u"abcdefg"sv);
    }

    SECTION("fromWide")
    {
        static constexpr auto wide = L"abçdéfgń"sv;
        std::u16string        output;
        za::Utf16::fromWide(wide.cbegin(), wide.cend(), zb::BackInserter(output));
        CHECK(output == u"abçdéfgń"sv);
    }

    SECTION("fromLatin1")
    {
        static constexpr auto latin1 =
            "\xA1"
            "ab\xE7"
            "d\xE9!"sv;
        std::u16string output;
        za::Utf16::fromLatin1(latin1.cbegin(), latin1.cend(), zb::BackInserter(output));
        CHECK(output == u"¡abçdé!"sv);
    }

    SECTION("toAnsi")
    {
        std::string output;

        SECTION("Default replacement character")
        {
            za::Utf16::toAnsi(utf16.cbegin(), utf16.cend(), zb::BackInserter(output), 0, getFacet());
            CHECK(output == "SFML \0"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf16::toAnsi(utf16.cbegin(), utf16.cend(), zb::BackInserter(output), '_', getFacet());
            CHECK(output == "SFML _"sv);
        }
    }

    SECTION("toWide")
    {
        std::wstring output;

        SECTION("Default replacement character")
        {
            za::Utf16::toWide(utf16.cbegin(), utf16.cend(), zb::BackInserter(output), 0);
            CHECK(output == select(L"SFML "sv, L"SFML 🐌"sv));
        }

        SECTION("Custom replacement character")
        {
            za::Utf16::toWide(utf16.cbegin(), utf16.cend(), zb::BackInserter(output), '_');
            CHECK(output == select(L"SFML _"sv, L"SFML 🐌"sv));
        }
    }

    SECTION("toLatin1")
    {
        std::string output;

        SECTION("Default replacement character")
        {
            za::Utf16::toLatin1(utf16.cbegin(), utf16.cend(), zb::BackInserter(output), 0);
            CHECK(output == "SFML \0\0"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf16::toLatin1(utf16.cbegin(), utf16.cend(), zb::BackInserter(output), '_');
            CHECK(output == "SFML __"sv);
        }
    }

    SECTION("toUtf8")
    {
        u8string output;
        za::Utf16::toUtf8(utf16.cbegin(), utf16.cend(), zb::BackInserter(output));
        CHECK(output == u8"SFML 🐌"sv);
    }

    SECTION("toUtf16")
    {
        std::u16string output;
        za::Utf16::toUtf16(utf16.cbegin(), utf16.cend(), zb::BackInserter(output));
        CHECK(output == utf16);
    }

    SECTION("toUtf32")
    {
        std::u32string output;
        za::Utf16::toUtf32(utf16.cbegin(), utf16.cend(), zb::BackInserter(output));
        CHECK(output == U"SFML 🐌"sv);
    }
}

TEST_CASE("[System] za::Utf32")
{
    static constexpr auto utf32 = U"SFML 🐌"sv;

    SECTION("decode")
    {
        std::u32string output;
        for (auto begin = utf32.cbegin(); begin < utf32.cend();)
        {
            char32_t character = 0;
            begin              = za::Utf32::decode(begin, {}, character, 0);
            output.push_back(character);
        }
        CHECK(output == utf32);
    }

    SECTION("encode")
    {
        std::u32string output;
        for (const auto character : utf32)
            za::Utf32::encode(character, zb::BackInserter(output), 0);
        CHECK(output == utf32);
    }

    SECTION("next")
    {
        auto next = utf32.cbegin();
        CHECK(*next == U'S');
        next = za::Utf32::next(next, utf32.cend());
        CHECK(*next == U'F');
        next = za::Utf32::next(next, utf32.cend());
        CHECK(*next == U'M');
        next = za::Utf32::next(next, utf32.cend());
        CHECK(*next == U'L');
        next = za::Utf32::next(next, utf32.cend());
        CHECK(*next == U' ');
        next = za::Utf32::next(next, utf32.cend());
        CHECK(*next == U'🐌');
        next = za::Utf32::next(next, utf32.cend());
        CHECK((next == utf32.cend()));
    }

    SECTION("count")
    {
        REQUIRE(utf32.size() == 6);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cend()) == 6);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cbegin() + 6) == 6);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cbegin() + 5) == 5);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cbegin() + 4) == 4);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cbegin() + 3) == 3);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cbegin() + 2) == 2);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cbegin() + 1) == 1);
        CHECK(za::Utf32::count(utf32.cbegin(), utf32.cbegin()) == 0);
    }

    SECTION("fromAnsi")
    {
        static constexpr auto ansi = "abcdefg"sv;
        std::u32string        output;
        za::Utf32::fromAnsi(ansi.cbegin(), ansi.cend(), zb::BackInserter(output), getFacet());
        CHECK(output == U"abcdefg"sv);
    }

    SECTION("fromWide")
    {
        static constexpr auto wide = L"abçdéfgń"sv;
        std::u32string        output;
        za::Utf32::fromWide(wide.cbegin(), wide.cend(), zb::BackInserter(output));
        CHECK(output == U"abçdéfgń"sv);
    }

    SECTION("fromLatin1")
    {
        static constexpr auto latin1 =
            "\xA1"
            "ab\xE7"
            "d\xE9!"sv;
        std::u32string output;
        za::Utf32::fromLatin1(latin1.cbegin(), latin1.cend(), zb::BackInserter(output));
        CHECK(output == U"¡abçdé!"sv);
    }

    SECTION("toAnsi")
    {
        std::string output;

        SECTION("Default replacement character")
        {
            za::Utf32::toAnsi(utf32.cbegin(), utf32.cend(), zb::BackInserter(output), 0, getFacet());
            CHECK(output == "SFML \0"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf32::toAnsi(utf32.cbegin(), utf32.cend(), zb::BackInserter(output), '_', getFacet());
            CHECK(output == "SFML _"sv);
        }
    }

    SECTION("toWide")
    {
        std::wstring output;

        SECTION("Default replacement character")
        {
            za::Utf32::toWide(utf32.cbegin(), utf32.cend(), zb::BackInserter(output), 0);
            CHECK(output == select(L"SFML "sv, L"SFML 🐌"sv));
        }

        SECTION("Custom replacement character")
        {
            za::Utf32::toWide(utf32.cbegin(), utf32.cend(), zb::BackInserter(output), L'_');
            CHECK(output == select(L"SFML _"sv, L"SFML 🐌"sv));
        }
    }

    SECTION("toLatin1")
    {
        std::string output;

        SECTION("Default replacement character")
        {
            za::Utf32::toLatin1(utf32.cbegin(), utf32.cend(), zb::BackInserter(output), 0);
            CHECK(output == "SFML \0"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf32::toLatin1(utf32.cbegin(), utf32.cend(), zb::BackInserter(output), '_');
            CHECK(output == "SFML _"sv);
        }
    }

    SECTION("toUtf8")
    {
        u8string output;
        za::Utf32::toUtf8(utf32.cbegin(), utf32.cend(), zb::BackInserter(output));
        CHECK(output == u8"SFML 🐌"sv);
    }

    SECTION("toUtf16")
    {
        std::u16string output;
        za::Utf32::toUtf16(utf32.cbegin(), utf32.cend(), zb::BackInserter(output));
        CHECK(output == u"SFML 🐌"sv);
    }

    SECTION("toUtf32")
    {
        std::u32string output;
        za::Utf32::toUtf32(utf32.cbegin(), utf32.cend(), zb::BackInserter(output));
        CHECK(output == utf32);
    }

    SECTION("decodeAnsi")
    {
        CHECK(za::Utf32::decodeAnsi('\0', getFacet()) == U'\0');
        CHECK(za::Utf32::decodeAnsi(' ', getFacet()) == U' ');
        CHECK(za::Utf32::decodeAnsi('a', getFacet()) == U'a');
        CHECK(za::Utf32::decodeAnsi('A', getFacet()) == U'A');
    }

    SECTION("decodeWide")
    {
        CHECK(za::Utf32::decodeWide(L'\0') == U'\0');
        CHECK(za::Utf32::decodeWide(L' ') == U' ');
        CHECK(za::Utf32::decodeWide(L'a') == U'a');
        CHECK(za::Utf32::decodeWide(L'A') == U'A');
        CHECK(za::Utf32::decodeWide(L'é') == U'é');
        CHECK(za::Utf32::decodeWide(L'ń') == U'ń');
    }

    SECTION("encodeAnsi")
    {
        std::string output;

        SECTION("Default replacement character")
        {
            za::Utf32::encodeAnsi(U' ', zb::BackInserter(output), 0, getFacet());
            CHECK(output == " "sv);
            za::Utf32::encodeAnsi(U'_', zb::BackInserter(output), 0, getFacet());
            CHECK(output == " _"sv);
            za::Utf32::encodeAnsi(U'a', zb::BackInserter(output), 0, getFacet());
            CHECK(output == " _a"sv);
            za::Utf32::encodeAnsi(U'🐌', zb::BackInserter(output), 0, getFacet());
            CHECK(output == " _a\0"sv);
        }

        SECTION("Custom replacement character")
        {
            za::Utf32::encodeAnsi(U' ', zb::BackInserter(output), '?', getFacet());
            CHECK(output == " "sv);
            za::Utf32::encodeAnsi(U'_', zb::BackInserter(output), '?', getFacet());
            CHECK(output == " _"sv);
            za::Utf32::encodeAnsi(U'a', zb::BackInserter(output), '?', getFacet());
            CHECK(output == " _a"sv);
            za::Utf32::encodeAnsi(U'🐌', zb::BackInserter(output), '?', getFacet());
            CHECK(output == " _a?"sv);
        }
    }

    SECTION("encodeWide")
    {
        std::wstring output;

        SECTION("Default replacement character")
        {
            za::Utf32::encodeWide(U' ', zb::BackInserter(output), 0);
            CHECK(output == L" "sv);
            za::Utf32::encodeWide(U'_', zb::BackInserter(output), 0);
            CHECK(output == L" _"sv);
            za::Utf32::encodeWide(U'a', zb::BackInserter(output), 0);
            CHECK(output == L" _a"sv);
            za::Utf32::encodeWide(U'🐌', zb::BackInserter(output), 0);
            CHECK(output == select(L" _a"sv, L" _a🐌"sv));
        }

        SECTION("Custom replacement character")
        {
            za::Utf32::encodeWide(U' ', zb::BackInserter(output), L'?');
            CHECK(output == L" "sv);
            za::Utf32::encodeWide(U'_', zb::BackInserter(output), L'?');
            CHECK(output == L" _"sv);
            za::Utf32::encodeWide(U'a', zb::BackInserter(output), L'?');
            CHECK(output == L" _a"sv);
            za::Utf32::encodeWide(U'🐌', zb::BackInserter(output), L'?');
            CHECK(output == select(L" _a?"sv, L" _a🐌"sv));
        }
    }
}

// NOLINTEND(readability-qualified-auto)
