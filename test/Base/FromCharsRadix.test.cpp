#include "Tst/Tst.hpp"

#include "SFML/Base/FromCharsRadix.hpp"

#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/ToCharsRadix.hpp"

#include <initializer_list>


////////////////////////////////////////////////////////////
namespace
{
template <typename T>
[[nodiscard]] sf::base::FromCharsResult parse(const char* s, T& value, const sf::base::Radix radix)
{
    const char* const last = s + SFML_BASE_STRLEN(s);
    return sf::base::fromCharsRadix(s, last, value, radix);
}
} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - basic parses")
{
    SECTION("Hex (lowercase)")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("abcd", v, sf::base::Radix::Hex);
        CHECK(v == 0xAB'CDu);
        CHECK(r.ec == sf::base::FromCharsError::None);
        CHECK(*r.ptr == '\0'); // Consumed entire input.
    }

    SECTION("Hex (uppercase)")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("DEADBEEF", v, sf::base::Radix::Hex);
        CHECK(v == 0xDE'AD'BE'EFu);
        CHECK(r.ec == sf::base::FromCharsError::None);
    }

    SECTION("Hex (mixed case)")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("AbCd", v, sf::base::Radix::Hex);
        CHECK(v == 0xAB'CDu);
        CHECK(r.ec == sf::base::FromCharsError::None);
    }

    SECTION("Octal")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("777", v, sf::base::Radix::Oct);
        CHECK(v == 511u);
        CHECK(r.ec == sf::base::FromCharsError::None);
    }

    SECTION("Binary")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("11111111", v, sf::base::Radix::Bin);
        CHECK(v == 0xFFu);
        CHECK(r.ec == sf::base::FromCharsError::None);
    }

    SECTION("Zero")
    {
        sf::base::U32 v = 1234u;
        const auto    r = parse("0", v, sf::base::Radix::Hex);
        CHECK(v == 0u);
        CHECK(r.ec == sf::base::FromCharsError::None);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - stops at first non-digit")
{
    SECTION("Hex stops at non-hex char")
    {
        const char    s[] = "ffXY";
        sf::base::U32 v   = 0u;
        const auto    r   = sf::base::fromCharsRadix(s, s + 4, v, sf::base::Radix::Hex);
        CHECK(v == 0xFFu);
        CHECK(r.ec == sf::base::FromCharsError::None);
        CHECK(r.ptr == s + 2); // Stopped at 'X'.
    }

    SECTION("Octal stops at out-of-radix digit '8'")
    {
        const char    s[] = "78";
        sf::base::U32 v   = 0u;
        const auto    r   = sf::base::fromCharsRadix(s, s + 2, v, sf::base::Radix::Oct);
        CHECK(v == 7u);
        CHECK(r.ec == sf::base::FromCharsError::None);
        CHECK(r.ptr == s + 1);
    }

    SECTION("Binary stops at out-of-radix digit '2'")
    {
        const char    s[] = "1012";
        sf::base::U32 v   = 0u;
        const auto    r   = sf::base::fromCharsRadix(s, s + 4, v, sf::base::Radix::Bin);
        CHECK(v == 0b101u);
        CHECK(r.ec == sf::base::FromCharsError::None);
        CHECK(r.ptr == s + 3);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - failure modes")
{
    SECTION("Empty input is InvalidArgument")
    {
        sf::base::U32 v = 0u;
        const auto    r = sf::base::fromCharsRadix(nullptr, nullptr, v, sf::base::Radix::Hex);
        CHECK(r.ec == sf::base::FromCharsError::InvalidArgument);
    }

    SECTION("No valid digits is InvalidArgument")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("zzz", v, sf::base::Radix::Hex);
        CHECK(r.ec == sf::base::FromCharsError::InvalidArgument);
    }

    SECTION("Leading sign is not accepted")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("-1", v, sf::base::Radix::Hex);
        CHECK(r.ec == sf::base::FromCharsError::InvalidArgument);
    }

    SECTION("Overflow on a too-large hex value")
    {
        sf::base::U8 v = 0u;
        const auto   r = parse("100", v, sf::base::Radix::Hex); // 0x100 doesn't fit in U8
        CHECK(r.ec == sf::base::FromCharsError::ResultOutOfRange);
    }

    SECTION("Maximum value fits exactly")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("ffffffff", v, sf::base::Radix::Hex);
        CHECK(v == 0xFF'FF'FF'FFu);
        CHECK(r.ec == sf::base::FromCharsError::None);
    }

    SECTION("Just-over-maximum is rejected")
    {
        sf::base::U32 v = 0u;
        const auto    r = parse("100000000", v, sf::base::Radix::Hex);
        CHECK(r.ec == sf::base::FromCharsError::ResultOutOfRange);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - round-trip with toCharsRadix")
{
    // Sanity: parse what we format. Decoupled, but the two halves should agree.
    for (sf::base::U32 v : {0u, 1u, 0xFu, 0x10u, 0xAB'CDu, 0xDE'AD'BE'EFu, 0xFF'FF'FF'FFu})
    {
        for (auto radix : {sf::base::Radix::Bin, sf::base::Radix::Oct, sf::base::Radix::Hex})
        {
            char        buf[64]{};
            const auto* end = sf::base::toCharsRadix(buf, buf + sizeof(buf), v, radix);
            REQUIRE(end != nullptr);

            sf::base::U32 parsed = 0u;
            const auto    r      = sf::base::fromCharsRadix(buf, end, parsed, radix);
            CHECK(r.ec == sf::base::FromCharsError::None);
            CHECK(r.ptr == end);
            CHECK(parsed == v);
        }
    }
}
