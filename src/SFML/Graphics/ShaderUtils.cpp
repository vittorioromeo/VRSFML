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
#include "SFML/Base/PtrDiffT.hpp"
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
    sf::InFileStream file(path.c_str(), sf::FileOpenMode::bin);

    if (!file)
        return sf::base::nullOpt;

    file.seekg(0, sf::SeekDir::end);
    const auto size = file.tellg();

    sf::base::Vector<char> buffer;

    if (size > 0)
    {
        file.seekg(0, sf::SeekDir::beg);
        buffer.resize(static_cast<sf::base::SizeT>(size));
        file.read(buffer.data(), static_cast<sf::base::PtrDiffT>(size));
    }

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
        sf::priv::err() << "GLSL #include depth limit exceeded (" << maxGlslIncludeDepth << ")";
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
            const sf::Path includePath = (basePath / sf::Path(static_cast<const char*>(filenameBuf))).absolute();

            // Check for circular includes
            for (const auto& stackPath : includeStack)
            {
                if (stackPath == includePath)
                {
                    sf::priv::err() << "Circular GLSL #include detected for '" << static_cast<const char*>(filenameBuf)
                                    << "':\n";

                    for (const auto& p : includeStack)
                        sf::priv::err(/* multiLine */ true) << "  " << p << " ->\n";

                    sf::priv::err(/* multiLine */ true) << "  " << includePath << '\n';
                    return false;
                }
            }

            // Read the included file
            auto optFileContents = readFileContents(includePath);

            if (!optFileContents.hasValue())
            {
                sf::priv::err() << "Failed to open GLSL #include file '" << static_cast<const char*>(filenameBuf) << "'";

                if (!includeStack.empty())
                {
                    sf::priv::err(/* multiLine */ true) << "\n  Include stack:\n";

                    for (const auto& p : includeStack)
                        sf::priv::err(/* multiLine */ true) << "    " << p << '\n';
                }

                return false;
            }

            // Get included source
            sf::base::StringView includedSource{optFileContents->data(), optFileContents->size()};

            // Push to include stack and recursively process
            includeStack.pushBack(includePath);

            if (!preprocessGlslIncludesImpl(includedSource, includePath.parent(), output, includeStack, depth + 1))
                return false;

            includeStack.popBack();

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
        priv::err() << "Malformed GLSL #include directive (" << what << "): " << line;
        return base::nullOpt;
    };

    const auto notAnInclude = base::makeOptional<base::StringView>();

    base::SizeT pos = 0;

    // Skip leading whitespace
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t'))
        ++pos;

    // Check for #include
    constexpr base::StringView includeKeyword{"#include"};

    if (line.size() - pos < includeKeyword.size())
        return notAnInclude;

    if (line.substrByPosLen(pos, includeKeyword.size()) != includeKeyword)
        return notAnInclude;

    pos += includeKeyword.size();

    // Must be followed by whitespace or quote (not e.g. #includeFoo)
    if (pos < line.size() && line[pos] != ' ' && line[pos] != '\t' && line[pos] != '"')
        return notAnInclude;

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

    const Path absoluteShaderPath = shaderPath.absolute();

    base::Vector<Path> includeStack;
    includeStack.pushBack(absoluteShaderPath);

    return preprocessGlslIncludesImpl(source, absoluteShaderPath.parent(), output, includeStack, 0);
}

} // namespace sf
