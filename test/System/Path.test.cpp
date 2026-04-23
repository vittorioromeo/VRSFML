#include "StringifySfBaseStringUtil.hpp"
#include "StringifyStdStringUtil.hpp"
#include "TemporaryFile.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>

#include <filesystem>
#include <sstream>
#include <string>


using sf::testing::TemporaryFile;


TEST_CASE("[System] sf::Path")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::Path));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Path));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Path));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Path));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Path));
    }

    SECTION("Default construction")
    {
        const sf::Path p;
        CHECK(p.empty());
        CHECK(!p.exists());
    }

    SECTION("Construction from const char*")
    {
        const sf::Path p("hello.txt");
        CHECK(!p.empty());
        CHECK(p.to<std::string>() == "hello.txt");
    }

    SECTION("Construction from const wchar_t*")
    {
        const sf::Path p(L"hello.txt");
        CHECK(p.to<std::wstring>() == std::wstring(L"hello.txt"));
    }

    SECTION("Construction from const char32_t*")
    {
        const sf::Path p(U"hello.txt");
        CHECK(p.to<std::u32string>() == std::u32string(U"hello.txt"));
    }

    SECTION("Construction from std::string")
    {
        const std::string s = "hello.txt";
        const sf::Path    p(s);
        CHECK(p.to<std::string>() == "hello.txt");
    }

    SECTION("Construction from std::wstring")
    {
        const std::wstring s = L"hello.txt";
        const sf::Path     p(s);
        CHECK(p.to<std::wstring>() == s);
    }

    SECTION("Construction from std::u32string")
    {
        const std::u32string s = U"hello.txt";
        const sf::Path       p(s);
        CHECK(p.to<std::u32string>() == s);
    }

    SECTION("Construction from base::String")
    {
        const sf::base::String s("hello.txt");
        const sf::Path         p(s);
        CHECK(p.to<sf::base::String>() == sf::base::String("hello.txt"));
    }

    SECTION("Construction from std::filesystem::path")
    {
        const auto     fsp = sf::Path("hello.txt").to<std::filesystem::path>();
        const sf::Path p(fsp);
        CHECK(p.to<std::filesystem::path>() == fsp);
        CHECK(p == sf::Path("hello.txt"));
    }

    SECTION("Copy construction is independent")
    {
        const sf::Path a("foo.txt");
        sf::Path       b(a);
        CHECK(a == b);
        b /= sf::Path("sub");
        CHECK(a == sf::Path("foo.txt")); // original unchanged
        CHECK(b != a);
    }

    SECTION("Copy assignment is independent")
    {
        const sf::Path a("foo.txt");
        sf::Path       b("bar.txt");
        b = a;
        CHECK(a == b);
        b /= sf::Path("sub");
        CHECK(a == sf::Path("foo.txt")); // original unchanged
    }

    SECTION("Move construction transfers state")
    {
        sf::Path       src("foo.txt");
        const sf::Path dst(SFML_BASE_MOVE(src));
        CHECK(dst.to<std::string>() == "foo.txt");
    }

    SECTION("Move assignment transfers state")
    {
        sf::Path src("foo.txt");
        sf::Path dst("bar.txt");
        dst = SFML_BASE_MOVE(src);
        CHECK(dst.to<std::string>() == "foo.txt");
    }

    SECTION("filename()")
    {
        CHECK(sf::Path("/foo/bar.txt").filename() == sf::Path("bar.txt"));
        CHECK(sf::Path("bar.txt").filename() == sf::Path("bar.txt"));
        CHECK(sf::Path("/foo/").filename().empty());
        CHECK(sf::Path("").filename().empty());
    }

    SECTION("extension()")
    {
        CHECK(sf::Path("bar.txt").extension() == sf::Path(".txt"));
        CHECK(sf::Path("/foo/bar.TAR.GZ").extension() == sf::Path(".GZ"));
        CHECK(sf::Path("noext").extension().empty());
        CHECK(sf::Path("").extension().empty());
        CHECK(sf::Path(".hidden").extension().empty()); // leading dot is stem, not extension
    }

    SECTION("parent()")
    {
        CHECK(sf::Path("/foo/bar.txt").parent() == sf::Path("/foo"));
        CHECK(sf::Path("bar.txt").parent().empty());
    }

    SECTION("absolute() returns non-empty for non-empty relative path")
    {
        const sf::Path rel("hello.txt");
        const sf::Path abs = rel.absolute();
        CHECK(!abs.empty());
    }

    SECTION("c_str() returns non-null native string")
    {
        const sf::Path              p("hello.txt");
        const sf::Path::value_type* ptr = p.c_str();
        REQUIRE(ptr != nullptr);
        const bool firstNonNull = ptr[0] != sf::Path::value_type{0};
        CHECK(firstNonNull);
    }

    SECTION("to<T>() conversions")
    {
        const sf::Path p("hello.txt");
        CHECK(p.to<std::string>() == "hello.txt");
        CHECK(p.to<std::wstring>() == std::wstring(L"hello.txt"));
        CHECK(p.to<std::u8string>() == std::u8string(u8"hello.txt"));
        CHECK(p.to<std::u32string>() == std::u32string(U"hello.txt"));
        // Round-trip through std::filesystem::path and back via sf::Path
        CHECK(sf::Path(p.to<std::filesystem::path>()) == p);
        CHECK(p.to<sf::base::String>() == sf::base::String("hello.txt"));
    }

    SECTION("empty()")
    {
        CHECK(sf::Path().empty());
        CHECK(sf::Path("").empty());
        CHECK(!sf::Path("a").empty());
    }

    SECTION("exists() and removeFromDisk() with a real file")
    {
        const TemporaryFile tmp("payload");
        CHECK(tmp.getPath().exists());
        CHECK(tmp.getPath().removeFromDisk());
        CHECK(!tmp.getPath().exists());
    }

    SECTION("exists() returns false for non-existent path")
    {
        CHECK(!sf::Path("this/really/should/not/exist/0xDEADBEEF.tmp").exists());
    }

    SECTION("removeFromDisk() returns false for non-existent path")
    {
        CHECK(!sf::Path("this/really/should/not/exist/0xDEADBEEF.tmp").removeFromDisk());
    }

    SECTION("extensionIs() matches ASCII extensions")
    {
        CHECK(sf::Path("hello.png").extensionIs(".png"));
        CHECK(sf::Path("hello.wav").extensionIs(".wav"));
        CHECK(sf::Path("/path/to/hello.wav").extensionIs(".wav"));
        CHECK(!sf::Path("hello.png").extensionIs(".jpg"));
    }

    SECTION("extensionIs() is case-insensitive")
    {
        CHECK(sf::Path("hello.PNG").extensionIs(".png"));
        CHECK(sf::Path("hello.Png").extensionIs(".pNg"));
    }

    SECTION("extensionIs() returns false when no extension is present")
    {
        CHECK(!sf::Path("hello").extensionIs(".png"));
        CHECK(!sf::Path("").extensionIs(".png"));
    }

    SECTION("operator/= appends path components")
    {
        sf::Path p("foo");
        p /= sf::Path("bar.txt");
        CHECK(p.filename() == sf::Path("bar.txt"));
        CHECK(p.parent() == sf::Path("foo"));
    }

    SECTION("operator/ composes a new path")
    {
        const sf::Path joined = sf::Path("foo") / sf::Path("bar.txt");
        CHECK(joined.filename() == sf::Path("bar.txt"));
        CHECK(joined.parent() == sf::Path("foo"));
    }

    SECTION("operator== / operator!= on sf::Path")
    {
        CHECK(sf::Path("a.txt") == sf::Path("a.txt"));
        CHECK(sf::Path("a.txt") != sf::Path("b.txt"));
    }

    SECTION("operator== / operator!= against const char*")
    {
        CHECK(sf::Path("a.txt") == "a.txt");
        CHECK(sf::Path("a.txt") != "b.txt");
    }

    SECTION("operator== / operator!= against const wchar_t*")
    {
        // Extra parens force the expression to evaluate to `bool` before doctest's
        // expression decomposer runs, avoiding stringification of `wchar_t[N]`.
        CHECK((sf::Path(L"a.txt") == L"a.txt"));
        CHECK((sf::Path(L"a.txt") != L"b.txt"));
    }

    SECTION("operator== / operator!= against std::string")
    {
        const std::string rhs = "a.txt";
        CHECK(sf::Path("a.txt") == rhs);
        CHECK(sf::Path("b.txt") != rhs);
    }

    SECTION("operator<<(ostream, Path) produces non-empty output")
    {
        std::ostringstream oss;
        oss << sf::Path("hello.txt");
        CHECK(oss.str() == "hello.txt");
    }

    SECTION("tempDirectoryPath() returns an existing directory")
    {
        const sf::Path tmp = sf::Path::tempDirectoryPath();
        CHECK(!tmp.empty());
        CHECK(tmp.exists());
    }

    // ------------------------------------------------------------------
    // Unicode / UTF-8 round-trip (previously threw under MinGW/Clang64)
    // ------------------------------------------------------------------

    SECTION("to<std::string>() with non-ASCII paths does not throw")
    {
        CHECK_NOTHROW((void)sf::Path(U"hello-ñ.txt").to<std::string>());
        CHECK_NOTHROW((void)sf::Path(U"hello-ń.txt").to<std::string>());
        CHECK_NOTHROW((void)sf::Path(U"hello-日.txt").to<std::string>());
        CHECK_NOTHROW((void)sf::Path(U"hello-🐌.txt").to<std::string>());
    }

    SECTION("to<std::string>() produces UTF-8 for non-ASCII")
    {
        // U+00F1 (ñ) encodes to the UTF-8 bytes 0xC3 0xB1.
        const auto s = sf::Path(U"\u00F1").to<std::string>();
        REQUIRE(s.size() == 2);
        CHECK(static_cast<unsigned char>(s[0]) == 0xC3);
        CHECK(static_cast<unsigned char>(s[1]) == 0xB1);
    }

    SECTION("to<base::String>() with non-ASCII paths does not throw")
    {
        CHECK_NOTHROW((void)sf::Path(U"hello-ñ.txt").to<sf::base::String>());
        CHECK_NOTHROW((void)sf::Path(U"hello-日.txt").to<sf::base::String>());
        CHECK_NOTHROW((void)sf::Path(U"hello-🐌.txt").to<sf::base::String>());
    }

    SECTION("to<std::u32string>() round-trips non-ASCII exactly")
    {
        CHECK(sf::Path(U"hello-🐌.txt").to<std::u32string>() == U"hello-🐌.txt");
        CHECK(sf::Path(U"hello-日.txt").to<std::u32string>() == U"hello-日.txt");
    }

    SECTION("operator<<(ostream, Path) with non-ASCII does not throw")
    {
        // If streaming threw, doctest catches it and fails the test -- no explicit guard needed.
        std::ostringstream oss;
        oss << sf::Path(U"hello-🐌.txt");
        CHECK(!oss.str().empty());
    }

    SECTION("extensionIs() does not throw on non-ASCII paths")
    {
        CHECK_NOTHROW((void)sf::Path(U"hello-🐌").extensionIs(".png"));
        CHECK_NOTHROW((void)sf::Path(U"hello-ñ").extensionIs(".png"));
        CHECK(!sf::Path(U"hello-🐌").extensionIs(".png"));
        CHECK(sf::Path(U"hello-🐌.png").extensionIs(".png"));
    }
}
