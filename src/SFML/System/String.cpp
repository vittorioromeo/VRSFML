#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/String.hpp>
#include <SFML/System/Utf.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>

#include <locale>
#include <string>

#include <cstring>
#include <cwchar>


namespace sf
{
////////////////////////////////////////////////////////////
struct String::Impl
{
    std::u32string string; //!< Internal string of UTF-32 characters
};


////////////////////////////////////////////////////////////
String::String() = default;


////////////////////////////////////////////////////////////
String::~String() = default;


////////////////////////////////////////////////////////////
String::String(const String&) = default;


////////////////////////////////////////////////////////////
String& String::operator=(const String&) = default;


////////////////////////////////////////////////////////////
String::String(String&&) noexcept = default;


////////////////////////////////////////////////////////////
String& String::operator=(String&&) noexcept = default;


////////////////////////////////////////////////////////////
String::String(char ansiChar) : String(ansiChar, std::locale{})
{
}


////////////////////////////////////////////////////////////
String::String(char ansiChar, const std::locale& locale)
{
    m_impl->string += Utf32::decodeAnsi(ansiChar, locale);
}


////////////////////////////////////////////////////////////
String::String(wchar_t wideChar)
{
    m_impl->string += Utf32::decodeWide(wideChar);
}


////////////////////////////////////////////////////////////
String::String(char32_t utf32Char)
{
    m_impl->string += utf32Char;
}


////////////////////////////////////////////////////////////
String::String(const char* ansiString) : String(ansiString, std::locale{})
{
}


////////////////////////////////////////////////////////////
String::String(const char* ansiString, const std::locale& locale)
{
    if (ansiString)
    {
        const std::size_t length = std::strlen(ansiString);
        if (length > 0)
        {
            m_impl->string.reserve(length + 1);
            Utf32::fromAnsi(ansiString, ansiString + length, base::BackInserter(m_impl->string), locale);
        }
    }
}


////////////////////////////////////////////////////////////
String::String(const std::string& ansiString) : String(ansiString, std::locale{})
{
}


////////////////////////////////////////////////////////////
String::String(const std::string& ansiString, const std::locale& locale)
{
    m_impl->string.reserve(ansiString.length() + 1);
    Utf32::fromAnsi(ansiString.begin(), ansiString.end(), base::BackInserter(m_impl->string), locale);
}


////////////////////////////////////////////////////////////
String::String(const wchar_t* wideString)
{
    if (wideString)
    {
        const std::size_t length = std::wcslen(wideString);
        if (length > 0)
        {
            m_impl->string.reserve(length + 1);
            Utf32::fromWide(wideString, wideString + length, base::BackInserter(m_impl->string));
        }
    }
}


////////////////////////////////////////////////////////////
String::String(const std::wstring& wideString)
{
    m_impl->string.reserve(wideString.length() + 1);
    Utf32::fromWide(wideString.begin(), wideString.end(), base::BackInserter(m_impl->string));
}


////////////////////////////////////////////////////////////
String::String(const char32_t* utf32String)
{
    if (utf32String)
        m_impl->string = utf32String;
}


////////////////////////////////////////////////////////////
String::String(std::u32string utf32String) : m_impl(SFML_BASE_MOVE(utf32String))
{
}


////////////////////////////////////////////////////////////
String::operator std::string() const
{
    return toAnsiString();
}


////////////////////////////////////////////////////////////
String::operator std::wstring() const
{
    return toWideString();
}


////////////////////////////////////////////////////////////
std::string String::toAnsiString() const
{
    return toAnsiString(std::locale{});
}


////////////////////////////////////////////////////////////
std::string String::toAnsiString(const std::locale& locale) const
{
    // Prepare the output string
    std::string output;
    output.reserve(m_impl->string.length() + 1);

    // Convert
    Utf32::toAnsi(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output), 0, locale);

    return output;
}


////////////////////////////////////////////////////////////
std::wstring String::toWideString() const
{
    // Prepare the output string
    std::wstring output;
    output.reserve(m_impl->string.length() + 1);

    // Convert
    Utf32::toWide(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output), 0);

    return output;
}


////////////////////////////////////////////////////////////
std::u8string String::toUtf8() const
{
    // Prepare the output string
    std::u8string output;
    output.reserve(m_impl->string.length());

    // Convert
    Utf32::toUtf8(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output));

    return output;
}


////////////////////////////////////////////////////////////
std::u16string String::toUtf16() const
{
    // Prepare the output string
    std::u16string output;
    output.reserve(m_impl->string.length());

    // Convert
    Utf32::toUtf16(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output));

    return output;
}


////////////////////////////////////////////////////////////
std::u32string String::toUtf32() const
{
    return m_impl->string;
}


////////////////////////////////////////////////////////////
String& String::operator+=(const String& right)
{
    m_impl->string += right.m_impl->string;
    return *this;
}


////////////////////////////////////////////////////////////
char32_t String::operator[](std::size_t index) const
{
    SFML_BASE_ASSERT(index < m_impl->string.size() && "Index is out of bounds");
    return m_impl->string[index];
}


////////////////////////////////////////////////////////////
char32_t& String::operator[](std::size_t index)
{
    SFML_BASE_ASSERT(index < m_impl->string.size() && "Index is out of bounds");
    return m_impl->string[index];
}


////////////////////////////////////////////////////////////
void String::clear()
{
    m_impl->string.clear();
}


////////////////////////////////////////////////////////////
std::size_t String::getSize() const
{
    return m_impl->string.size();
}


////////////////////////////////////////////////////////////
bool String::isEmpty() const
{
    return m_impl->string.empty();
}


////////////////////////////////////////////////////////////
void String::erase(std::size_t position, std::size_t count)
{
    m_impl->string.erase(position, count);
}


////////////////////////////////////////////////////////////
void String::insert(std::size_t position, const String& str)
{
    m_impl->string.insert(position, str.m_impl->string);
}


////////////////////////////////////////////////////////////
std::size_t String::find(const String& str, std::size_t start) const
{
    return m_impl->string.find(str.m_impl->string, start);
}


////////////////////////////////////////////////////////////
void String::replace(std::size_t position, std::size_t length, const String& replaceWith)
{
    m_impl->string.replace(position, length, replaceWith.m_impl->string);
}


////////////////////////////////////////////////////////////
void String::replace(const String& searchFor, const String& replaceWith)
{
    const std::size_t step = replaceWith.getSize();
    const std::size_t len  = searchFor.getSize();
    std::size_t       pos  = find(searchFor);

    // Replace each occurrence of search
    while (pos != InvalidPos)
    {
        replace(pos, len, replaceWith);
        pos = find(searchFor, pos + step);
    }
}


////////////////////////////////////////////////////////////
String String::substring(std::size_t position, std::size_t length) const
{
    return m_impl->string.substr(position, length);
}


////////////////////////////////////////////////////////////
const char32_t* String::getData() const
{
    return m_impl->string.c_str();
}


////////////////////////////////////////////////////////////
String::Iterator String::begin()
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front());
}


////////////////////////////////////////////////////////////
String::ConstIterator String::begin() const
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front());
}


////////////////////////////////////////////////////////////
String::Iterator String::end()
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front()) + m_impl->string.size();
}


////////////////////////////////////////////////////////////
String::ConstIterator String::end() const
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front()) + m_impl->string.size();
}


////////////////////////////////////////////////////////////
std::u32string& String::getImplString()
{
    return m_impl->string;
}


////////////////////////////////////////////////////////////
bool operator==(const String& left, const String& right)
{
    return left.m_impl->string == right.m_impl->string;
}


////////////////////////////////////////////////////////////
bool operator!=(const String& left, const String& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
bool operator<(const String& left, const String& right)
{
    return left.m_impl->string < right.m_impl->string;
}


////////////////////////////////////////////////////////////
bool operator>(const String& left, const String& right)
{
    return right < left;
}


////////////////////////////////////////////////////////////
bool operator<=(const String& left, const String& right)
{
    return !(right < left);
}


////////////////////////////////////////////////////////////
bool operator>=(const String& left, const String& right)
{
    return !(left < right);
}


////////////////////////////////////////////////////////////
String operator+(const String& left, const String& right)
{
    String string = left;
    string += right;

    return string;
}


////////////////////////////////////////////////////////////
const std::size_t String::InvalidPos{std::u32string::npos};

} // namespace sf
