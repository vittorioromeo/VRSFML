#include "StringifySfBaseStringUtil.hpp"
#include "StringifyStdStringUtil.hpp"
#include "StringifyStringViewUtil.hpp"
#include "TemporaryFile.hpp"
#include "Tst/Tst.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/Scn/Scn.hpp"
#include "SFML/Base/Scn/ScnString.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"
#include "SFML/Base/Vector.hpp"

#include <string>


using sf::testing::TemporaryFile;


TEST_CASE("[System] sf::OutFile and sf::InFile")
{
    using namespace sf::base::literals;

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::OutFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::OutFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::OutFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::OutFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::OutFile));

        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::InFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::InFile));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::InFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::InFile));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::InFile));
    }

    SECTION("openFromFile returns nullOpt on non-openable path")
    {
        // A clearly bogus path; the factory should report failure rather than
        // returning a half-constructed object.
        const auto opt = sf::InFile::open(sf::Path{"/this/path/does/not/exist__"}, sf::FileOpenMode::bin);
        CHECK(!opt.hasValue());
    }

    SECTION("Write then read binary payload")
    {
        const TemporaryFile temporaryFile;

        {
            auto optOfs = sf::OutFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
            REQUIRE(optOfs.hasValue());

            constexpr sf::base::StringView payload = "Hello 42 world"_sv;
            CHECK(optOfs->write(payload.data(), payload.size()));
            CHECK(optOfs->flush());
            // Destructor closes when `optOfs` goes out of scope below.
        }

        {
            auto optIfs = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
            REQUIRE(optIfs.hasValue());

            char            buffer[14] = {};
            sf::base::SizeT got        = 0;
            CHECK(optIfs->read(buffer, 14, got));
            CHECK(got == 14u);
            CHECK(sf::base::StringView(buffer, 14) == "Hello 42 world"_sv);
        }
    }

    SECTION("OutFile tellPos and seekPos")
    {
        const TemporaryFile temporaryFile;

        auto optOfs = sf::OutFile::open(temporaryFile.getPath());
        REQUIRE(optOfs.hasValue());
        CHECK(optOfs->write("abcdef", 6));

        sf::base::PtrDiffT pos = 0;
        CHECK(optOfs->tellPos(pos));
        CHECK(pos == 6);

        CHECK(optOfs->seekPos(3));
        CHECK(optOfs->tellPos(pos));
        CHECK(pos == 3);

        CHECK(optOfs->write("XYZ", 3));
        CHECK(optOfs->tellPos(pos));
        CHECK(pos == 6);
        optOfs.reset(); // explicit close before reopening for read

        auto optIfs = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optIfs.hasValue());
        char            buffer[7] = {};
        sf::base::SizeT got       = 0;
        CHECK(optIfs->read(buffer, 6, got));
        CHECK(got == 6u);
        CHECK(sf::base::StringView(buffer, 6) == "abcXYZ"_sv);
    }

    SECTION("InFile seekPos and tellPos")
    {
        const TemporaryFile temporaryFile("Hello world");

        auto optIfs = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optIfs.hasValue());

        sf::base::PtrDiffT pos = -1;
        CHECK(optIfs->tellPos(pos));
        CHECK(pos == 0);

        CHECK(optIfs->seekPos(6));
        CHECK(optIfs->tellPos(pos));
        CHECK(pos == 6);

        char            buffer[6] = {};
        sf::base::SizeT got       = 0;
        CHECK(optIfs->read(buffer, 5, got));
        CHECK(got == 5u);
        CHECK(sf::base::StringView(buffer, 5) == "world"_sv);

        CHECK(optIfs->seekPos(0, sf::SeekDir::beg));
        CHECK(optIfs->tellPos(pos));
        CHECK(pos == 0);

        CHECK(optIfs->seekPos(0, sf::SeekDir::end));
        CHECK(optIfs->tellPos(pos));
        CHECK(pos == 11);
    }

    SECTION("InFile seekPos(cur) compensates for a live peek cache")
    {
        // Regression: after `peek()` the raw FILE* cursor is one byte
        // ahead of the logical cursor. `seekPos(N, cur)` must seek from
        // the *logical* position, not the raw FILE* position.
        const TemporaryFile temporaryFile("abcdef");

        auto optIfs = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optIfs.hasValue());

        const auto peeked = optIfs->peek();
        REQUIRE(peeked.hasValue());
        CHECK(*peeked == 'a');

        // Logical cursor is at 0 (we peeked but didn't consume).
        // `seekPos(1, cur)` should land at logical position 1 -> next read 'b'.
        CHECK(optIfs->seekPos(1, sf::SeekDir::cur));

        char            ch  = '\0';
        sf::base::SizeT got = 0;
        CHECK(optIfs->read(&ch, 1, got));
        CHECK(got == 1u);
        CHECK(ch == 'b');
    }

    SECTION("InFile reports EOF after reading past the end")
    {
        const TemporaryFile temporaryFile("abc");

        auto optIfs = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optIfs.hasValue());

        char            buffer[8] = {};
        sf::base::SizeT got       = 0;

        // The read returns success with a short count -- EOF is normal,
        // not an error.
        CHECK(optIfs->read(buffer, 8, got));
        CHECK(got == 3u);
        CHECK(optIfs->isEOF());
    }

    SECTION("InFile open with FileOpenMode::ate seeks to end")
    {
        const TemporaryFile temporaryFile("abc");

        auto optIfs = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin | sf::FileOpenMode::ate);
        REQUIRE(optIfs.hasValue());

        sf::base::PtrDiffT pos = 0;
        CHECK(optIfs->tellPos(pos));
        CHECK(pos == 3);
    }

    SECTION("Move semantics")
    {
        const TemporaryFile temporaryFile("Hello world");

        SECTION("OutFile move constructor")
        {
            const TemporaryFile target;

            auto optMoved = sf::OutFile::open(target.getPath());
            REQUIRE(optMoved.hasValue());

            sf::OutFile ofs = SFML_BASE_MOVE(*optMoved);
            CHECK(ofs.write("moved", 5));
            // ofs's destructor closes when this scope ends.
            {
                sf::OutFile temp = SFML_BASE_MOVE(ofs);
                (void)temp;
            }

            auto optIfs = sf::InFile::open(target.getPath(), sf::FileOpenMode::bin);
            REQUIRE(optIfs.hasValue());

            char            buffer[5] = {};
            sf::base::SizeT got       = 0;
            CHECK(optIfs->read(buffer, 5, got));
            CHECK(got == 5u);
            CHECK(sf::base::StringView(buffer, 5) == "moved"_sv);
        }

        SECTION("InFile move constructor")
        {
            auto optMoved = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
            REQUIRE(optMoved.hasValue());

            sf::InFile ifs = SFML_BASE_MOVE(*optMoved);

            char            buffer[5] = {};
            sf::base::SizeT got       = 0;
            CHECK(ifs.read(buffer, 5, got));
            CHECK(got == 5u);
            CHECK(sf::base::StringView(buffer, 5) == "Hello"_sv);
        }
    }
}


TEST_CASE("[System] sf::InFile peek/consume cache semantics")
{
    using namespace sf::base::literals;

    const TemporaryFile temporaryFile("ABCD"_sv);

    SECTION("peek without consume is idempotent")
    {
        auto optFile = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optFile.hasValue());

        CHECK(*optFile->peek() == 'A');
        CHECK(*optFile->peek() == 'A'); // Still 'A' -- cache hit, no advance.

        optFile->consume();
        CHECK(*optFile->peek() == 'B');
    }

    SECTION("peek + read delivers cached byte first")
    {
        auto optFile = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optFile.hasValue());

        CHECK(*optFile->peek() == 'A');

        char            buf[4]{};
        sf::base::SizeT got = 0u;
        CHECK(optFile->read(buf, 3, got));
        CHECK(got == 3u);
        CHECK(sf::base::StringView(buf, 3) == "ABC"_sv);
    }

    SECTION("tellPos accounts for peeked-but-not-consumed byte")
    {
        auto optFile = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optFile.hasValue());

        sf::base::PtrDiffT pos = -1;
        CHECK(optFile->tellPos(pos));
        CHECK(pos == 0);

        CHECK(*optFile->peek() == 'A');
        CHECK(optFile->tellPos(pos));
        CHECK(pos == 0); // The peek must not advance the visible position.

        optFile->consume();
        CHECK(optFile->tellPos(pos));
        CHECK(pos == 1);
    }

    SECTION("seekPos drops the peek cache")
    {
        auto optFile = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optFile.hasValue());

        CHECK(*optFile->peek() == 'A');
        CHECK(optFile->seekPos(2)); // jump to 'C'
        CHECK(*optFile->peek() == 'C');
    }

    SECTION("peek at EOF returns nullOpt")
    {
        auto optFile = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optFile.hasValue());

        CHECK(optFile->seekPos(4));
        CHECK_FALSE(optFile->peek().hasValue());
        CHECK(optFile->isEOF());
    }
}


TEST_CASE("[System] sf::InFile as sf::base::ScnSource")
{
    using namespace sf::base::literals;

    SECTION("scnReadLine over a multi-KiB file")
    {
        // Big enough to exercise the FILE* internal buffer transitioning
        // mid-line (stdio default buffer is typically 4-8 KiB).
        sf::base::String          payload;
        constexpr sf::base::SizeT longLineSize = 16u * 1024u;
        payload.reserve(longLineSize + 16u);
        for (sf::base::SizeT i = 0u; i < longLineSize; ++i)
            payload.append('x');
        payload.append("\nshort\n");

        const TemporaryFile temporaryFile(payload.toStringView());

        auto optFile = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optFile.hasValue());

        sf::base::String line;
        CHECK(sf::base::scnReadLine(*optFile, line));
        CHECK(line.size() == longLineSize);

        CHECK(sf::base::scnReadLine(*optFile, line));
        CHECK(line == sf::base::String{"short"});

        CHECK(!sf::base::scnReadLine(*optFile, line));
        CHECK(sf::base::scnAtEnd(*optFile));
    }

    SECTION("scn<int> / scn<String> from a file")
    {
        const TemporaryFile temporaryFile("42 hello\n3.14 world"_sv);

        auto optFile = sf::InFile::open(temporaryFile.getPath(), sf::FileOpenMode::bin);
        REQUIRE(optFile.hasValue());

        const auto v = sf::base::scn<int>(*optFile);
        REQUIRE(v.hasValue());
        CHECK(*v == 42);

        const auto tok = sf::base::scn<sf::base::String>(*optFile);
        REQUIRE(tok.hasValue());
        CHECK(*tok == sf::base::String{"hello"});
    }
}


TEST_CASE("[System] sf::writeToFile and sf::readFromFile")
{
    using namespace sf::base::literals;

    SECTION("Round-trip")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "Hello world"_sv));

        std::string contents;
        CHECK(sf::readFromFile(path, contents));
        CHECK(contents == "Hello world");
    }

    SECTION("Read empty file")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, ""_sv));

        std::string contents = "stale";
        CHECK(sf::readFromFile(path, contents));
        CHECK(contents.empty());
    }

    SECTION("Read missing file fails")
    {
        const TemporaryFile temporaryFile;
        // The file does not exist on disk yet.
        const sf::Path& path = temporaryFile.getPath();

        std::string contents;
        CHECK(!sf::readFromFile(path, contents));
    }

    SECTION("Binary content with embedded NULs and high bytes is preserved")
    {
        // The native fast path goes through a single buffered read, so make sure
        // it doesn't truncate at NUL or mangle non-ASCII bytes.
        const char                 raw[] = {'a', '\0', 'b', '\xff', '\x01', '\x80', 'z'};
        const sf::base::StringView payload(raw, sizeof(raw));

        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, payload));

        std::string contents;
        CHECK(sf::readFromFile(path, contents));
        CHECK(contents.size() == sizeof(raw));
        CHECK(sf::base::StringView(contents.data(), contents.size()) == payload);
    }

    SECTION("Large file (multiple MiB) round-trips byte-exactly")
    {
        // Sized to comfortably exceed any plausible internal buffer in the
        // fallback iostream path (typically 4-8 KiB) and force the native paths
        // to issue more than one read/ReadFile call's worth of work.
        constexpr sf::base::SizeT bytes = 4u * 1024u * 1024u + 17u; // 4 MiB + odd tail

        std::string payload;
        payload.resize(bytes);

        // Deterministic but non-trivial content; covers all 256 byte values.
        sf::base::SizeT seed = 0x12'34'56'78u;
        for (sf::base::SizeT i = 0; i < bytes; ++i)
        {
            seed       = seed * 1'103'515'245u + 12'345u;
            payload[i] = static_cast<char>(seed >> 16);
        }

        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, sf::base::StringView(payload.data(), payload.size())));

        std::string contents;
        CHECK(sf::readFromFile(path, contents));
        REQUIRE(contents.size() == payload.size());
        CHECK(contents == payload);
    }

    SECTION("Read overwrites pre-existing content in target")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "short"_sv));

        std::string contents = "this is a much longer pre-existing string that should be overwritten";
        CHECK(sf::readFromFile(path, contents));
        CHECK(contents == "short");
    }

    SECTION("Round-trip into base::String")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "Hello base::String"_sv));

        sf::base::String contents;
        CHECK(sf::readFromFile(path, contents));
        CHECK(contents == sf::base::String{"Hello base::String"});
    }

    SECTION("Read empty file into base::String")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, ""_sv));

        sf::base::String contents{"stale"};
        CHECK(sf::readFromFile(path, contents));
        CHECK(contents.empty());
    }

    SECTION("Read missing file into base::String fails")
    {
        const TemporaryFile temporaryFile;
        // File never created.
        const sf::Path& path = temporaryFile.getPath();

        sf::base::String contents;
        CHECK(!sf::readFromFile(path, contents));
    }

    SECTION("Round-trip into base::Vector<char>")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "Hello vector"_sv));

        sf::base::Vector<char> contents;
        CHECK(sf::readFromFile(path, contents));
        REQUIRE(contents.size() == 12u);
        CHECK(sf::base::StringView(contents.data(), contents.size()) == "Hello vector"_sv);
    }

    SECTION("Read empty file into base::Vector<char>")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, ""_sv));

        sf::base::Vector<char> contents;
        // Pre-populate to verify that an empty read clears existing content.
        contents.pushBack('x');
        contents.pushBack('y');
        CHECK(sf::readFromFile(path, contents));
        CHECK(contents.empty());
    }

    SECTION("Read missing file into base::Vector<char> fails")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        sf::base::Vector<char> contents;
        CHECK(!sf::readFromFile(path, contents));
    }

    SECTION("Read into base::Vector<char> reuses capacity across calls")
    {
        // Two reads into the same Vector: the second (smaller) read should not
        // grow capacity, and the first's allocation should already be amortized.
        const TemporaryFile temporaryFile1;
        const TemporaryFile temporaryFile2;
        const sf::Path&     path1 = temporaryFile1.getPath();
        const sf::Path&     path2 = temporaryFile2.getPath();

        // Make file1 large enough to definitely allocate, file2 strictly smaller.
        std::string big(1024u, 'A');
        CHECK(sf::writeToFile(path1, sf::base::StringView{big.data(), big.size()}));
        CHECK(sf::writeToFile(path2, "small"_sv));

        sf::base::Vector<char> buffer;
        CHECK(sf::readFromFile(path1, buffer));
        const auto capacityAfterFirst = buffer.capacity();
        REQUIRE(buffer.size() == 1024u);

        CHECK(sf::readFromFile(path2, buffer));
        CHECK(buffer.size() == 5u);
        CHECK(sf::base::StringView(buffer.data(), buffer.size()) == "small"_sv);
        // The smaller read must NOT shrink or re-grow capacity.
        CHECK(buffer.capacity() == capacityAfterFirst);
    }

    SECTION("Read into base::Vector<char> overwrites pre-existing content")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "abc"_sv));

        sf::base::Vector<char> buffer;
        for (char c : {'X', 'Y', 'Z', 'W', 'V'})
            buffer.pushBack(c);

        CHECK(sf::readFromFile(path, buffer));
        CHECK(buffer.size() == 3u);
        CHECK(sf::base::StringView(buffer.data(), buffer.size()) == "abc"_sv);
    }

    SECTION("Binary content with embedded NULs round-trips through base::Vector<char>")
    {
        const char                 raw[] = {'a', '\0', 'b', '\xff', '\x01', '\x80', 'z'};
        const sf::base::StringView payload(raw, sizeof(raw));

        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, payload));

        sf::base::Vector<char> contents;
        CHECK(sf::readFromFile(path, contents));
        REQUIRE(contents.size() == sizeof(raw));
        CHECK(sf::base::StringView(contents.data(), contents.size()) == payload);
    }

    SECTION("StringView overloads")
    {
        // Exercise the `base::StringView` filename overloads: the rest of the
        // suite uses the `Path` overloads; these tests cover the alternate API.
        const TemporaryFile        temporaryFile;
        const auto                 pathOwning = temporaryFile.getPath().to<std::string>();
        const sf::base::StringView pathView{pathOwning.data(), pathOwning.size()};

        CHECK(sf::writeToFile(pathView, "Hello via StringView"_sv));

        SECTION("Read into std::string via StringView")
        {
            std::string contents;
            CHECK(sf::readFromFile(pathView, contents));
            CHECK(contents == "Hello via StringView");
        }

        SECTION("Read into base::String via StringView")
        {
            sf::base::String contents;
            CHECK(sf::readFromFile(pathView, contents));
            CHECK(contents == sf::base::String{"Hello via StringView"});
        }

        SECTION("Read into base::Vector<char> via StringView")
        {
            sf::base::Vector<char> contents;
            CHECK(sf::readFromFile(pathView, contents));
            REQUIRE(contents.size() == 20u);
            CHECK(sf::base::StringView(contents.data(), contents.size()) == "Hello via StringView"_sv);
        }
    }
}


TEST_CASE("[System] sf::appendFromFile")
{
    using namespace sf::base::literals;

    SECTION("Append into empty target equals readFromFile")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "alpha"_sv));

        sf::base::Vector<char> contents;
        CHECK(sf::appendFromFile(path, contents));
        REQUIRE(contents.size() == 5u);
        CHECK(sf::base::StringView(contents.data(), contents.size()) == "alpha"_sv);
    }

    SECTION("Append preserves existing content")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "world"_sv));

        sf::base::Vector<char> contents;
        for (char c : {'h', 'e', 'l', 'l', 'o', ' '})
            contents.pushBack(c);

        CHECK(sf::appendFromFile(path, contents));
        REQUIRE(contents.size() == 11u);
        CHECK(sf::base::StringView(contents.data(), contents.size()) == "hello world"_sv);
    }

    SECTION("Multiple successive appends concatenate cleanly")
    {
        const TemporaryFile temporaryFile1;
        const TemporaryFile temporaryFile2;
        const TemporaryFile temporaryFile3;

        CHECK(sf::writeToFile(temporaryFile1.getPath(), "foo"_sv));
        CHECK(sf::writeToFile(temporaryFile2.getPath(), "bar"_sv));
        CHECK(sf::writeToFile(temporaryFile3.getPath(), "baz"_sv));

        sf::base::Vector<char> contents;
        CHECK(sf::appendFromFile(temporaryFile1.getPath(), contents));
        CHECK(sf::appendFromFile(temporaryFile2.getPath(), contents));
        CHECK(sf::appendFromFile(temporaryFile3.getPath(), contents));

        REQUIRE(contents.size() == 9u);
        CHECK(sf::base::StringView(contents.data(), contents.size()) == "foobarbaz"_sv);
    }

    SECTION("Append from empty file leaves target unchanged")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, ""_sv));

        sf::base::Vector<char> contents;
        for (char c : {'k', 'e', 'e', 'p'})
            contents.pushBack(c);

        CHECK(sf::appendFromFile(path, contents));
        REQUIRE(contents.size() == 4u);
        CHECK(sf::base::StringView(contents.data(), contents.size()) == "keep"_sv);
    }

    SECTION("Append from missing file fails and does not touch target")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        sf::base::Vector<char> contents;
        for (char c : {'i', 'n', 't', 'a', 'c', 't'})
            contents.pushBack(c);

        CHECK(!sf::appendFromFile(path, contents));
        REQUIRE(contents.size() == 6u);
        CHECK(sf::base::StringView(contents.data(), contents.size()) == "intact"_sv);
    }

    SECTION("Append via StringView filename overload")
    {
        const TemporaryFile        temporaryFile;
        const auto                 pathOwning = temporaryFile.getPath().to<std::string>();
        const sf::base::StringView pathView{pathOwning.data(), pathOwning.size()};

        CHECK(sf::writeToFile(pathView, " world"_sv));

        sf::base::Vector<char> contents;
        for (char c : {'h', 'e', 'l', 'l', 'o'})
            contents.pushBack(c);

        CHECK(sf::appendFromFile(pathView, contents));
        REQUIRE(contents.size() == 11u);
        CHECK(sf::base::StringView(contents.data(), contents.size()) == "hello world"_sv);
    }
}


TEST_CASE("[System] sf::getThreadLocalScratchCharBuffer")
{
    using namespace sf::base::literals;

    SECTION("Returns the same reference on repeated calls (same thread)")
    {
        sf::base::Vector<char>& a = sf::getThreadLocalScratchCharBuffer();
        sf::base::Vector<char>& b = sf::getThreadLocalScratchCharBuffer();
        CHECK(&a == &b);
    }

    SECTION("Buffer is usable as a readFromFile target")
    {
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        CHECK(sf::writeToFile(path, "scratch payload"_sv));

        sf::base::Vector<char>& scratch = sf::getThreadLocalScratchCharBuffer();
        CHECK(sf::readFromFile(path, scratch));
        CHECK(sf::base::StringView(scratch.data(), scratch.size()) == "scratch payload"_sv);
    }

    SECTION("Capacity is preserved across reads on the same thread")
    {
        // Successive smaller reads must reuse the buffer that the first read
        // grew, since this is the whole point of the scratch.
        const TemporaryFile temporaryFile;
        const sf::Path&     path = temporaryFile.getPath();

        std::string big(2048u, 'q');
        CHECK(sf::writeToFile(path, sf::base::StringView{big.data(), big.size()}));

        sf::base::Vector<char>& scratch = sf::getThreadLocalScratchCharBuffer();
        CHECK(sf::readFromFile(path, scratch));
        const auto capacityAfterBig = scratch.capacity();
        REQUIRE(capacityAfterBig >= 2048u);

        CHECK(sf::writeToFile(path, "tiny"_sv));
        CHECK(sf::readFromFile(path, scratch));
        CHECK(scratch.size() == 4u);
        CHECK(scratch.capacity() == capacityAfterBig); // not shrunk, not regrown
    }
}


TEST_CASE("[System] sf::FileOpenMode")
{
    SECTION("Bitwise operations")
    {
        constexpr auto combined = sf::FileOpenMode::in | sf::FileOpenMode::bin;
        STATIC_CHECK((combined & sf::FileOpenMode::in) == sf::FileOpenMode::in);
        STATIC_CHECK((combined & sf::FileOpenMode::bin) == sf::FileOpenMode::bin);
        STATIC_CHECK((combined & sf::FileOpenMode::out) == sf::FileOpenMode::none);
    }
}
