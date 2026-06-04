#include "Tst/Tst.hpp"
#include "ZancleBase/Builtin/Strlen.hpp"
#include "ZancleBase/FromCharsRadix.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/ToCharsRadix.hpp"

#include <initializer_list>


////////////////////////////////////////////////////////////
namespace
{
template <typename T>
[[nodiscard]] zb::FromCharsResult parse(const char* s, T& value, const zb::Radix radix)
{
    const char* const last = s + ZB_STRLEN(s);
    return zb::fromCharsRadix(s, last, value, radix);
}
} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - basic parses")
{
    SECTION("Hex (lowercase)")
    {
        zb::U32    v = 0u;
        const auto r = parse("abcd", v, zb::Radix::Hex);
        CHECK(v == 0xAB'CDu);
        CHECK(r.ec == zb::FromCharsError::None);
        CHECK(*r.ptr == '\0'); // Consumed entire input.
    }

    SECTION("Hex (uppercase)")
    {
        zb::U32    v = 0u;
        const auto r = parse("DEADBEEF", v, zb::Radix::Hex);
        CHECK(v == 0xDE'AD'BE'EFu);
        CHECK(r.ec == zb::FromCharsError::None);
    }

    SECTION("Hex (mixed case)")
    {
        zb::U32    v = 0u;
        const auto r = parse("AbCd", v, zb::Radix::Hex);
        CHECK(v == 0xAB'CDu);
        CHECK(r.ec == zb::FromCharsError::None);
    }

    SECTION("Octal")
    {
        zb::U32    v = 0u;
        const auto r = parse("777", v, zb::Radix::Oct);
        CHECK(v == 511u);
        CHECK(r.ec == zb::FromCharsError::None);
    }

    SECTION("Binary")
    {
        zb::U32    v = 0u;
        const auto r = parse("11111111", v, zb::Radix::Bin);
        CHECK(v == 0xFFu);
        CHECK(r.ec == zb::FromCharsError::None);
    }

    SECTION("Zero")
    {
        zb::U32    v = 1234u;
        const auto r = parse("0", v, zb::Radix::Hex);
        CHECK(v == 0u);
        CHECK(r.ec == zb::FromCharsError::None);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - stops at first non-digit")
{
    SECTION("Hex stops at non-hex char")
    {
        const char s[] = "ffXY";
        zb::U32    v   = 0u;
        const auto r   = zb::fromCharsRadix(s, s + 4, v, zb::Radix::Hex);
        CHECK(v == 0xFFu);
        CHECK(r.ec == zb::FromCharsError::None);
        CHECK(r.ptr == s + 2); // Stopped at 'X'.
    }

    SECTION("Octal stops at out-of-radix digit '8'")
    {
        const char s[] = "78";
        zb::U32    v   = 0u;
        const auto r   = zb::fromCharsRadix(s, s + 2, v, zb::Radix::Oct);
        CHECK(v == 7u);
        CHECK(r.ec == zb::FromCharsError::None);
        CHECK(r.ptr == s + 1);
    }

    SECTION("Binary stops at out-of-radix digit '2'")
    {
        const char s[] = "1012";
        zb::U32    v   = 0u;
        const auto r   = zb::fromCharsRadix(s, s + 4, v, zb::Radix::Bin);
        CHECK(v == 0b101u);
        CHECK(r.ec == zb::FromCharsError::None);
        CHECK(r.ptr == s + 3);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - failure modes")
{
    SECTION("Empty input is InvalidArgument")
    {
        zb::U32    v = 0u;
        const auto r = zb::fromCharsRadix(nullptr, nullptr, v, zb::Radix::Hex);
        CHECK(r.ec == zb::FromCharsError::InvalidArgument);
    }

    SECTION("No valid digits is InvalidArgument")
    {
        zb::U32    v = 0u;
        const auto r = parse("zzz", v, zb::Radix::Hex);
        CHECK(r.ec == zb::FromCharsError::InvalidArgument);
    }

    SECTION("Leading sign is not accepted")
    {
        zb::U32    v = 0u;
        const auto r = parse("-1", v, zb::Radix::Hex);
        CHECK(r.ec == zb::FromCharsError::InvalidArgument);
    }

    SECTION("Overflow on a too-large hex value")
    {
        zb::U8     v = 0u;
        const auto r = parse("100", v, zb::Radix::Hex); // 0x100 doesn't fit in U8
        CHECK(r.ec == zb::FromCharsError::ResultOutOfRange);
    }

    SECTION("Maximum value fits exactly")
    {
        zb::U32    v = 0u;
        const auto r = parse("ffffffff", v, zb::Radix::Hex);
        CHECK(v == 0xFF'FF'FF'FFu);
        CHECK(r.ec == zb::FromCharsError::None);
    }

    SECTION("Just-over-maximum is rejected")
    {
        zb::U32    v = 0u;
        const auto r = parse("100000000", v, zb::Radix::Hex);
        CHECK(r.ec == zb::FromCharsError::ResultOutOfRange);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] FromCharsRadix.hpp - round-trip with toCharsRadix")
{
    // Sanity: parse what we format. Decoupled, but the two halves should agree.
    for (zb::U32 v : {0u, 1u, 0xFu, 0x10u, 0xAB'CDu, 0xDE'AD'BE'EFu, 0xFF'FF'FF'FFu})
    {
        for (auto radix : {zb::Radix::Bin, zb::Radix::Oct, zb::Radix::Hex})
        {
            char        buf[64]{};
            const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), v, radix);
            REQUIRE(end != nullptr);

            zb::U32    parsed = 0u;
            const auto r      = zb::fromCharsRadix(buf, end, parsed, radix);
            CHECK(r.ec == zb::FromCharsError::None);
            CHECK(r.ptr == end);
            CHECK(parsed == v);
        }
    }
}
