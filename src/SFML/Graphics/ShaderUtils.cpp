// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/ShaderUtils.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr unsigned int maxGlslIncludeDepth      = 32;
constexpr unsigned int maxIncludeFilenameLength = 256;


////////////////////////////////////////////////////////////
// Read the full contents of a file into a Vector<char>.
[[nodiscard]] sf::base::Optional<sf::base::Vector<char>> readFileContents(const sf::Path& path)
{
    sf::base::Vector<char> buffer;
    if (!sf::readFromFile(path, buffer))
        return sf::base::nullOpt;

    return sf::base::makeOptional(SFML_BASE_MOVE(buffer));
}


////////////////////////////////////////////////////////////
// Recursively preprocess GLSL #include directives
[[nodiscard]] bool preprocessGlslIncludesImpl(
    sf::base::StringView        source,
    const sf::Path&             basePath,
    sf::base::Vector<char>&     output,
    sf::base::Vector<sf::Path>& includeStack,
    unsigned int                depth)
{
    if (depth > maxGlslIncludeDepth)
    {
        sf::priv::errMsg("GLSL #include depth limit exceeded ({})", maxGlslIncludeDepth);
        return false;
    }

    // Emit #line directive to set correct line numbering for this file
    sf::ShaderUtils::emitLineDirective(output, 1);

    unsigned int    lineNumber = 0;
    sf::base::SizeT lineStart  = 0;

    while (lineStart < source.size())
    {
        // Find end of current line
        sf::base::SizeT lineEnd  = source.find('\n', lineStart);
        const bool      lastLine = (lineEnd == sf::base::StringView::nPos);

        if (lastLine)
            lineEnd = source.size();

        ++lineNumber;

        // Extract line content (without \n), strip trailing \r
        sf::base::StringView line = source.substrByPosLen(lineStart, lineEnd - lineStart);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.removeSuffix(1);

        // Check for #include directive
        const sf::base::Optional<sf::base::StringView> optIncludedFilename = sf::ShaderUtils::parseIncludeDirective(line);

        if (!optIncludedFilename.hasValue())
            return false; // Malformed #include directive

        const sf::base::StringView& includedFilename = *optIncludedFilename;

        if (!includedFilename.empty())
        {
            // Build null-terminated filename for Path construction
            SFML_BASE_ASSERT(includedFilename.size() < maxIncludeFilenameLength && "Include filename too long");

            char filenameBuf[maxIncludeFilenameLength];
            SFML_BASE_MEMCPY(filenameBuf, includedFilename.data(), includedFilename.size());
            filenameBuf[includedFilename.size()] = '\0';

            // Resolve include path relative to base directory
            const auto absMaybe = (basePath / sf::Path(static_cast<const char*>(filenameBuf))).getAbsolute();
            if (!absMaybe)
            {
                sf::priv::errMsg("Failed to resolve absolute path for GLSL #include '{}'",
                                 static_cast<const char*>(filenameBuf));
                return false;
            }
            const sf::Path& includePath = *absMaybe;

            // Check for circular includes
            for (const auto& stackPath : includeStack)
            {
                if (stackPath == includePath)
                {
                    sf::priv::errMsg("Circular GLSL #include detected for '{}':\n", static_cast<const char*>(filenameBuf));

                    for (const auto& p : includeStack)
                        sf::priv::errMsgMulti("  {} ->\n", p);

                    sf::priv::errMsgMulti("  {}{}", includePath, '\n');
                    return false;
                }
            }

            // Read the included file
            auto optFileContents = readFileContents(includePath);

            if (!optFileContents.hasValue())
            {
                sf::priv::errMsg("Failed to open GLSL #include file '{}'", static_cast<const char*>(filenameBuf));

                if (!includeStack.empty())
                {
                    sf::priv::errMsgMulti("\n  Include stack:\n");

                    for (const auto& p : includeStack)
                        sf::priv::errMsgMulti("    {}{}", p, '\n');
                }

                return false;
            }

            // Get included source
            sf::base::StringView includedSource{optFileContents->data(), optFileContents->size()};

            // Emit begin-include marker
            {
                constexpr sf::base::StringView prefix{"// >>> begin included from \""};
                constexpr sf::base::StringView suffix{"\" >>>\n"};

                output.emplaceRange(prefix.data(), prefix.size());
                output.emplaceRange(filenameBuf, includedFilename.size());
                output.emplaceRange(suffix.data(), suffix.size());
            }

            // Push to include stack and recursively process
            includeStack.pushBack(includePath);

            if (!preprocessGlslIncludesImpl(includedSource, includePath.getParent(), output, includeStack, depth + 1))
                return false;

            includeStack.popBack();

            // Emit end-include marker
            {
                constexpr sf::base::StringView prefix{"// <<< end included from \""};
                constexpr sf::base::StringView suffix{"\" <<<\n"};

                output.emplaceRange(prefix.data(), prefix.size());
                output.emplaceRange(filenameBuf, includedFilename.size());
                output.emplaceRange(suffix.data(), suffix.size());
            }

            // Restore line numbering for parent file
            sf::ShaderUtils::emitLineDirective(output, lineNumber + 1);
        }
        else
        {
            // Copy line as-is
            if (!line.empty())
                output.emplaceRange(line.data(), line.size());

            output.pushBack('\n');
        }

        if (lastLine)
            break;

        lineStart = lineEnd + 1;
    }

    return true;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
void ShaderUtils::emitLineDirective(base::Vector<char>& buffer, unsigned int lineNumber)
{
    constexpr base::StringView prefix{"#line "};

    char        tmp[16];
    char* const end = base::toChars(tmp, tmp + sizeof(tmp), lineNumber);
    SFML_BASE_ASSERT(end != nullptr);

    buffer.emplaceRange(prefix.data(), prefix.size());
    buffer.emplaceRange(tmp, static_cast<base::SizeT>(end - tmp));
    buffer.pushBack('\n');
}


////////////////////////////////////////////////////////////
base::Optional<base::StringView> ShaderUtils::parseIncludeDirective(base::StringView line)
{
    const auto fail = [&](const char* what)
    {
        priv::errMsg("Malformed GLSL #include directive ({}): {}", what, line);
        return base::nullOpt;
    };

    base::SizeT pos = 0;

    // Skip leading whitespace
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t'))
        ++pos;

    // Check for #include
    constexpr base::StringView includeKeyword{"#include"};

    if (line.size() - pos < includeKeyword.size())
        return base::makeOptional<base::StringView>(); // not an include

    if (line.substrByPosLen(pos, includeKeyword.size()) != includeKeyword)
        return base::makeOptional<base::StringView>(); // not an include

    pos += includeKeyword.size();

    // Must be followed by whitespace or quote (not e.g. #includeFoo)
    if (pos < line.size() && line[pos] != ' ' && line[pos] != '\t' && line[pos] != '"')
        return base::makeOptional<base::StringView>(); // not an include

    // At this point we know it's an #include directive -- any further issue is a hard error

    // Skip whitespace after #include
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t'))
        ++pos;

    // Expect opening double quote
    if (pos >= line.size() || line[pos] != '"')
        return fail("expected '\"'");

    ++pos;

    // Find closing double quote
    const base::SizeT filenameStart = pos;
    while (pos < line.size() && line[pos] != '"')
        ++pos;

    if (pos >= line.size())
        return fail("missing closing '\"'");

    if (pos == filenameStart)
        return fail("empty filename");

    return base::makeOptional<base::StringView>(line.substrByPosLen(filenameStart, pos - filenameStart));
}


////////////////////////////////////////////////////////////
bool ShaderUtils::preprocessGlslIncludes(base::StringView source, const Path& shaderPath, base::Vector<char>& output)
{
    output.clear();

    // Strip trailing null that may be present from file reading
    if (!source.empty() && source[source.size() - 1] == '\0')
        source.removeSuffix(1);

    const auto absMaybe = shaderPath.getAbsolute();
    if (!absMaybe)
    {
        priv::errMsg("Failed to resolve absolute path for shader");
        return false;
    }
    const Path& absoluteShaderPath = *absMaybe;

    base::Vector<Path> includeStack;
    includeStack.pushBack(absoluteShaderPath);

    return preprocessGlslIncludesImpl(source, absoluteShaderPath.getParent(), output, includeStack, 0);
}

} // namespace sf
