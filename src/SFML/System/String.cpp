#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/String.hpp"
#include "SFML/System/Utf.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Strlen.hpp"

#include <locale>
#include <string>

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
String::String(char ansiChar, const priv::LocaleLike auto& locale)
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
String::String(const char* ansiString, const priv::LocaleLike auto& locale)
{
    if (ansiString)
    {
        const base::SizeT length = SFML_BASE_STRLEN(ansiString);
        if (length > 0)
        {
            m_impl->string.reserve(length + 1);
            Utf32::fromAnsi(ansiString, ansiString + length, base::BackInserter(m_impl->string), locale);
        }
    }
}


////////////////////////////////////////////////////////////
String::String(const priv::AnsiStringLike auto& ansiString) : String(ansiString, std::locale{})
{
}


////////////////////////////////////////////////////////////
String::String(const priv::AnsiStringLike auto& ansiString, const priv::LocaleLike auto& locale)
{
    m_impl->string.reserve(ansiString.length() + 1);
    Utf32::fromAnsi(ansiString.begin(), ansiString.end(), base::BackInserter(m_impl->string), locale);
}


////////////////////////////////////////////////////////////
String::String(const wchar_t* wideString)
{
    if (wideString)
    {
        const base::SizeT length = std::wcslen(wideString);
        if (length > 0)
        {
            m_impl->string.reserve(length + 1);
            Utf32::fromWide(wideString, wideString + length, base::BackInserter(m_impl->string));
        }
    }
}


////////////////////////////////////////////////////////////
String::String(const priv::WStringLike auto& wideString)
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
String::String(const priv::U32StringLike auto& utf32String) : m_impl(utf32String)
{
}


////////////////////////////////////////////////////////////
template <priv::AnsiStringLike TString>
TString String::toAnsiString() const
{
    return toAnsiString<std::string>(std::locale{});
}


////////////////////////////////////////////////////////////
template <priv::AnsiStringLike TString>
TString String::toAnsiString(const priv::LocaleLike auto& locale) const
{
    // Prepare the output string
    std::string output;
    output.reserve(m_impl->string.length() + 1);

    // Convert
    Utf32::toAnsi(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output), 0, locale);

    return output;
}


////////////////////////////////////////////////////////////
template <priv::WStringLike TString>
TString String::toWideString() const
{
    // Prepare the output string
    std::wstring output;
    output.reserve(m_impl->string.length() + 1);

    // Convert
    Utf32::toWide(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output), 0);

    return output;
}


////////////////////////////////////////////////////////////
template <priv::U8StringLike TString>
TString String::toUtf8() const
{
    // Prepare the output string
    std::u8string output;
    output.reserve(m_impl->string.length());

    // Convert
    Utf32::toUtf8(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output));

    return output;
}


////////////////////////////////////////////////////////////
template <priv::U16StringLike TString>
TString String::toUtf16() const
{
    // Prepare the output string
    std::u16string output;
    output.reserve(m_impl->string.length());

    // Convert
    Utf32::toUtf16(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output));

    return output;
}


////////////////////////////////////////////////////////////
template <priv::U32StringLike TString>
TString String::toUtf32() const
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
char32_t String::operator[](base::SizeT index) const
{
    SFML_BASE_ASSERT(index < m_impl->string.size() && "Index is out of bounds");
    return m_impl->string[index];
}


////////////////////////////////////////////////////////////
char32_t& String::operator[](base::SizeT index)
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
base::SizeT String::getSize() const
{
    return m_impl->string.size();
}


////////////////////////////////////////////////////////////
bool String::isEmpty() const
{
    return m_impl->string.empty();
}


////////////////////////////////////////////////////////////
void String::erase(base::SizeT position, base::SizeT count)
{
    m_impl->string.erase(position, count);
}


////////////////////////////////////////////////////////////
void String::insert(base::SizeT position, const String& str)
{
    m_impl->string.insert(position, str.m_impl->string);
}


////////////////////////////////////////////////////////////
base::SizeT String::find(const String& str, base::SizeT start) const
{
    return m_impl->string.find(str.m_impl->string, start);
}


////////////////////////////////////////////////////////////
void String::replace(base::SizeT position, base::SizeT length, const String& replaceWith)
{
    m_impl->string.replace(position, length, replaceWith.m_impl->string);
}


////////////////////////////////////////////////////////////
void String::replace(const String& searchFor, const String& replaceWith)
{
    const base::SizeT step = replaceWith.getSize();
    const base::SizeT len  = searchFor.getSize();
    base::SizeT       pos  = find(searchFor);

    // Replace each occurrence of search
    while (pos != InvalidPos)
    {
        replace(pos, len, replaceWith);
        pos = find(searchFor, pos + step);
    }
}


////////////////////////////////////////////////////////////
String String::substring(base::SizeT position, base::SizeT length) const
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
void* String::getImplString()
{
    return &m_impl->string;
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
const base::SizeT String::InvalidPos{std::u32string::npos};


////////////////////////////////////////////////////////////
template String::String(char ansiChar, const std::locale& locale);
template String::String(const char* ansiString, const std::locale& locale);


////////////////////////////////////////////////////////////
template String::String(const std::string& ansiString);
template String::String(const std::string& ansiString, const std::locale& locale);


////////////////////////////////////////////////////////////
template String::String(const std::wstring& wideString);


////////////////////////////////////////////////////////////
template String::String(const std::u32string& utf32String);


////////////////////////////////////////////////////////////
template std::string String::toAnsiString() const;
template std::string String::toAnsiString(const std::locale& locale) const;


////////////////////////////////////////////////////////////
template std::wstring String::toWideString<std::wstring>() const;


////////////////////////////////////////////////////////////
template std::u8string  String::toUtf8<std::u8string>() const;
template std::u16string String::toUtf16<std::u16string>() const;
template std::u32string String::toUtf32<std::u32string>() const;

} // namespace sf
