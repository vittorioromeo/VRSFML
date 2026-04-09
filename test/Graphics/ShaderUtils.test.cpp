#include "StringifyStringViewUtil.hpp"
#include "TemporaryFile.hpp"

#include "SFML/Graphics/ShaderUtils.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


using sf::testing::TemporaryFile;

namespace
{
////////////////////////////////////////////////////////////
sf::base::StringView bufferToView(const sf::base::Vector<char>& buffer)
{
    return {buffer.data(), buffer.size()};
}

} // namespace


TEST_CASE("[Graphics] sf::ShaderUtils::parseIncludeDirective")
{
    SECTION("Not an include -- regular GLSL lines")
    {
        const auto r0 = sf::ShaderUtils::parseIncludeDirective("uniform float time;");
        CHECK(r0.hasValue());
        CHECK(r0->empty());

        const auto r1 = sf::ShaderUtils::parseIncludeDirective("");
        CHECK(r1.hasValue());
        CHECK(r1->empty());

        const auto r2 = sf::ShaderUtils::parseIncludeDirective("void main() {}");
        CHECK(r2.hasValue());
        CHECK(r2->empty());

        const auto r3 = sf::ShaderUtils::parseIncludeDirective("// just a comment");
        CHECK(r3.hasValue());
        CHECK(r3->empty());

        const auto r4 = sf::ShaderUtils::parseIncludeDirective("#define FOO 1");
        CHECK(r4.hasValue());
        CHECK(r4->empty());

        const auto r5 = sf::ShaderUtils::parseIncludeDirective("#ifdef FOO");
        CHECK(r5.hasValue());
        CHECK(r5->empty());
    }

    SECTION("Not an include -- #includeFoo (no word boundary)")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#includeFoo");
        CHECK(r.hasValue());
        CHECK(r->empty());
    }

    SECTION("Successful parse -- basic")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include \"common.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "common.glsl");
    }

    SECTION("Successful parse -- leading whitespace")
    {
        const auto r0 = sf::ShaderUtils::parseIncludeDirective("  #include \"foo.glsl\"");
        CHECK(r0.hasValue());
        CHECK(*r0 == "foo.glsl");

        const auto r1 = sf::ShaderUtils::parseIncludeDirective("\t#include \"bar.glsl\"");
        CHECK(r1.hasValue());
        CHECK(*r1 == "bar.glsl");

        const auto r2 = sf::ShaderUtils::parseIncludeDirective("  \t  #include \"baz.glsl\"");
        CHECK(r2.hasValue());
        CHECK(*r2 == "baz.glsl");
    }

    SECTION("Successful parse -- extra whitespace between keyword and filename")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include   \"spaced.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "spaced.glsl");
    }

    SECTION("Successful parse -- no space before quote")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include\"nospace.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "nospace.glsl");
    }

    SECTION("Successful parse -- path with subdirectory")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include \"sub/dir/file.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "sub/dir/file.glsl");
    }

    SECTION("Successful parse -- trailing content after closing quote")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include \"file.glsl\" // comment");
        CHECK(r.hasValue());
        CHECK(*r == "file.glsl");
    }

    SECTION("Error -- missing opening quote (angle bracket)")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include <system.glsl>");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- missing opening quote (bare identifier)")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include foo.glsl");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- missing closing quote")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include \"unterminated.glsl");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- empty filename")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include \"\"");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- #include with nothing after it")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- #include with only whitespace after it")
    {
        const auto r = sf::ShaderUtils::parseIncludeDirective("#include   ");
        CHECK_FALSE(r.hasValue());
    }
}


TEST_CASE("[Graphics] sf::ShaderUtils::emitLineDirective")
{
    SECTION("Line 1")
    {
        sf::base::Vector<char> buffer;
        sf::ShaderUtils::emitLineDirective(buffer, 1);

        CHECK(bufferToView(buffer) == "#line 1\n");
    }

    SECTION("Line 42")
    {
        sf::base::Vector<char> buffer;
        sf::ShaderUtils::emitLineDirective(buffer, 42);

        CHECK(bufferToView(buffer) == "#line 42\n");
    }

    SECTION("Line 0")
    {
        sf::base::Vector<char> buffer;
        sf::ShaderUtils::emitLineDirective(buffer, 0);

        CHECK(bufferToView(buffer) == "#line 0\n");
    }

    SECTION("Appends to existing buffer")
    {
        sf::base::Vector<char> buffer;
        buffer.pushBack('X');
        sf::ShaderUtils::emitLineDirective(buffer, 5);

        CHECK(bufferToView(buffer) == "X#line 5\n");
    }
}


TEST_CASE("[Graphics] sf::ShaderUtils::preprocessGlslIncludes")
{
    SECTION("No includes -- passthrough")
    {
        const sf::base::StringView source = "uniform float time;\nvoid main() {}\n";

        TemporaryFile mainFile(source);

        sf::base::Vector<char> output;
        CHECK(sf::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));

        const auto result = bufferToView(output);

        // Should contain #line 1 at the start and all original lines
        CHECK(result.find("#line 1") != sf::base::StringView::nPos);
        CHECK(result.find("uniform float time;") != sf::base::StringView::nPos);
        CHECK(result.find("void main() {}") != sf::base::StringView::nPos);
    }

    SECTION("Single include")
    {
        TemporaryFile includedFile("float helper() { return 1.0; }\n");

        // Build the #include line pointing to the temp file's filename
        const auto includedFilename = includedFile.getPath().filename().to<sf::base::String>();

        sf::base::String source;
        source.append("uniform float time;\n");
        source.append("#include \"");
        source.append(includedFilename);
        source.append("\"\n");
        source.append("void main() {}\n");

        // Main file in the same directory as the included file
        TemporaryFile mainFile(source.toStringView());

        sf::base::Vector<char> output;
        CHECK(sf::ShaderUtils::preprocessGlslIncludes(source.toStringView(), mainFile.getPath(), output));

        const auto result = bufferToView(output);

        // Should contain the included content
        CHECK(result.find("float helper()") != sf::base::StringView::nPos);
        // Should contain #line directives to restore numbering
        CHECK(result.find("#line 3") != sf::base::StringView::nPos);
        // Should contain the line after the include
        CHECK(result.find("void main() {}") != sf::base::StringView::nPos);
    }

    SECTION("Missing include file returns false")
    {
        const sf::base::StringView source = "#include \"nonexistent_file_12345.glsl\"\n";

        TemporaryFile mainFile(source);

        sf::base::Vector<char> output;
        CHECK_FALSE(sf::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));
    }

    SECTION("Malformed include returns false")
    {
        const sf::base::StringView source = "#include <bad.glsl>\n";

        TemporaryFile mainFile(source);

        sf::base::Vector<char> output;
        CHECK_FALSE(sf::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));
    }

    SECTION("Circular include returns false")
    {
        // Create two files that include each other
        // We need to know filenames beforehand, so create them in two steps

        // First, create file A that includes file B
        const auto pathA = sf::Path::tempDirectoryPath() / "sfml_circular_a.glsl";
        const auto pathB = sf::Path::tempDirectoryPath() / "sfml_circular_b.glsl";

        {
            constexpr sf::base::StringView contentA = "#include \"sfml_circular_b.glsl\"\n";
            sf::OutFileStream              ofs(pathA);
            ofs.write(contentA.data(), static_cast<sf::base::PtrDiffT>(contentA.size()));
        }

        {
            constexpr sf::base::StringView contentB = "#include \"sfml_circular_a.glsl\"\n";
            sf::OutFileStream              ofs(pathB);
            ofs.write(contentB.data(), static_cast<sf::base::PtrDiffT>(contentB.size()));
        }

        const sf::base::StringView source = "#include \"sfml_circular_b.glsl\"\n";

        sf::base::Vector<char> output;
        CHECK_FALSE(sf::ShaderUtils::preprocessGlslIncludes(source, pathA, output));

        // Cleanup
        [[maybe_unused]] const bool removedA = pathA.removeFromDisk();
        [[maybe_unused]] const bool removedB = pathB.removeFromDisk();
    }

    SECTION("Nested includes")
    {
        TemporaryFile innerFile("int inner_val = 42;\n");

        const auto innerFilename = innerFile.getPath().filename().to<sf::base::String>();

        sf::base::String outerSource;
        outerSource.append("#include \"");
        outerSource.append(innerFilename);
        outerSource.append("\"\n");
        outerSource.append("int outer_val = 1;\n");

        TemporaryFile outerFile(outerSource.toStringView());

        const auto outerFilename = outerFile.getPath().filename().to<sf::base::String>();

        sf::base::String mainSource;
        mainSource.append("#include \"");
        mainSource.append(outerFilename);
        mainSource.append("\"\n");
        mainSource.append("void main() {}\n");

        TemporaryFile mainFile(mainSource.toStringView());

        sf::base::Vector<char> output;
        CHECK(sf::ShaderUtils::preprocessGlslIncludes(mainSource.toStringView(), mainFile.getPath(), output));

        const auto result = bufferToView(output);

        CHECK(result.find("int inner_val = 42;") != sf::base::StringView::nPos);
        CHECK(result.find("int outer_val = 1;") != sf::base::StringView::nPos);
        CHECK(result.find("void main() {}") != sf::base::StringView::nPos);
    }

    SECTION("Source with trailing null is handled")
    {
        const char                 sourceWithNull[] = "uniform float x;\n\0";
        const sf::base::StringView source{sourceWithNull, sizeof(sourceWithNull) - 1}; // includes the \0

        TemporaryFile mainFile(source);

        sf::base::Vector<char> output;
        CHECK(sf::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));

        const auto result = bufferToView(output);
        CHECK(result.find("uniform float x;") != sf::base::StringView::nPos);
    }

    SECTION("Windows line endings (CRLF) are handled")
    {
        const sf::base::StringView source = "uniform float a;\r\nvoid main() {}\r\n";

        TemporaryFile mainFile(source);

        sf::base::Vector<char> output;
        CHECK(sf::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));

        const auto result = bufferToView(output);
        CHECK(result.find("uniform float a;") != sf::base::StringView::nPos);
        CHECK(result.find("void main() {}") != sf::base::StringView::nPos);
        // Should not contain \r in the output
        CHECK(result.find('\r') == sf::base::StringView::nPos);
    }

    SECTION("Empty source")
    {
        const sf::base::StringView source = "";

        TemporaryFile mainFile("");

        sf::base::Vector<char> output;
        CHECK(sf::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));
    }

    SECTION("#line directives preserve correct line numbers")
    {
        TemporaryFile includedFile("// included line 1\n// included line 2\n");

        const auto includedFilename = includedFile.getPath().filename().to<sf::base::String>();

        sf::base::String source;
        source.append("// line 1\n"); // line 1
        source.append("// line 2\n"); // line 2
        source.append("#include \""); // line 3
        source.append(includedFilename);
        source.append("\"\n");
        source.append("// line 4\n"); // line 4

        TemporaryFile mainFile(source.toStringView());

        sf::base::Vector<char> output;
        CHECK(sf::ShaderUtils::preprocessGlslIncludes(source.toStringView(), mainFile.getPath(), output));

        const auto result = bufferToView(output);

        // After the include, #line 4 should be emitted to restore parent numbering
        CHECK(result.find("#line 4") != sf::base::StringView::nPos);
    }
}
