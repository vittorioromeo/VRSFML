#include "StringifySfBaseStringUtil.hpp"
#include "StringifyStdStringUtil.hpp"
#include "StringifyStringViewUtil.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>

#include <string>


namespace
{
sf::Path getTemporaryFilePath()
{
    static int counter = 0;

    sf::OutStringStream oss;
    oss << "sfmltemp_io_" << counter++ << ".tmp";

    return sf::Path::tempDirectoryPath() / oss.to<sf::base::String>();
}

class TemporaryFile
{
public:
    // Reserve a unique path without creating any file on disk.
    explicit TemporaryFile() : m_path(getTemporaryFilePath())
    {
    }

    // Create a temporary file with a randomly generated path, containing 'contents'.
    explicit TemporaryFile(const sf::base::StringView contents) : m_path(getTemporaryFilePath())
    {
        sf::OutFileStream ofs(m_path);
        SFML_BASE_ASSERT(ofs && "Stream encountered an error");

        ofs << contents;
        SFML_BASE_ASSERT(ofs && "Stream encountered an error");
    }

    // Close and delete the generated file (if it still exists).
    ~TemporaryFile()
    {
        if (m_path.exists())
        {
            [[maybe_unused]] const bool removed = m_path.removeFromDisk();
            SFML_BASE_ASSERT(removed && "m_path failed to be removed from filesystem");
        }
    }

    // Prevent copies.
    TemporaryFile(const TemporaryFile&)            = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    // Return the randomly generated path.
    [[nodiscard]] const sf::Path& getPath() const
    {
        return m_path;
    }

private:
    sf::Path m_path;
};
} // namespace


TEST_CASE("[System] sf::OutStringStream")
{
    using namespace sf::base::literals;

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::OutStringStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::OutStringStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::OutStringStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::OutStringStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::OutStringStream));
    }

    SECTION("Default construction")
    {
        const sf::OutStringStream oss;
        CHECK(oss.isGood());
        CHECK(static_cast<bool>(oss));
        CHECK(oss.getString().empty());
    }

    SECTION("Write fundamental types")
    {
        sf::OutStringStream oss;
        oss << "Hello " << 42 << ' ' << true;
        CHECK(oss.getString() == "Hello 42 1");
    }

    SECTION("Write base::StringView")
    {
        SECTION("Empty")
        {
            sf::OutStringStream oss;
            oss << ""_sv;
            CHECK(oss.getString().empty());
        }

        SECTION("Single value")
        {
            sf::OutStringStream oss;
            oss << "abc"_sv;
            CHECK(oss.getString() == "abc");
        }

        SECTION("Chained with other types")
        {
            sf::OutStringStream oss;
            oss << "prefix:"_sv << 42 << '|' << "suffix"_sv;
            CHECK(oss.getString() == "prefix:42|suffix");
        }

        SECTION("Embedded null bytes are preserved")
        {
            const char                 data[] = {'a', '\0', 'b'};
            const sf::base::StringView view(data, 3);

            sf::OutStringStream oss;
            oss << view;

            const std::string result = oss.getString();
            CHECK(result.size() == 3);
            CHECK(result[0] == 'a');
            CHECK(result[1] == '\0');
            CHECK(result[2] == 'b');
        }
    }

    SECTION("Write base::String")
    {
        SECTION("Empty")
        {
            sf::OutStringStream oss;
            oss << sf::base::String{};
            CHECK(oss.getString().empty());
        }

        SECTION("Single value")
        {
            sf::OutStringStream oss;
            oss << sf::base::String{"hello"};
            CHECK(oss.getString() == "hello");
        }

        SECTION("Chained with other types")
        {
            sf::OutStringStream oss;
            oss << "n="_sv << 7 << ' ' << sf::base::String{"value"};
            CHECK(oss.getString() == "n=7 value");
        }

        SECTION("Multiple base::String writes")
        {
            sf::OutStringStream oss;
            oss << sf::base::String{"foo"} << sf::base::String{"bar"} << sf::base::String{"baz"};
            CHECK(oss.getString() == "foobarbaz");
        }

        SECTION("Embedded null bytes are preserved")
        {
            const char             data[] = {'x', '\0', 'y'};
            const sf::base::String s(data, 3);

            sf::OutStringStream oss;
            oss << s;

            const std::string result = oss.getString();
            CHECK(result.size() == 3);
            CHECK(result[0] == 'x');
            CHECK(result[1] == '\0');
            CHECK(result[2] == 'y');
        }
    }

    SECTION("to<T>() conversion")
    {
        sf::OutStringStream oss;
        oss << "answer=" << 42;

        CHECK(oss.to<std::string>() == "answer=42");
        CHECK(oss.to<sf::base::String>() == sf::base::String{"answer=42"});
    }

    SECTION("setStr replaces buffer contents")
    {
        sf::OutStringStream oss;
        oss << "garbage";
        oss.setStr("clean"_sv);
        CHECK(oss.getString() == "clean");
    }

    SECTION("Manipulators - SetWidth and SetFill")
    {
        sf::OutStringStream oss;
        oss << sf::SetFill{'0'} << sf::SetWidth{4} << 7;
        CHECK(oss.getString() == "0007");
    }

    SECTION("Manipulators - Hex")
    {
        sf::OutStringStream oss;
        oss << sf::Hex{} << 255;
        CHECK(oss.getString() == "ff");
    }

    SECTION("setPrecision")
    {
        sf::OutStringStream oss;
        oss.setPrecision(3);
        oss << 3.14159;
        CHECK(oss.getString() == "3.14");
    }

    SECTION("setFormatFlags")
    {
        sf::OutStringStream oss;
        oss.setFormatFlags(sf::FormatFlags::boolalpha);
        oss << true;
        CHECK(oss.getString() == "true");
    }

    SECTION("Move semantics")
    {
        SECTION("Move constructor")
        {
            sf::OutStringStream moved;
            moved << "moved";
            sf::OutStringStream oss = SFML_BASE_MOVE(moved);
            CHECK(oss.getString() == "moved");
        }

        SECTION("Move assignment")
        {
            sf::OutStringStream moved;
            moved << "moved";
            sf::OutStringStream oss;
            oss << "original";
            oss = SFML_BASE_MOVE(moved);
            CHECK(oss.getString() == "moved");
        }
    }
}


TEST_CASE("[System] sf::InStringStream")
{
    using namespace sf::base::literals;

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::InStringStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::InStringStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::InStringStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::InStringStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::InStringStream));
    }

    SECTION("Default construction")
    {
        sf::InStringStream iss;
        CHECK(iss.isGood());
        CHECK(static_cast<bool>(iss));
        CHECK(!iss.isEOF());
    }

    SECTION("Construct from std::string")
    {
        sf::InStringStream iss(std::string{"42 hello"});
        int                value = 0;
        std::string        word;
        iss >> value >> word;
        CHECK(value == 42);
        CHECK(word == "hello");
    }

    SECTION("Construct from base::String")
    {
        sf::InStringStream iss(sf::base::String{"3.5 world"});
        float              value = 0.f;
        sf::base::String   word;
        iss >> value >> word;
        CHECK(value == 3.5f);
        CHECK(word == sf::base::String{"world"});
    }

    SECTION("get and read")
    {
        sf::InStringStream iss(std::string{"abcdef"});

        char ch = '\0';
        iss.get(ch);
        CHECK(ch == 'a');

        char buffer[3] = {};
        iss.read(buffer, 3);
        CHECK(iss.gcount() == 3);
        CHECK(sf::base::StringView(buffer, 3) == "bcd"_sv);
    }

    SECTION("ignore advances past delimiter")
    {
        sf::InStringStream iss(std::string{"prefix:value"});
        iss.ignore(100, ':');

        std::string rest;
        iss >> rest;
        CHECK(rest == "value");
    }

    SECTION("tellg, EOF and clear")
    {
        sf::InStringStream iss(std::string{"abc"});

        CHECK(iss.tellg() == 0);

        char buffer[4] = {};
        iss.read(buffer, 3);
        CHECK(iss.tellg() == 3);
        CHECK(iss.gcount() == 3);

        // One more read should hit EOF.
        char extra = '\0';
        iss.get(extra);
        CHECK(iss.isEOF());
        CHECK(!iss.isGood());

        iss.clear();
        CHECK(iss.isGood());
    }

    SECTION("Hex extraction")
    {
        sf::InStringStream iss(std::string{"ff"});
        int                value = 0;
        iss >> sf::Hex{} >> value;
        CHECK(value == 255);
    }

    SECTION("Move semantics")
    {
        SECTION("Move constructor")
        {
            sf::InStringStream moved(std::string{"42"});
            sf::InStringStream iss   = SFML_BASE_MOVE(moved);
            int                value = 0;
            iss >> value;
            CHECK(value == 42);
        }

        SECTION("Move assignment")
        {
            sf::InStringStream moved(std::string{"42"});
            sf::InStringStream iss(std::string{"99"});
            iss       = SFML_BASE_MOVE(moved);
            int value = 0;
            iss >> value;
            CHECK(value == 42);
        }
    }
}


TEST_CASE("[System] sf::OutFileStream and sf::InFileStream")
{
    using namespace sf::base::literals;

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::OutFileStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::OutFileStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::OutFileStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::OutFileStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::OutFileStream));

        STATIC_CHECK(SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::InFileStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::InFileStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::InFileStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::InFileStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::InFileStream));
    }

    SECTION("Default-constructed OutFileStream is not open")
    {
        const sf::OutFileStream ofs;
        CHECK(!ofs.isOpen());
    }

    SECTION("Default-constructed InFileStream is not open")
    {
        const sf::InFileStream ifs;
        CHECK(!ifs.isOpen());
    }

    SECTION("Write then read text")
    {
        const TemporaryFile temporaryFile;

        {
            sf::OutFileStream ofs(temporaryFile.getPath());
            CHECK(ofs.isOpen());
            CHECK(ofs.isGood());
            CHECK(static_cast<bool>(ofs));

            ofs << "Hello "_sv << 42 << ' ' << "world"_sv;
            ofs.flush();
            ofs.close();
            CHECK(!ofs.isOpen());
        }

        {
            sf::InFileStream ifs(temporaryFile.getPath(), sf::FileOpenMode::in);
            CHECK(ifs.isOpen());
            CHECK(ifs.isGood());
            CHECK(static_cast<bool>(ifs));

            std::string word;
            int         number = 0;
            ifs >> word >> number;
            CHECK(word == "Hello");
            CHECK(number == 42);

            ifs >> word;
            CHECK(word == "world");
        }
    }

    SECTION("OutFileStream tellPos and seekPos")
    {
        const TemporaryFile temporaryFile;

        sf::OutFileStream ofs(temporaryFile.getPath());
        ofs << "abcdef"_sv;
        CHECK(ofs.tellPos() == 6);

        ofs.seekPos(3);
        CHECK(ofs.tellPos() == 3);

        ofs.write("XYZ", 3);
        CHECK(ofs.tellPos() == 6);
        ofs.close();

        sf::InFileStream ifs(temporaryFile.getPath(), sf::FileOpenMode::in);
        char             buffer[7] = {};
        ifs.read(buffer, 6);
        CHECK(ifs.gcount() == 6);
        CHECK(sf::base::StringView(buffer, 6) == "abcXYZ"_sv);
    }

    SECTION("InFileStream seekg and tellg")
    {
        const TemporaryFile temporaryFile("Hello world");

        sf::InFileStream ifs(temporaryFile.getPath(), sf::FileOpenMode::in);
        CHECK(ifs.tellg() == 0);

        ifs.seekg(6);
        CHECK(ifs.tellg() == 6);

        char buffer[6] = {};
        ifs.read(buffer, 5);
        CHECK(ifs.gcount() == 5);
        CHECK(sf::base::StringView(buffer, 5) == "world"_sv);

        ifs.seekg(0, sf::SeekDir::beg);
        CHECK(ifs.tellg() == 0);

        ifs.seekg(0, sf::SeekDir::end);
        CHECK(ifs.tellg() == 11);
    }

    SECTION("InFileStream EOF")
    {
        const TemporaryFile temporaryFile("abc");

        sf::InFileStream ifs(temporaryFile.getPath(), sf::FileOpenMode::in);
        char             buffer[8] = {};
        ifs.read(buffer, 8);
        CHECK(ifs.isEOF());
    }

    SECTION("Open with explicit mode")
    {
        const TemporaryFile temporaryFile;

        sf::OutFileStream ofs;
        CHECK(!ofs.isOpen());
        ofs.open(temporaryFile.getPath(), sf::FileOpenMode::out | sf::FileOpenMode::bin);
        CHECK(ofs.isOpen());

        const char data[] = {'b', 'i', 'n'};
        ofs.write(data, 3);
        ofs.close();

        sf::InFileStream ifs;
        ifs.open(temporaryFile.getPath(), sf::FileOpenMode::in | sf::FileOpenMode::bin);
        CHECK(ifs.isOpen());

        char buffer[3] = {};
        ifs.read(buffer, 3);
        CHECK(sf::base::StringView(buffer, 3) == "bin"_sv);
    }

    SECTION("Move semantics")
    {
        const TemporaryFile temporaryFile("Hello world");

        SECTION("OutFileStream move constructor")
        {
            const TemporaryFile target;

            sf::OutFileStream moved(target.getPath());
            sf::OutFileStream ofs = SFML_BASE_MOVE(moved);
            CHECK(ofs.isOpen());
            ofs << "moved"_sv;
            ofs.close();

            sf::InFileStream ifs(target.getPath(), sf::FileOpenMode::in);
            char             buffer[5] = {};
            ifs.read(buffer, 5);
            CHECK(sf::base::StringView(buffer, 5) == "moved"_sv);
        }

        SECTION("InFileStream move constructor")
        {
            sf::InFileStream moved(temporaryFile.getPath(), sf::FileOpenMode::in);
            sf::InFileStream ifs = SFML_BASE_MOVE(moved);
            CHECK(ifs.isOpen());

            char buffer[5] = {};
            ifs.read(buffer, 5);
            CHECK(sf::base::StringView(buffer, 5) == "Hello"_sv);
        }
    }
}


TEST_CASE("[System] sf::writeToFile and sf::readFromFile")
{
    using namespace sf::base::literals;

    SECTION("Round-trip")
    {
        const TemporaryFile temporaryFile;
        const auto          pathStr = temporaryFile.getPath().to<std::string>();

        CHECK(sf::writeToFile(pathStr, "Hello world"_sv));

        std::string contents;
        CHECK(sf::readFromFile(pathStr, contents));
        CHECK(contents == "Hello world");
    }

    SECTION("Read empty file")
    {
        const TemporaryFile temporaryFile;
        const auto          pathStr = temporaryFile.getPath().to<std::string>();

        CHECK(sf::writeToFile(pathStr, ""_sv));

        std::string contents = "stale";
        CHECK(sf::readFromFile(pathStr, contents));
        CHECK(contents.empty());
    }

    SECTION("Read missing file fails")
    {
        const TemporaryFile temporaryFile;
        // The file does not exist on disk yet.
        const auto pathStr = temporaryFile.getPath().to<std::string>();

        std::string contents;
        CHECK(!sf::readFromFile(pathStr, contents));
    }
}


TEST_CASE("[System] sf::getLine")
{
    using namespace sf::base::literals;

    SECTION("Read lines into std::string")
    {
        sf::InStringStream iss(std::string{"first\nsecond\nthird"});

        std::string line;

        CHECK(sf::getLine(iss, line));
        CHECK(line == "first");

        CHECK(sf::getLine(iss, line));
        CHECK(line == "second");

        CHECK(sf::getLine(iss, line));
        CHECK(line == "third");

        CHECK(!sf::getLine(iss, line));
    }

    SECTION("Read lines into base::String")
    {
        sf::InStringStream iss(std::string{"alpha\nbeta"});

        sf::base::String line;

        CHECK(sf::getLine(iss, line));
        CHECK(line == sf::base::String{"alpha"});

        CHECK(sf::getLine(iss, line));
        CHECK(line == sf::base::String{"beta"});

        CHECK(!sf::getLine(iss, line));
    }
}


TEST_CASE("[System] sf::IOStreamOutput")
{
    using namespace sf::base::literals;

    // `IOStreamOutput` is intended to be used through `sf::cOut()` / `sf::cErr()`,
    // so we redirect those streams' rdbuf to a local `OutStringStream` for the
    // duration of each section and restore the original buffer afterwards.
    auto* const previousBuffer = sf::cOut().rdbuf();
    CHECK(previousBuffer != nullptr);

    SECTION("Identity of cOut, cErr, cIn")
    {
        CHECK(&sf::cOut() == &sf::cOut());
        CHECK(&sf::cErr() == &sf::cErr());
        CHECK(&sf::cIn() == &sf::cIn());
        CHECK(sf::cErr().rdbuf() != nullptr);
    }

    SECTION("Redirect cOut to OutStringStream and write fundamentals")
    {
        const sf::OutStringStream stream;
        sf::cOut().rdbuf(stream.rdbuf());

        sf::cOut() << "Hello " << 42;
        sf::cOut().flush();
        CHECK(stream.to<std::string>() == "Hello 42");
    }

    SECTION("rdbuf setter redirects output")
    {
        const sf::OutStringStream first;
        const sf::OutStringStream second;

        sf::cOut().rdbuf(first.rdbuf());
        sf::cOut() << "to first";
        sf::cOut().flush();

        sf::cOut().rdbuf(second.rdbuf());
        sf::cOut() << "to second";
        sf::cOut().flush();

        CHECK(first.to<std::string>() == "to first");
        CHECK(second.to<std::string>() == "to second");
    }

    SECTION("endL writes newline and flushes")
    {
        const sf::OutStringStream stream;
        sf::cOut().rdbuf(stream.rdbuf());

        sf::cOut() << "line" << sf::endL;
        CHECK(stream.to<std::string>() == "line\n");
    }

    SECTION("flush manipulator")
    {
        const sf::OutStringStream stream;
        sf::cOut().rdbuf(stream.rdbuf());

        sf::cOut() << "data" << sf::flush;
        CHECK(stream.to<std::string>() == "data");
    }

    // Restore the original buffer so subsequent tests are not affected.
    sf::cOut().rdbuf(previousBuffer);
    CHECK(sf::cOut().rdbuf() == previousBuffer);
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


TEST_CASE("[System] sf::FormatFlags")
{
    SECTION("Bitwise operations")
    {
        constexpr auto combined = sf::FormatFlags::hex | sf::FormatFlags::showbase;
        STATIC_CHECK((combined & sf::FormatFlags::hex) == sf::FormatFlags::hex);
        STATIC_CHECK((combined & sf::FormatFlags::showbase) == sf::FormatFlags::showbase);
        STATIC_CHECK((combined & sf::FormatFlags::dec) == sf::FormatFlags::none);
    }

    SECTION("Composite masks")
    {
        STATIC_CHECK((sf::FormatFlags::adjustfield & sf::FormatFlags::left) == sf::FormatFlags::left);
        STATIC_CHECK((sf::FormatFlags::adjustfield & sf::FormatFlags::right) == sf::FormatFlags::right);
        STATIC_CHECK((sf::FormatFlags::adjustfield & sf::FormatFlags::internal) == sf::FormatFlags::internal);

        STATIC_CHECK((sf::FormatFlags::basefield & sf::FormatFlags::dec) == sf::FormatFlags::dec);
        STATIC_CHECK((sf::FormatFlags::basefield & sf::FormatFlags::oct) == sf::FormatFlags::oct);
        STATIC_CHECK((sf::FormatFlags::basefield & sf::FormatFlags::hex) == sf::FormatFlags::hex);

        STATIC_CHECK((sf::FormatFlags::floatfield & sf::FormatFlags::scientific) == sf::FormatFlags::scientific);
        STATIC_CHECK((sf::FormatFlags::floatfield & sf::FormatFlags::fixed) == sf::FormatFlags::fixed);
    }
}
