#include "StringifySfBaseStringUtil.hpp"
#include "StringifyStringViewUtil.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Base/Scn/Scn.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Radix.hpp"
#include "SFML/Base/Scn/ScnNumeric.hpp"
#include "SFML/Base/Scn/ScnString.hpp"
#include "SFML/Base/Scn/ScnStringSource.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"


////////////////////////////////////////////////////////////
namespace
{
[[nodiscard]] sf::base::ScnStringSource stringSource(sf::base::StringView v)
{
    return sf::base::ScnStringSource{v};
}


////////////////////////////////////////////////////////////
// Pure-`ScnSource` wrapper that hides `remaining()`/`advance()` -- forces
// parsers to take the slow per-byte fallback path. Used in tests that
// need to validate the non-contiguous codepath.
class NonContiguousSource
{
public:
    explicit NonContiguousSource(sf::base::StringView v) noexcept : m_inner{v}
    {
    }

    [[nodiscard]] sf::base::Optional<char> peek() const noexcept
    {
        return m_inner.peek();
    }

    void consume() noexcept
    {
        m_inner.consume();
    }

private:
    sf::base::ScnStringSource m_inner;
};

static_assert(sf::base::ScnSource<NonContiguousSource>);
static_assert(!sf::base::ContiguousScnSource<NonContiguousSource>);

static_assert(sf::base::ScnSource<sf::base::ScnStringSource>);
static_assert(sf::base::ContiguousScnSource<sf::base::ScnStringSource>);

namespace customscn
{
struct MyVec2
{
    int x = 0;
    int y = 0;
};

inline bool scnArg(sf::base::ScnStringSource& src, MyVec2& out)
{
    // Format: `x,y`
    if (!sf::base::scnInto(src, out.x))
        return false;

    auto c = src.peek();
    if (!c || *c != ',')
        return false;

    src.consume();
    return sf::base::scnInto(src, out.y);
}

struct BaseValue
{
    int value = 0;
};

struct DerivedValue : BaseValue
{
};

inline bool scnArg(sf::base::ScnStringSource& src, BaseValue& out)
{
    return sf::base::scnInto(src, out.value);
}
} // namespace customscn

} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - empty input")
{
    auto src = stringSource({});

    CHECK(sf::base::scnAtEnd(src));
    CHECK(src.bytesConsumed() == 0u);

    char c = 0;
    CHECK_FALSE(sf::base::scnInto(src, c));

    sf::base::String s;
    CHECK_FALSE(sf::base::scnInto(src, s));

    int n = 0;
    CHECK_FALSE(sf::base::scnInto(src, n));

    bool b = true;
    CHECK_FALSE(sf::base::scnInto(src, b));

    CHECK_FALSE(sf::base::scn<int>(src).hasValue());
    CHECK_FALSE(sf::base::scn<sf::base::String>(src).hasValue());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnArg<char> does NOT skip whitespace")
{
    auto src = stringSource("  ab");

    char c = 0;
    CHECK(sf::base::scnInto(src, c));
    CHECK(c == ' ');

    CHECK(sf::base::scnInto(src, c));
    CHECK(c == ' ');

    CHECK(sf::base::scnInto(src, c));
    CHECK(c == 'a');

    CHECK(sf::base::scnInto(src, c));
    CHECK(c == 'b');

    CHECK_FALSE(sf::base::scnInto(src, c));
    CHECK(sf::base::scnAtEnd(src));
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnArg<String> reads whitespace-delimited token")
{
    SUBCASE("token sequence")
    {
        auto src = stringSource("foo bar baz");

        const auto a = sf::base::scn<sf::base::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == sf::base::String{"foo"});

        const auto b = sf::base::scn<sf::base::String>(src);
        REQUIRE(b.hasValue());
        CHECK(*b == sf::base::String{"bar"});

        const auto c = sf::base::scn<sf::base::String>(src);
        REQUIRE(c.hasValue());
        CHECK(*c == sf::base::String{"baz"});

        CHECK_FALSE(sf::base::scn<sf::base::String>(src).hasValue());
    }

    SUBCASE("leading whitespace is skipped")
    {
        auto src = stringSource("   hello\tworld\n");

        const auto a = sf::base::scn<sf::base::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == sf::base::String{"hello"});

        const auto b = sf::base::scn<sf::base::String>(src);
        REQUIRE(b.hasValue());
        CHECK(*b == sf::base::String{"world"});

        CHECK_FALSE(sf::base::scn<sf::base::String>(src).hasValue());
    }

    SUBCASE("trailing whitespace produces a final token then EOF")
    {
        auto src = stringSource("only   ");

        const auto a = sf::base::scn<sf::base::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == sf::base::String{"only"});

        CHECK_FALSE(sf::base::scn<sf::base::String>(src).hasValue());
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnReadLine")
{
    using sf::base::scnReadLine;

    SUBCASE("multiple lines, LF endings")
    {
        auto src = stringSource("alpha\nbeta\ngamma\n");

        sf::base::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"alpha"});

        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"beta"});

        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"gamma"});

        CHECK_FALSE(scnReadLine(src, line));
    }

    SUBCASE("CRLF endings leave the '\\r' in the result")
    {
        auto src = stringSource("alpha\r\nbeta\r\n");

        sf::base::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"alpha\r"});

        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"beta\r"});
    }

    SUBCASE("missing trailing newline still returns the last line")
    {
        auto src = stringSource("first\nlast-no-newline");

        sf::base::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"first"});

        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"last-no-newline"});

        CHECK_FALSE(scnReadLine(src, line));
    }

    SUBCASE("empty lines are reported as success with empty string")
    {
        auto src = stringSource("\n\nx\n");

        sf::base::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line.empty());

        CHECK(scnReadLine(src, line));
        CHECK(line.empty());

        CHECK(scnReadLine(src, line));
        CHECK(line == sf::base::String{"x"});

        CHECK_FALSE(scnReadLine(src, line));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - integer parsing skips whitespace")
{
    SUBCASE("multiple signed values")
    {
        auto src = stringSource("42 -7 100");

        CHECK(*sf::base::scn<int>(src) == 42);
        CHECK(*sf::base::scn<int>(src) == -7);
        CHECK(*sf::base::scn<int>(src) == 100);

        CHECK_FALSE(sf::base::scn<int>(src).hasValue());
    }

    SUBCASE("stops at first non-digit")
    {
        auto src = stringSource("123.45");

        const auto a = sf::base::scn<int>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == 123);

        // The '.' is still on the source -- it is not a valid int byte.
        CHECK_FALSE(sf::base::scn<int>(src).hasValue());
    }

    SUBCASE("non-numeric input fails")
    {
        auto src = stringSource("hello");

        CHECK_FALSE(sf::base::scn<int>(src).hasValue());
    }

    SUBCASE("unsigned rejects negative")
    {
        auto src = stringSource("-1x");

        CHECK_FALSE(sf::base::scn<unsigned int>(src).hasValue());
        CHECK(src.bytesConsumed() == 2u);

        char c = '\0';
        CHECK(sf::base::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("signed minimum is accepted")
    {
        auto src = stringSource("-2147483648");

        const auto v = sf::base::scn<int>(src);
        REQUIRE(v.hasValue());
        CHECK(*v == -2'147'483'648);
    }

    SUBCASE("signed below minimum is consumed and rejected")
    {
        auto src = stringSource("-2147483649x");

        int v = 123;
        CHECK_FALSE(sf::base::scnInto(src, v));
        CHECK(v == 123);
        CHECK(src.bytesConsumed() == 11u);

        char c = '\0';
        CHECK(sf::base::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("many leading zeroes are accepted")
    {
        sf::base::String input;
        for (sf::base::SizeT i = 0u; i < 32u; ++i)
            input.append('0');
        input.append("1x");

        auto src = stringSource(input.toStringView());

        unsigned v = 123u;
        CHECK(sf::base::scnInto(src, v));
        CHECK(v == 1u);
        CHECK(src.bytesConsumed() == 33u);

        char c = '\0';
        CHECK(sf::base::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("overflowing digit run is consumed and rejected")
    {
        auto src = stringSource("99999999999999999999999999999999x");

        unsigned v = 123u;
        CHECK_FALSE(sf::base::scnInto(src, v));
        CHECK(v == 123u);
        CHECK(src.bytesConsumed() == 32u);

        char c = '\0';
        CHECK(sf::base::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnRadix")
{
    using sf::base::Radix;
    using sf::base::scnRadix;

    SUBCASE("hex lowercase")
    {
        auto            src = stringSource("abcd");
        sf::base::SizeT v   = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xab'cdu);
    }

    SUBCASE("hex stops at first non-hex byte")
    {
        auto src = stringSource("ff\n0\n");

        sf::base::SizeT v = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xffu);

        sf::base::scnSkipWhitespace(src);

        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0u);
    }

    SUBCASE("octal rejects '8'")
    {
        auto     src = stringSource("78");
        unsigned v   = 0u;
        CHECK(scnRadix(src, v, Radix::Oct));
        CHECK(v == 7u);
    }

    SUBCASE("leading whitespace is skipped")
    {
        auto     src = stringSource("   1a");
        unsigned v   = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0x1au);
    }

    SUBCASE("overlong radix digit run is consumed and rejected")
    {
        sf::base::String input;
        for (sf::base::SizeT i = 0u; i < 70u; ++i)
            input.append('f');
        input.append('z');

        auto src = stringSource(input.toStringView());

        unsigned v = 123u;
        CHECK_FALSE(scnRadix(src, v, Radix::Hex));
        CHECK(v == 123u);
        CHECK(src.bytesConsumed() == 70u);

        char c = '\0';
        CHECK(sf::base::scnInto(src, c));
        CHECK(c == 'z');
    }

    SUBCASE("many leading zeroes followed by small value parse correctly")
    {
        // Incremental parser accepts arbitrarily many leading zeroes
        // since the accumulated value stays in range. The previous
        // scratch-buffer impl rejected this once the run exceeded
        // `scnNumericScratchSize`.
        sf::base::String input;
        for (sf::base::SizeT i = 0u; i < 200u; ++i)
            input.append('0');
        input.append("ff");

        auto src = stringSource(input.toStringView());

        unsigned v = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xffu);
        CHECK(src.bytesConsumed() == 202u);
    }

    SUBCASE("octal with leading zeroes")
    {
        sf::base::String input;
        for (sf::base::SizeT i = 0u; i < 100u; ++i)
            input.append('0');
        input.append("17");

        auto src = stringSource(input.toStringView());

        unsigned v = 0u;
        CHECK(scnRadix(src, v, Radix::Oct));
        CHECK(v == 017u);
    }

    SUBCASE("binary with leading zeroes")
    {
        sf::base::String input;
        for (sf::base::SizeT i = 0u; i < 100u; ++i)
            input.append('0');
        input.append("1011");

        auto src = stringSource(input.toStringView());

        unsigned v = 0u;
        CHECK(scnRadix(src, v, Radix::Bin));
        CHECK(v == 0b1011u);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - float parsing")
{
    SUBCASE("positive and negative")
    {
        auto src = stringSource("3.14 -2.5 0.0");

        CHECK(*sf::base::scn<double>(src) == tst::Approx(3.14));
        CHECK(*sf::base::scn<double>(src) == tst::Approx(-2.5));
        CHECK(*sf::base::scn<double>(src) == tst::Approx(0.0));
    }

    SUBCASE("integer-only is accepted")
    {
        auto src = stringSource("42");

        CHECK(*sf::base::scn<float>(src) == tst::Approx(42.0));
    }

    SUBCASE("overlong float token is consumed and rejected")
    {
        sf::base::String input;
        for (sf::base::SizeT i = 0u; i < 45u; ++i)
            input.append('1');
        input.append(".25x");

        auto src = stringSource(input.toStringView());

        double v = 12.0;
        CHECK_FALSE(sf::base::scnInto(src, v));
        CHECK(v == tst::Approx(12.0));
        CHECK(src.bytesConsumed() == 48u);

        char c = '\0';
        CHECK(sf::base::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - bool parsing")
{
    SUBCASE("literal true/false")
    {
        auto src = stringSource("true false true");

        CHECK(*sf::base::scn<bool>(src) == true);
        CHECK(*sf::base::scn<bool>(src) == false);
        CHECK(*sf::base::scn<bool>(src) == true);
    }

    SUBCASE("digit form")
    {
        auto src = stringSource("0 1 0");

        CHECK(*sf::base::scn<bool>(src) == false);
        CHECK(*sf::base::scn<bool>(src) == true);
        CHECK(*sf::base::scn<bool>(src) == false);
    }

    SUBCASE("garbage rejected")
    {
        auto src = stringSource("hello");

        CHECK_FALSE(sf::base::scn<bool>(src).hasValue());
    }

    SUBCASE("'truncated' literal fails")
    {
        auto src = stringSource("trux");

        CHECK_FALSE(sf::base::scn<bool>(src).hasValue());
        CHECK(src.bytesConsumed() == 3u);

        char c = '\0';
        CHECK(sf::base::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnSkipPast")
{
    SUBCASE("delimiter found in the middle")
    {
        auto src = stringSource("garbage:value");

        sf::base::scnSkipPast(src, ':');

        const auto tok = sf::base::scn<sf::base::String>(src);
        REQUIRE(tok.hasValue());
        CHECK(*tok == sf::base::String{"value"});
    }

    SUBCASE("delimiter not present: drain the source")
    {
        auto src = stringSource("no-delim");

        sf::base::scnSkipPast(src, ':');

        CHECK(sf::base::scnAtEnd(src));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - mixed value types in one source")
{
    auto src = stringSource("42 hello 3.14 true x");

    CHECK(*sf::base::scn<int>(src) == 42);

    const auto tok = sf::base::scn<sf::base::String>(src);
    REQUIRE(tok.hasValue());
    CHECK(*tok == sf::base::String{"hello"});

    CHECK(*sf::base::scn<double>(src) == tst::Approx(3.14));
    CHECK(*sf::base::scn<bool>(src) == true);

    // `scnArg<char>` does not skip leading whitespace.
    sf::base::scnSkipWhitespace(src);

    char c = 0;
    CHECK(sf::base::scnInto(src, c));
    CHECK(c == 'x');
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - non-contiguous source takes the fallback path")
{
    NonContiguousSource src{"hello world\nline2"};

    sf::base::String tok;
    CHECK(sf::base::scnInto(src, tok));
    CHECK(tok == sf::base::String{"hello"});

    CHECK(sf::base::scnInto(src, tok));
    CHECK(tok == sf::base::String{"world"});

    sf::base::String line;
    CHECK(sf::base::scnReadLine(src, line));
    // The next byte after "world" is '\n', so the line is empty -- but
    // `scnArg<String>` left the source positioned at the '\n' itself
    // since the trailing whitespace check stopped there. `scnReadLine`
    // therefore consumes the '\n' and returns an empty `line`.
    CHECK(line.empty());

    CHECK(sf::base::scnReadLine(src, line));
    CHECK(line == sf::base::String{"line2"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - long inputs (contiguous fast path)")
{
    // Push a 4 KiB token through to exercise the bulk-append branch.
    sf::base::String payload;
    payload.reserve(4096u + 8u);
    for (sf::base::SizeT i = 0u; i < 4096u; ++i)
        payload.append('a');
    payload.append(" tail");

    auto src = sf::base::ScnStringSource{payload.toStringView()};

    const auto tok = sf::base::scn<sf::base::String>(src);
    REQUIRE(tok.hasValue());
    CHECK(tok->size() == 4096u);

    const auto tail = sf::base::scn<sf::base::String>(src);
    REQUIRE(tail.hasValue());
    CHECK(*tail == sf::base::String{"tail"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ScnStringSource bytesConsumed tracks position")
{
    auto src = stringSource("42 hello");

    CHECK(src.bytesConsumed() == 0u);
    CHECK(*sf::base::scn<int>(src) == 42);
    CHECK(src.bytesConsumed() == 2u);

    sf::base::String tok;
    CHECK(sf::base::scnInto(src, tok));
    CHECK(tok == sf::base::String{"hello"});
    CHECK(src.bytesConsumed() == 8u);
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ScnStringSource advance covers full range without overflow")
{
    // Regression: the precondition used to be `m_pos + n <= size()`,
    // which could wrap for huge `n`. The non-overflowing form
    // `n <= size() - m_pos` is exercised below at the boundary
    // (advance exactly to end, advance zero, advance after partial).
    sf::base::ScnStringSource src{"abcdef"};

    src.advance(0u);
    CHECK(src.bytesConsumed() == 0u);
    CHECK(src.remaining().size() == 6u);

    src.advance(3u);
    CHECK(src.bytesConsumed() == 3u);
    CHECK(src.remaining() == sf::base::StringView{"def"});

    // Boundary: advance exactly to end.
    src.advance(3u);
    CHECK(src.bytesConsumed() == 6u);
    CHECK(src.remaining().empty());
    CHECK(sf::base::scnAtEnd(src));

    // Zero-advance at end is still valid.
    src.advance(0u);
    CHECK(src.remaining().empty());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ADL on a user-defined type via custom scnArg")
{
    auto              src = stringSource("3,7");
    customscn::MyVec2 v{};
    CHECK(sf::base::scnInto(src, v));
    CHECK(v.x == 3);
    CHECK(v.y == 7);
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - base-class ADL customization remains viable")
{
    auto src = stringSource("42");

    customscn::DerivedValue v{};
    CHECK(sf::base::scnInto(src, v));
    CHECK(v.value == 42);
}
