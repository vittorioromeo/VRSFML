#include "StringifySfBaseStringUtil.hpp"
#include "StringifyStringViewUtil.hpp"

#include "SFML/Base/Scanner.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"

#include <Doctest.hpp>


////////////////////////////////////////////////////////////
namespace
{
[[nodiscard]] auto stringScanner(sf::base::StringView v)
{
    return sf::base::Scanner{sf::base::StringSource{v}};
}
} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - empty input")
{
    auto sc = stringScanner({});

    CHECK(sc.atEnd());
    CHECK(sc.bytesConsumed() == 0u);

    char c{};
    CHECK_FALSE(sc.readChar(c));

    sf::base::String s;
    CHECK_FALSE(sc.readToken(s));
    CHECK_FALSE(sc.readLine(s));

    int n = 0;
    CHECK_FALSE(sc.readInt(n));
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - readChar")
{
    auto sc = stringScanner("ab");

    char c{};
    CHECK(sc.readChar(c));
    CHECK(c == 'a');
    CHECK(sc.bytesConsumed() == 1u);

    CHECK(sc.readChar(c));
    CHECK(c == 'b');
    CHECK(sc.bytesConsumed() == 2u);

    CHECK_FALSE(sc.readChar(c));
    CHECK(sc.atEnd());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - readToken")
{
    SECTION("Simple whitespace-separated tokens")
    {
        auto             sc = stringScanner("foo bar baz");
        sf::base::String out;

        CHECK(sc.readToken(out));
        CHECK(out == sf::base::String{"foo"});

        CHECK(sc.readToken(out));
        CHECK(out == sf::base::String{"bar"});

        CHECK(sc.readToken(out));
        CHECK(out == sf::base::String{"baz"});

        CHECK_FALSE(sc.readToken(out));
        CHECK(sc.atEnd());
    }

    SECTION("Leading whitespace is skipped")
    {
        auto             sc = stringScanner("   hello\tworld\n");
        sf::base::String out;

        CHECK(sc.readToken(out));
        CHECK(out == sf::base::String{"hello"});

        CHECK(sc.readToken(out));
        CHECK(out == sf::base::String{"world"});

        CHECK_FALSE(sc.readToken(out));
    }

    SECTION("Trailing whitespace is not consumed by token")
    {
        auto             sc = stringScanner("only   ");
        sf::base::String out;

        CHECK(sc.readToken(out));
        CHECK(out == sf::base::String{"only"});

        CHECK_FALSE(sc.readToken(out));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - readLine")
{
    SECTION("Multiple LF-terminated lines")
    {
        auto             sc = stringScanner("alpha\nbeta\ngamma\n");
        sf::base::String line;

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"alpha"});

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"beta"});

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"gamma"});

        CHECK_FALSE(sc.readLine(line));
    }

    SECTION("CRLF endings preserve the '\\r' in the output")
    {
        auto             sc = stringScanner("alpha\r\nbeta\r\n");
        sf::base::String line;

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"alpha\r"});

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"beta\r"});
    }

    SECTION("Final line without trailing '\\n' is still returned")
    {
        auto             sc = stringScanner("first\nlast-no-newline");
        sf::base::String line;

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"first"});

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"last-no-newline"});

        CHECK_FALSE(sc.readLine(line));
    }

    SECTION("Empty lines yield empty strings")
    {
        auto             sc = stringScanner("\n\nx\n");
        sf::base::String line;

        CHECK(sc.readLine(line));
        CHECK(line.empty());

        CHECK(sc.readLine(line));
        CHECK(line.empty());

        CHECK(sc.readLine(line));
        CHECK(line == sf::base::String{"x"});

        CHECK_FALSE(sc.readLine(line));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - readInt")
{
    SECTION("Decimal integers, signed and unsigned")
    {
        auto sc = stringScanner("42 -7 100");

        int      a = 0;
        int      b = 0;
        unsigned c = 0;
        CHECK(sc.readInt(a));
        CHECK(a == 42);
        CHECK(sc.readInt(b));
        CHECK(b == -7);
        CHECK(sc.readInt(c));
        CHECK(c == 100u);
    }

    SECTION("Float-looking input parses only the integer prefix")
    {
        auto sc = stringScanner("123.45");
        int  v  = 0;
        CHECK(sc.readInt(v));
        CHECK(v == 123);

        // The '.' is left as the next byte, then "45" follows.
        char c{};
        CHECK(sc.readChar(c));
        CHECK(c == '.');
    }

    SECTION("Non-digit input fails without consuming the byte")
    {
        auto sc = stringScanner("hello");
        int  v  = 99;
        CHECK_FALSE(sc.readInt(v));

        // The 'h' is still readable.
        char c{};
        CHECK(sc.readChar(c));
        CHECK(c == 'h');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - readIntRadix")
{
    SECTION("Hex")
    {
        auto sc = stringScanner("abcd");

        sf::base::U32 v = 0u;
        CHECK(sc.readIntRadix(v, sf::base::Radix::Hex));
        CHECK(v == 0xAB'CDu);
    }

    SECTION("Hex chained reads (chunked-encoding pattern)")
    {
        auto sc = stringScanner("ff\n0\n");

        sf::base::SizeT length = 0u;
        CHECK(sc.readIntRadix(length, sf::base::Radix::Hex));
        CHECK(length == 0xFFu);

        sc.skipPast('\n');

        CHECK(sc.readIntRadix(length, sf::base::Radix::Hex));
        CHECK(length == 0u);
    }

    SECTION("Whitespace is skipped before the digits")
    {
        auto          sc = stringScanner("   1a");
        sf::base::U32 v  = 0u;
        CHECK(sc.readIntRadix(v, sf::base::Radix::Hex));
        CHECK(v == 0x1Au);
    }

    SECTION("Octal stops at out-of-range digit")
    {
        auto          sc = stringScanner("78");
        sf::base::U32 v  = 0u;
        CHECK(sc.readIntRadix(v, sf::base::Radix::Oct));
        CHECK(v == 7u);

        // The '8' is left for the next read.
        char c{};
        CHECK(sc.readChar(c));
        CHECK(c == '8');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - skipPast")
{
    SECTION("Consumes through the delimiter")
    {
        auto sc = stringScanner("garbage:value");
        sc.skipPast(':');

        sf::base::String rest;
        CHECK(sc.readToken(rest));
        CHECK(rest == sf::base::String{"value"});
    }

    SECTION("Stops at EOF if delimiter is missing")
    {
        auto sc = stringScanner("no-delim");
        sc.skipPast(';');

        CHECK(sc.atEnd());
    }

    SECTION("maxCount caps the skip")
    {
        auto sc = stringScanner("abcdef:end");
        sc.skipPast(':', /* maxCount */ 3u);

        // We capped before reaching ':', so we should still be at 'd'.
        char c{};
        CHECK(sc.readChar(c));
        CHECK(c == 'd');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - bytesConsumed tracks position across mixed ops")
{
    auto sc = stringScanner("42 hello\nworld");

    int v = 0;
    CHECK(sc.readInt(v));
    CHECK(v == 42);
    CHECK(sc.bytesConsumed() == 2u);

    sf::base::String tok;
    CHECK(sc.readToken(tok));
    CHECK(tok == sf::base::String{"hello"});
    CHECK(sc.bytesConsumed() == 8u); // "42 hello"

    sf::base::String line;
    CHECK(sc.readLine(line)); // empty line because cursor is at '\n'
    CHECK(line.empty());
    CHECK(sc.bytesConsumed() == 9u);

    CHECK(sc.readLine(line));
    CHECK(line == sf::base::String{"world"});
    CHECK(sc.bytesConsumed() == 14u);
    CHECK(sc.atEnd());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scanner.hpp - input longer than buffer triggers multiple refills")
{
    // Build a payload twice the size of the staging buffer; the line
    // accumulation must seamlessly span the refill boundary.
    sf::base::String payload;
    payload.reserve(2u * sf::base::Scanner<sf::base::StringSource>::bufferSize + 8u);

    for (sf::base::SizeT i = 0u; i < 2u * sf::base::Scanner<sf::base::StringSource>::bufferSize; ++i)
        payload.append('x');
    payload.append("\nshort");

    auto sc = sf::base::Scanner{sf::base::StringSource{payload.toStringView()}};

    sf::base::String line;
    CHECK(sc.readLine(line));
    CHECK(line.size() == 2u * sf::base::Scanner<sf::base::StringSource>::bufferSize);

    CHECK(sc.readLine(line));
    CHECK(line == sf::base::String{"short"});
}
