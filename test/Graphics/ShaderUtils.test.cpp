#include "StringifyStringViewUtil.hpp"
#include "TemporaryFile.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/ShaderUtils.hpp"

#include "Zancle/Fmt/FmtToString.hpp"

#include "Zancle/IO/IO.hpp"
#include "Zancle/IO/Path.hpp"

#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"

#include "Zancle/Container/Vector.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/PtrDiffT.hpp"


using za::testing::TemporaryFile;

namespace
{
////////////////////////////////////////////////////////////
za::StringView bufferToView(const za::Vector<char>& buffer)
{
    return {buffer.data(), buffer.size()};
}

} // namespace


TEST_CASE("[Graphics] za::ShaderUtils::parseIncludeDirective")
{
    SECTION("Not an include -- regular GLSL lines")
    {
        const auto r0 = za::ShaderUtils::parseIncludeDirective("uniform float time;");
        CHECK(r0.hasValue());
        CHECK(r0->empty());

        const auto r1 = za::ShaderUtils::parseIncludeDirective("");
        CHECK(r1.hasValue());
        CHECK(r1->empty());

        const auto r2 = za::ShaderUtils::parseIncludeDirective("void main() {}");
        CHECK(r2.hasValue());
        CHECK(r2->empty());

        const auto r3 = za::ShaderUtils::parseIncludeDirective("// just a comment");
        CHECK(r3.hasValue());
        CHECK(r3->empty());

        const auto r4 = za::ShaderUtils::parseIncludeDirective("#define FOO 1");
        CHECK(r4.hasValue());
        CHECK(r4->empty());

        const auto r5 = za::ShaderUtils::parseIncludeDirective("#ifdef FOO");
        CHECK(r5.hasValue());
        CHECK(r5->empty());
    }

    SECTION("Not an include -- #includeFoo (no word boundary)")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#includeFoo");
        CHECK(r.hasValue());
        CHECK(r->empty());
    }

    SECTION("Successful parse -- basic")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include \"common.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "common.glsl");
    }

    SECTION("Successful parse -- leading whitespace")
    {
        const auto r0 = za::ShaderUtils::parseIncludeDirective("  #include \"foo.glsl\"");
        CHECK(r0.hasValue());
        CHECK(*r0 == "foo.glsl");

        const auto r1 = za::ShaderUtils::parseIncludeDirective("\t#include \"bar.glsl\"");
        CHECK(r1.hasValue());
        CHECK(*r1 == "bar.glsl");

        const auto r2 = za::ShaderUtils::parseIncludeDirective("  \t  #include \"baz.glsl\"");
        CHECK(r2.hasValue());
        CHECK(*r2 == "baz.glsl");
    }

    SECTION("Successful parse -- extra whitespace between keyword and filename")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include   \"spaced.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "spaced.glsl");
    }

    SECTION("Successful parse -- no space before quote")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include\"nospace.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "nospace.glsl");
    }

    SECTION("Successful parse -- path with subdirectory")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include \"sub/dir/file.glsl\"");
        CHECK(r.hasValue());
        CHECK(*r == "sub/dir/file.glsl");
    }

    SECTION("Successful parse -- trailing content after closing quote")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include \"file.glsl\" // comment");
        CHECK(r.hasValue());
        CHECK(*r == "file.glsl");
    }

    SECTION("Error -- missing opening quote (angle bracket)")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include <system.glsl>");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- missing opening quote (bare identifier)")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include foo.glsl");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- missing closing quote")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include \"unterminated.glsl");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- empty filename")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include \"\"");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- #include with nothing after it")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include");
        CHECK_FALSE(r.hasValue());
    }

    SECTION("Error -- #include with only whitespace after it")
    {
        const auto r = za::ShaderUtils::parseIncludeDirective("#include   ");
        CHECK_FALSE(r.hasValue());
    }
}


TEST_CASE("[Graphics] za::ShaderUtils::emitLineDirective")
{
    SECTION("Line 1")
    {
        za::Vector<char> buffer;
        za::ShaderUtils::emitLineDirective(buffer, 1);

        CHECK(bufferToView(buffer) == "#line 1\n");
    }

    SECTION("Line 42")
    {
        za::Vector<char> buffer;
        za::ShaderUtils::emitLineDirective(buffer, 42);

        CHECK(bufferToView(buffer) == "#line 42\n");
    }

    SECTION("Line 0")
    {
        za::Vector<char> buffer;
        za::ShaderUtils::emitLineDirective(buffer, 0);

        CHECK(bufferToView(buffer) == "#line 0\n");
    }

    SECTION("Appends to existing buffer")
    {
        za::Vector<char> buffer;
        buffer.pushBack('X');
        za::ShaderUtils::emitLineDirective(buffer, 5);

        CHECK(bufferToView(buffer) == "X#line 5\n");
    }
}


TEST_CASE("[Graphics] za::ShaderUtils::preprocessGlslIncludes")
{
    SECTION("No includes -- passthrough")
    {
        const za::StringView source = "uniform float time;\nvoid main() {}\n";

        TemporaryFile mainFile(source);

        za::Vector<char> output;
        CHECK(za::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));

        const auto result = bufferToView(output);

        // Should contain #line 1 at the start and all original lines
        CHECK(result.find("#line 1") != za::StringView::nPos);
        CHECK(result.find("uniform float time;") != za::StringView::nPos);
        CHECK(result.find("void main() {}") != za::StringView::nPos);
    }

    SECTION("Single include")
    {
        TemporaryFile includedFile("float helper() { return 1.0; }\n");

        // Build the #include line pointing to the temp file's filename
        const auto includedFilename = includedFile.getPath().getFilename().to<za::String>();

        za::String source;
        source.append("uniform float time;\n");
        source.append("#include \"");
        source.append(includedFilename);
        source.append("\"\n");
        source.append("void main() {}\n");

        // Main file in the same directory as the included file
        TemporaryFile mainFile(source.toStringView());

        za::Vector<char> output;
        CHECK(za::ShaderUtils::preprocessGlslIncludes(source.toStringView(), mainFile.getPath(), output));

        const auto result = bufferToView(output);

        // Should contain the included content
        CHECK(result.find("float helper()") != za::StringView::nPos);
        // Should contain #line directives to restore numbering
        CHECK(result.find("#line 3") != za::StringView::nPos);
        // Should contain the line after the include
        CHECK(result.find("void main() {}") != za::StringView::nPos);
    }

    SECTION("Missing include file returns false")
    {
        const za::StringView source = "#include \"nonexistent_file_12345.glsl\"\n";

        TemporaryFile mainFile(source);

        za::Vector<char> output;
        CHECK_FALSE(za::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));
    }

    SECTION("Malformed include returns false")
    {
        const za::StringView source = "#include <bad.glsl>\n";

        TemporaryFile mainFile(source);

        za::Vector<char> output;
        CHECK_FALSE(za::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));
    }

    SECTION("Circular include returns false")
    {
        // Create two files that include each other
        // We need to know filenames beforehand, so create them in two steps

        // Disambiguate temp-file names across parallel test processes
        const auto nameA = za::fmtToString("zancle_circular_a_{}.glsl", za::testing::getProcessUniqueId());
        const auto nameB = za::fmtToString("zancle_circular_b_{}.glsl", za::testing::getProcessUniqueId());

        const auto pathA = za::Path::getTempDirectory().value() / za::Path(nameA);
        const auto pathB = za::Path::getTempDirectory().value() / za::Path(nameB);

        {
            const auto                  contentA = za::fmtToString("#include \"{}\"\n", nameB);
            auto                        optFile  = za::OutFile::open(pathA);
            [[maybe_unused]] const bool ok       = optFile->write(contentA.data(), contentA.size());
        }

        {
            const auto                  contentB = za::fmtToString("#include \"{}\"\n", nameA);
            auto                        optFile  = za::OutFile::open(pathB);
            [[maybe_unused]] const bool ok       = optFile->write(contentB.data(), contentB.size());
        }

        const auto source = za::fmtToString("#include \"{}\"\n", nameB);

        za::Vector<char> output;
        CHECK_FALSE(za::ShaderUtils::preprocessGlslIncludes(source.toStringView(), pathA, output));

        // Cleanup
        [[maybe_unused]] const bool removedA = pathA.removeFromDisk();
        [[maybe_unused]] const bool removedB = pathB.removeFromDisk();
    }

    SECTION("Nested includes")
    {
        TemporaryFile innerFile("int inner_val = 42;\n");

        const auto innerFilename = innerFile.getPath().getFilename().to<za::String>();

        za::String outerSource;
        outerSource.append("#include \"");
        outerSource.append(innerFilename);
        outerSource.append("\"\n");
        outerSource.append("int outer_val = 1;\n");

        TemporaryFile outerFile(outerSource.toStringView());

        const auto outerFilename = outerFile.getPath().getFilename().to<za::String>();

        za::String mainSource;
        mainSource.append("#include \"");
        mainSource.append(outerFilename);
        mainSource.append("\"\n");
        mainSource.append("void main() {}\n");

        TemporaryFile mainFile(mainSource.toStringView());

        za::Vector<char> output;
        CHECK(za::ShaderUtils::preprocessGlslIncludes(mainSource.toStringView(), mainFile.getPath(), output));

        const auto result = bufferToView(output);

        CHECK(result.find("int inner_val = 42;") != za::StringView::nPos);
        CHECK(result.find("int outer_val = 1;") != za::StringView::nPos);
        CHECK(result.find("void main() {}") != za::StringView::nPos);
    }

    SECTION("Source with trailing null is handled")
    {
        const char           sourceWithNull[] = "uniform float x;\n\0";
        const za::StringView source{sourceWithNull, sizeof(sourceWithNull) - 1}; // includes the \0

        TemporaryFile mainFile(source);

        za::Vector<char> output;
        CHECK(za::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));

        const auto result = bufferToView(output);
        CHECK(result.find("uniform float x;") != za::StringView::nPos);
    }

    SECTION("Windows line endings (CRLF) are handled")
    {
        const za::StringView source = "uniform float a;\r\nvoid main() {}\r\n";

        TemporaryFile mainFile(source);

        za::Vector<char> output;
        CHECK(za::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));

        const auto result = bufferToView(output);
        CHECK(result.find("uniform float a;") != za::StringView::nPos);
        CHECK(result.find("void main() {}") != za::StringView::nPos);
        // Should not contain \r in the output
        CHECK(result.find('\r') == za::StringView::nPos);
    }

    SECTION("Empty source")
    {
        const za::StringView source = "";

        TemporaryFile mainFile("");

        za::Vector<char> output;
        CHECK(za::ShaderUtils::preprocessGlslIncludes(source, mainFile.getPath(), output));
    }

    SECTION("#line directives preserve correct line numbers")
    {
        TemporaryFile includedFile("// included line 1\n// included line 2\n");

        const auto includedFilename = includedFile.getPath().getFilename().to<za::String>();

        za::String source;
        source.append("// line 1\n"); // line 1
        source.append("// line 2\n"); // line 2
        source.append("#include \""); // line 3
        source.append(includedFilename);
        source.append("\"\n");
        source.append("// line 4\n"); // line 4

        TemporaryFile mainFile(source.toStringView());

        za::Vector<char> output;
        CHECK(za::ShaderUtils::preprocessGlslIncludes(source.toStringView(), mainFile.getPath(), output));

        const auto result = bufferToView(output);

        // After the include, #line 4 should be emitted to restore parent numbering
        CHECK(result.find("#line 4") != za::StringView::nPos);
    }
}
