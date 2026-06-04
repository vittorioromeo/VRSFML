#include "StringifyStringViewUtil.hpp"
#include "Tst/Tst.hpp"

#include "ZancleBase/ToCharsRadix.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/StringView.hpp"

#include <limits>


////////////////////////////////////////////////////////////
namespace
{
// Wrap a successful `[first, end)` result from `toCharsRadix` in a
// `StringView` for readable assertions. Precondition: `end != nullptr`
// (overflow is checked separately, not via this helper).
[[nodiscard]] zb::StringView view(const char* buf, const char* end)
{
    return {buf, static_cast<zb::SizeT>(end - buf)};
}
} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Base] ToCharsRadix.hpp - basic unsigned values")
{
    char buf[64]{};

    SECTION("Hex lowercase")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 0xAB'CDu, zb::Radix::Hex);
        CHECK(view(buf, end) == "abcd");
    }

    SECTION("Hex uppercase")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 0xAB'CDu, zb::Radix::Hex, /* upperHex */ true);
        CHECK(view(buf, end) == "ABCD");
    }

    SECTION("Octal")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 511u, zb::Radix::Oct);
        CHECK(view(buf, end) == "777");
    }

    SECTION("Binary")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 0xAu, zb::Radix::Bin);
        CHECK(view(buf, end) == "1010");
    }

    SECTION("Zero always emits a single digit")
    {
        const auto* eHex = zb::toCharsRadix(buf, buf + sizeof(buf), 0u, zb::Radix::Hex);
        CHECK(view(buf, eHex) == "0");

        const auto* eOct = zb::toCharsRadix(buf, buf + sizeof(buf), 0u, zb::Radix::Oct);
        CHECK(view(buf, eOct) == "0");

        const auto* eBin = zb::toCharsRadix(buf, buf + sizeof(buf), 0u, zb::Radix::Bin);
        CHECK(view(buf, eBin) == "0");
    }

    SECTION("upperHex is a no-op for non-hex radices")
    {
        const auto* eOct = zb::toCharsRadix(buf, buf + sizeof(buf), 9u, zb::Radix::Oct, /* upperHex */ true);
        CHECK(view(buf, eOct) == "11");

        const auto* eBin = zb::toCharsRadix(buf, buf + sizeof(buf), 9u, zb::Radix::Bin, /* upperHex */ true);
        CHECK(view(buf, eBin) == "1001");
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] ToCharsRadix.hpp - signed values emit raw bit pattern (no sign)")
{
    char buf[64]{};

    SECTION("Negative int -> 32-bit two's-complement hex")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), -1, zb::Radix::Hex);
        CHECK(view(buf, end) == "ffffffff");
    }

    SECTION("Negative int -> uppercase hex")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), -1, zb::Radix::Hex, /* upperHex */ true);
        CHECK(view(buf, end) == "FFFFFFFF");
    }

    SECTION("Negative signed char -> 8-bit binary")
    {
        // After integer-promotion-aware unsigned cast, -1 (signed char) ->
        // 0xFF (unsigned char) -> "11111111" in binary.
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), static_cast<signed char>(-1), zb::Radix::Bin);
        CHECK(view(buf, end) == "11111111");
    }

    SECTION("Negative long long -> 64-bit hex")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), static_cast<long long>(-1), zb::Radix::Hex);
        CHECK(view(buf, end) == "ffffffffffffffff");
    }

    SECTION("LLONG_MIN as 64-bit hex")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), std::numeric_limits<long long>::min(), zb::Radix::Hex);
        CHECK(view(buf, end) == "8000000000000000");
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] ToCharsRadix.hpp - integer width boundaries")
{
    char buf[64]{};

    SECTION("UINT32_MAX in hex = 8 digits")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 0xFF'FF'FF'FFu, zb::Radix::Hex);
        CHECK(view(buf, end) == "ffffffff");
    }

    SECTION("UINT64_MAX in hex = 16 digits")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 0xFF'FF'FF'FF'FF'FF'FF'FFull, zb::Radix::Hex);
        CHECK(view(buf, end) == "ffffffffffffffff");
    }

    SECTION("UINT64_MAX in binary = 64 digits")
    {
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 0xFF'FF'FF'FF'FF'FF'FF'FFull, zb::Radix::Bin);
        CHECK(end != nullptr);
        CHECK((end - buf) == 64);
        CHECK(view(buf, end).size() == 64u);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] ToCharsRadix.hpp - buffer overflow returns nullptr")
{
    char small[2]{};

    SECTION("Hex value needs 4 chars, only 2 available")
    {
        CHECK(zb::toCharsRadix(small, small + sizeof(small), 0xAB'CDu, zb::Radix::Hex) == nullptr);
    }

    SECTION("Binary value needs 8 chars, only 2 available")
    {
        CHECK(zb::toCharsRadix(small, small + sizeof(small), 0xFFu, zb::Radix::Bin) == nullptr);
    }

    SECTION("Empty buffer rejects even a single-digit zero")
    {
        CHECK(zb::toCharsRadix(small, small, 0u, zb::Radix::Hex) == nullptr);
    }

    SECTION("Exact-fit buffer succeeds")
    {
        char        exact[4]{};
        const auto* end = zb::toCharsRadix(exact, exact + sizeof(exact), 0xAB'CDu, zb::Radix::Hex);
        CHECK(end != nullptr);
        CHECK(view(exact, end) == "abcd");
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] ToCharsRadix.hpp - constexpr usability")
{
    // The function is marked `constexpr`; confirm it actually evaluates at
    // compile time when given a constant-evaluation context.
    constexpr auto checkHex = []
    {
        char        buf[8]{};
        const auto* end = zb::toCharsRadix(buf, buf + sizeof(buf), 0x2Au, zb::Radix::Hex);
        return end != nullptr && buf[0] == '2' && buf[1] == 'a' && (end - buf) == 2;
    };

    STATIC_CHECK(checkHex());
}
