// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/UnicodeString.hpp"

#include "SFML/System/Utf.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/BackInserter.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/String.hpp"

#include <locale>
#include <string>

#include <cwchar>


namespace sf
{
////////////////////////////////////////////////////////////
struct UnicodeString::Impl
{
    std::u32string string; //!< Internal string of UTF-32 characters
};


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString()                                    = default;
UnicodeString::~UnicodeString()                                   = default;
UnicodeString::UnicodeString(const UnicodeString&)                = default;
UnicodeString& UnicodeString::operator=(const UnicodeString&)     = default;
UnicodeString::UnicodeString(UnicodeString&&) noexcept            = default;
UnicodeString& UnicodeString::operator=(UnicodeString&&) noexcept = default;


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(char ansiChar) : UnicodeString(ansiChar, std::locale{})
{
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(char ansiChar, const priv::LocaleLike auto& locale)
{
    m_impl->string += Utf32::decodeAnsi(ansiChar, locale);
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(wchar_t wideChar)
{
    m_impl->string += Utf32::decodeWide(wideChar);
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(char32_t utf32Char)
{
    m_impl->string += utf32Char;
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(const char* ansiString) : UnicodeString(ansiString, std::locale{})
{
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(const char* ansiString, const priv::LocaleLike auto& locale)
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
UnicodeString::UnicodeString(const priv::AnsiStringLike auto& ansiString) : UnicodeString(ansiString, std::locale{})
{
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(const priv::AnsiStringLike auto& ansiString, const priv::LocaleLike auto& locale)
{
    m_impl->string.reserve(ansiString.size() + 1);
    Utf32::fromAnsi(ansiString.begin(), ansiString.end(), base::BackInserter(m_impl->string), locale);
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(const wchar_t* wideString)
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
UnicodeString::UnicodeString(const priv::WStringLike auto& wideString)
{
    m_impl->string.reserve(wideString.size() + 1);
    Utf32::fromWide(wideString.begin(), wideString.end(), base::BackInserter(m_impl->string));
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(const char32_t* utf32String)
{
    if (utf32String)
        m_impl->string = utf32String;
}


////////////////////////////////////////////////////////////
UnicodeString::UnicodeString(const priv::U32StringLike auto& utf32String) : m_impl(utf32String)
{
}


////////////////////////////////////////////////////////////
template <priv::AnsiStringLike TString>
TString UnicodeString::toAnsiString() const
{
    return toAnsiString<TString>(std::locale{});
}


////////////////////////////////////////////////////////////
template <priv::AnsiStringLike TString>
TString UnicodeString::toAnsiString(const priv::LocaleLike auto& locale) const
{
    // Prepare the output string
    TString output;
    output.reserve(m_impl->string.size() + 1);

    // Convert
    Utf32::toAnsi(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output), 0, locale);

    return output;
}


////////////////////////////////////////////////////////////
template <priv::WStringLike TString>
TString UnicodeString::toWideString() const
{
    // Prepare the output string
    std::wstring output;
    output.reserve(m_impl->string.size() + 1);

    // Convert
    Utf32::toWide(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output), 0);

    return output;
}


////////////////////////////////////////////////////////////
template <priv::U8StringLike TString>
TString UnicodeString::toUtf8() const
{
    // Prepare the output string
    std::u8string output;
    output.reserve(m_impl->string.size());

    // Convert
    Utf32::toUtf8(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output));

    return output;
}


////////////////////////////////////////////////////////////
template <priv::U16StringLike TString>
TString UnicodeString::toUtf16() const
{
    // Prepare the output string
    std::u16string output;
    output.reserve(m_impl->string.size());

    // Convert
    Utf32::toUtf16(m_impl->string.begin(), m_impl->string.end(), base::BackInserter(output));

    return output;
}


////////////////////////////////////////////////////////////
template <priv::U32StringLike TString>
TString UnicodeString::toUtf32() const
{
    return m_impl->string;
}


////////////////////////////////////////////////////////////
UnicodeString& UnicodeString::operator+=(const UnicodeString& rhs)
{
    m_impl->string += rhs.m_impl->string;
    return *this;
}


////////////////////////////////////////////////////////////
char32_t UnicodeString::operator[](base::SizeT index) const
{
    SFML_BASE_ASSERT(index < m_impl->string.size() && "Index is out of bounds");
    return m_impl->string[index];
}


////////////////////////////////////////////////////////////
char32_t& UnicodeString::operator[](base::SizeT index)
{
    SFML_BASE_ASSERT(index < m_impl->string.size() && "Index is out of bounds");
    return m_impl->string[index];
}


////////////////////////////////////////////////////////////
void UnicodeString::clear()
{
    m_impl->string.clear();
}


////////////////////////////////////////////////////////////
base::SizeT UnicodeString::getSize() const
{
    return m_impl->string.size();
}


////////////////////////////////////////////////////////////
bool UnicodeString::isEmpty() const
{
    return m_impl->string.empty();
}


////////////////////////////////////////////////////////////
void UnicodeString::erase(base::SizeT position, base::SizeT count)
{
    m_impl->string.erase(position, count);
}


////////////////////////////////////////////////////////////
void UnicodeString::insert(base::SizeT position, const UnicodeString& str)
{
    m_impl->string.insert(position, str.m_impl->string);
}


////////////////////////////////////////////////////////////
base::SizeT UnicodeString::find(const UnicodeString& str, base::SizeT start) const
{
    return m_impl->string.find(str.m_impl->string, start);
}


////////////////////////////////////////////////////////////
void UnicodeString::replace(base::SizeT position, base::SizeT length, const UnicodeString& replaceWith)
{
    m_impl->string.replace(position, length, replaceWith.m_impl->string);
}


////////////////////////////////////////////////////////////
void UnicodeString::replace(const UnicodeString& searchFor, const UnicodeString& replaceWith)
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
UnicodeString UnicodeString::substring(base::SizeT position, base::SizeT length) const
{
    return m_impl->string.substr(position, length);
}


////////////////////////////////////////////////////////////
const char32_t* UnicodeString::getData() const
{
    return m_impl->string.c_str();
}


////////////////////////////////////////////////////////////
UnicodeString::Iterator UnicodeString::begin()
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front());
}


////////////////////////////////////////////////////////////
UnicodeString::ConstIterator UnicodeString::begin() const
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front());
}


////////////////////////////////////////////////////////////
UnicodeString::Iterator UnicodeString::end()
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front()) + m_impl->string.size();
}


////////////////////////////////////////////////////////////
UnicodeString::ConstIterator UnicodeString::end() const
{
    return m_impl->string.empty() ? nullptr : &(m_impl->string.front()) + m_impl->string.size();
}


////////////////////////////////////////////////////////////
void* UnicodeString::getImplString()
{
    return &m_impl->string;
}


////////////////////////////////////////////////////////////
bool operator==(const UnicodeString& lhs, const UnicodeString& rhs)
{
    return lhs.m_impl->string == rhs.m_impl->string;
}


////////////////////////////////////////////////////////////
bool operator!=(const UnicodeString& lhs, const UnicodeString& rhs)
{
    return !(lhs == rhs);
}


////////////////////////////////////////////////////////////
bool operator<(const UnicodeString& lhs, const UnicodeString& rhs)
{
    return lhs.m_impl->string < rhs.m_impl->string;
}


////////////////////////////////////////////////////////////
bool operator>(const UnicodeString& lhs, const UnicodeString& rhs)
{
    return rhs < lhs;
}


////////////////////////////////////////////////////////////
bool operator<=(const UnicodeString& lhs, const UnicodeString& rhs)
{
    return !(rhs < lhs);
}


////////////////////////////////////////////////////////////
bool operator>=(const UnicodeString& lhs, const UnicodeString& rhs)
{
    return !(lhs < rhs);
}


////////////////////////////////////////////////////////////
UnicodeString operator+(const UnicodeString& lhs, const UnicodeString& rhs)
{
    UnicodeString string = lhs;
    string += rhs;

    return string;
}


////////////////////////////////////////////////////////////
const base::SizeT UnicodeString::InvalidPos{std::u32string::npos};


////////////////////////////////////////////////////////////
template UnicodeString::UnicodeString(char ansiChar, const std::locale& locale);
template UnicodeString::UnicodeString(const char* ansiString, const std::locale& locale);


////////////////////////////////////////////////////////////
template UnicodeString::UnicodeString(const std::string& ansiString);
template UnicodeString::UnicodeString(const std::string& ansiString, const std::locale& locale);


////////////////////////////////////////////////////////////
template UnicodeString::UnicodeString(const base::String& ansiString);
template UnicodeString::UnicodeString(const base::String& ansiString, const std::locale& locale);


////////////////////////////////////////////////////////////
template UnicodeString::UnicodeString(const std::wstring& wideString);


////////////////////////////////////////////////////////////
template UnicodeString::UnicodeString(const std::u32string& utf32String);


////////////////////////////////////////////////////////////
template std::string UnicodeString::toAnsiString() const;
template std::string UnicodeString::toAnsiString(const std::locale& locale) const;


////////////////////////////////////////////////////////////
template base::String UnicodeString::toAnsiString() const;
template base::String UnicodeString::toAnsiString(const std::locale& locale) const;


////////////////////////////////////////////////////////////
template std::wstring UnicodeString::toWideString<std::wstring>() const;


////////////////////////////////////////////////////////////
template std::u8string  UnicodeString::toUtf8<std::u8string>() const;
template std::u16string UnicodeString::toUtf16<std::u16string>() const;
template std::u32string UnicodeString::toUtf32<std::u32string>() const;

} // namespace sf
