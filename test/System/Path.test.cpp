#include "StringifySfBaseStringUtil.hpp" // IWYU: pragma keep
#include "StringifyStdStringUtil.hpp"    // IWYU: pragma keep
#include "TemporaryFile.hpp"
#include "Tst/Tst.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/System/Fmt/FmtPath.hpp" // IWYU pragma: keep -- enables `fmtArg(Path, ...)` for the format test
#include "SFML/System/IO.hpp"

#include "SFML/Base/Fmt/FmtToString.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <filesystem>
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
        CHECK(sf::Path("/foo/bar.txt").getFilename() == sf::Path("bar.txt"));
        CHECK(sf::Path("bar.txt").getFilename() == sf::Path("bar.txt"));
        CHECK(sf::Path("/foo/").getFilename().empty());
        CHECK(sf::Path("").getFilename().empty());
    }

    SECTION("extension()")
    {
        CHECK(sf::Path("bar.txt").getExtension() == sf::Path(".txt"));
        CHECK(sf::Path("/foo/bar.TAR.GZ").getExtension() == sf::Path(".GZ"));
        CHECK(sf::Path("noext").getExtension().empty());
        CHECK(sf::Path("").getExtension().empty());
        CHECK(sf::Path(".hidden").getExtension().empty()); // leading dot is stem, not extension
    }

    SECTION("parent()")
    {
        CHECK(sf::Path("/foo/bar.txt").getParent() == sf::Path("/foo"));
        CHECK(sf::Path("bar.txt").getParent().empty());
    }

    SECTION("absolute() returns non-empty for non-empty relative path")
    {
        const sf::Path rel("hello.txt");
        const auto     abs = rel.getAbsolute();
        REQUIRE(abs.hasValue());
        CHECK(!abs->empty());
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

    SECTION("extensionIs() honors std::filesystem extension semantics")
    {
        // Leading dot of the filename is part of the stem, not the extension.
        CHECK(!sf::Path(".hidden").extensionIs(".hidden"));
        CHECK(!sf::Path("/foo/.hidden").extensionIs(".hidden"));
        CHECK(!sf::Path(".bashrc").extensionIs(".bashrc"));

        // `.` and `..` filenames have no extension.
        CHECK(!sf::Path(".").extensionIs("."));
        CHECK(!sf::Path("..").extensionIs(".."));
        CHECK(!sf::Path("/foo/.").extensionIs("."));
        CHECK(!sf::Path("/foo/..").extensionIs(".."));

        // Multiple dots: the rightmost wins.
        CHECK(sf::Path("hello.tar.gz").extensionIs(".gz"));
        CHECK(!sf::Path("hello.tar.gz").extensionIs(".tar"));
        CHECK(!sf::Path("hello.tar.gz").extensionIs(".tar.gz"));
        CHECK(sf::Path("/path/with.dots/file.png").extensionIs(".png"));

        // Filename starting with a dot but containing another dot has the right-side extension.
        CHECK(sf::Path(".foo.bar").extensionIs(".bar"));

        // Empty extension query: matches iff the path has no extension.
        CHECK(!sf::Path("hello.png").extensionIs(""));
        CHECK(sf::Path("hello").extensionIs(""));
        CHECK(sf::Path(".hidden").extensionIs(""));
        CHECK(sf::Path("/foo/").extensionIs(""));
        CHECK(sf::Path("").extensionIs(""));
    }

    SECTION("operator/= appends path components")
    {
        sf::Path p("foo");
        p /= sf::Path("bar.txt");
        CHECK(p.getFilename() == sf::Path("bar.txt"));
        CHECK(p.getParent() == sf::Path("foo"));
    }

    SECTION("operator/ composes a new path")
    {
        const sf::Path joined = sf::Path("foo") / sf::Path("bar.txt");
        CHECK(joined.getFilename() == sf::Path("bar.txt"));
        CHECK(joined.getParent() == sf::Path("foo"));
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
        // Extra parens force the expression to evaluate to `bool` before the framework.s
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

    SECTION("Path formats via fmtToString(\"{}\", path)")
    {
        CHECK(sf::base::fmtToString("{}", sf::Path("hello.txt")) == sf::base::String("hello.txt"));
    }

    SECTION("tempDirectoryPath() returns an existing directory")
    {
        const auto tmp = sf::Path::getTempDirectory();
        REQUIRE(tmp.hasValue());
        CHECK(!tmp->empty());
        CHECK(tmp->exists());
    }

    SECTION("absolute() returns Optional that is convertible to bool")
    {
        const auto abs = sf::Path("hello.txt").getAbsolute();
        CHECK(static_cast<bool>(abs));
    }

    SECTION("stem()")
    {
        CHECK(sf::Path("/foo/bar.txt").getStem() == sf::Path("bar"));
        CHECK(sf::Path("/foo/bar.tar.gz").getStem() == sf::Path("bar.tar"));
        CHECK(sf::Path("noext").getStem() == sf::Path("noext"));
        CHECK(sf::Path(".hidden").getStem() == sf::Path(".hidden"));
        CHECK(sf::Path("").getStem().empty());
    }

    SECTION("isDirectory(), isRegularFile(), isSymlink() on temp file")
    {
        const TemporaryFile tmp("payload");
        CHECK(tmp.getPath().isRegularFile());
        CHECK(!tmp.getPath().isDirectory());
        CHECK(!tmp.getPath().isSymlink());

        const auto tempDir = sf::Path::getTempDirectory();
        REQUIRE(tempDir.hasValue());
        CHECK(tempDir->isDirectory());
        CHECK(!tempDir->isRegularFile());
    }

    SECTION("isDirectory(), isRegularFile() return false for missing path")
    {
        const sf::Path missing("this/really/should/not/exist/0xDEADBEEF.tmp");
        CHECK(!missing.isDirectory());
        CHECK(!missing.isRegularFile());
        CHECK(!missing.isSymlink());
    }

    SECTION("fileSize() returns the byte length of the file")
    {
        const TemporaryFile tmp("0123456789");
        const auto          sz = tmp.getPath().getFileSize();
        REQUIRE(sz.hasValue());
        CHECK(*sz == 10u);
    }

    SECTION("fileSize() fails for missing file")
    {
        CHECK(!sf::Path("this/really/does/not/exist.tmp").getFileSize().hasValue());
    }

    SECTION("lastWriteTimeSecondsSinceEpoch() returns a sensible value")
    {
        const TemporaryFile tmp("payload");
        const auto          t = tmp.getPath().getLastWriteTimeSecondsSinceEpoch();
        REQUIRE(t.hasValue());
        // Sanity: should be after 2020-01-01 (1577836800) and before 2200-01-01 (7258118400).
        CHECK(*t > 1'577'836'800);
        CHECK(*t < 7'258'118'400);
    }

    SECTION("currentWorkingDirectory() returns an existing directory")
    {
        const auto cwd = sf::Path::getCurrentDirectory();
        REQUIRE(cwd.hasValue());
        CHECK(cwd->isDirectory());
    }

    SECTION("setCurrentWorkingDirectory() round-trips through getter")
    {
        const auto originalMaybe = sf::Path::getCurrentDirectory();
        REQUIRE(originalMaybe.hasValue());
        const sf::Path& original = *originalMaybe;

        const auto tempMaybe = sf::Path::getTempDirectory();
        REQUIRE(tempMaybe.hasValue());

        REQUIRE(sf::Path::setCurrentDirectory(*tempMaybe));
        const auto afterChange = sf::Path::getCurrentDirectory();
        REQUIRE(afterChange.hasValue());
        CHECK(afterChange->isDirectory());

        // Restore so other tests aren't affected.
        REQUIRE(sf::Path::setCurrentDirectory(original));
    }

    SECTION("homeDirectory() returns a path on platforms with HOME / USERPROFILE set")
    {
        const auto home = sf::Path::getHomeDirectory();
        // We don't strictly require the env var to be set, but if it is, the
        // returned path shouldn't be empty.
        if (home.hasValue())
            CHECK(!home->empty());
    }

    SECTION("operator+= concatenates without inserting a separator")
    {
        sf::Path p("foo");
        p += sf::Path(".bak");
        CHECK(p == sf::Path("foo.bak"));
    }

    SECTION("operator+ concatenates without inserting a separator")
    {
        const sf::Path joined = sf::Path("file") + sf::Path(".tmp");
        CHECK(joined == sf::Path("file.tmp"));
    }

    SECTION("renameTo() moves an existing file")
    {
        const TemporaryFile src("payload");
        const auto          dstParent = sf::Path::getTempDirectory();
        REQUIRE(dstParent.hasValue());
        const sf::Path dst = *dstParent / sf::Path("sf_rename_target.tmp");

        // Best-effort cleanup in case a previous run left it behind.
        (void)dst.removeFromDisk();

        REQUIRE(src.getPath().renameTo(dst));
        CHECK(!src.getPath().exists());
        CHECK(dst.exists());

        CHECK(dst.removeFromDisk());
    }

    SECTION("renameTo() fails for a missing source")
    {
        const sf::Path missing("this/really/does/not/exist.tmp");
        const sf::Path dst("sf_rename_unused_target.tmp");
        CHECK(!missing.renameTo(dst));
    }

    SECTION("forEachEntry() iterates over directory contents")
    {
        const auto tempDir = sf::Path::getTempDirectory();
        REQUIRE(tempDir.hasValue());

        // Create a unique sub-directory with two files inside.
        const sf::Path subdir = *tempDir / sf::Path("sf_path_iter_test_dir");
        (void)subdir.removeFromDisk(); // best-effort cleanup
        REQUIRE(subdir.createDirectoryTree());

        const sf::Path fileA = subdir / sf::Path("a.txt");
        const sf::Path fileB = subdir / sf::Path("b.txt");
        {
            const TemporaryFile srcA("aaa");
            const TemporaryFile srcB("bbb");
            REQUIRE(srcA.getPath().copyFileTo(fileA));
            REQUIRE(srcB.getPath().copyFileTo(fileB));
        }

        int        count = 0;
        bool       sawA  = false;
        bool       sawB  = false;
        const bool ok    = subdir.forEachEntry([&](const sf::Path& entry)
        {
            ++count;
            if (entry.getFilename() == sf::Path("a.txt"))
                sawA = true;
            if (entry.getFilename() == sf::Path("b.txt"))
                sawB = true;
        });

        CHECK(ok);
        CHECK(count == 2);
        CHECK(sawA);
        CHECK(sawB);

        // Cleanup.
        CHECK(fileA.removeFromDisk());
        CHECK(fileB.removeFromDisk());
        CHECK(subdir.removeFromDisk());
    }

    SECTION("forEachEntry() returns false for a non-existent path")
    {
        const bool ok = sf::Path("this/really/does/not/exist").forEachEntry([](const sf::Path&) {});
        CHECK(!ok);
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

    SECTION("Path formatting with non-ASCII does not throw")
    {
        // If formatting threw, the test runner catches it and fails the test -- no explicit guard needed.
        CHECK(!sf::base::fmtToString("{}", sf::Path(U"hello-🐌.txt")).empty());
    }

    SECTION("extensionIs() does not throw on non-ASCII paths")
    {
        CHECK_NOTHROW((void)sf::Path(U"hello-🐌").extensionIs(".png"));
        CHECK_NOTHROW((void)sf::Path(U"hello-ñ").extensionIs(".png"));
        CHECK(!sf::Path(U"hello-🐌").extensionIs(".png"));
        CHECK(sf::Path(U"hello-🐌.png").extensionIs(".png"));
    }
}
