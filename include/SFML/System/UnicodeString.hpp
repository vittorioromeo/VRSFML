#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
concept FacetLike = requires { T::id; };

////////////////////////////////////////////////////////////
template <typename T>
concept AnsiStringLike = SFML_BASE_IS_SAME(typename T::value_type, char);

////////////////////////////////////////////////////////////
template <typename T>
concept WStringLike = SFML_BASE_IS_SAME(typename T::value_type, wchar_t);

////////////////////////////////////////////////////////////
template <typename T>
concept U8StringLike = SFML_BASE_IS_SAME(typename T::value_type, char8_t) || SFML_BASE_IS_SAME(typename T::value_type, char);

////////////////////////////////////////////////////////////
template <typename T>
concept U16StringLike = SFML_BASE_IS_SAME(typename T::value_type, char16_t);

////////////////////////////////////////////////////////////
template <typename T>
concept U32StringLike = SFML_BASE_IS_SAME(typename T::value_type, char32_t);

} // namespace sf::priv


namespace sf
{
class UnicodeStringUtfUtils;

////////////////////////////////////////////////////////////
/// \brief Utility string class that automatically handles conversions between types and encodings
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_SYSTEM_API UnicodeString
{
    friend UnicodeStringUtfUtils;

public:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    using Iterator      = char32_t*;       //!< Iterator type
    using ConstIterator = const char32_t*; //!< Read-only iterator type
    using value_type    = char32_t;        //!< Character type

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    // NOLINTBEGIN(readability-identifier-naming)
    /// Represents an invalid position in the string
    static const base::SizeT InvalidPos;
    // NOLINTEND(readability-identifier-naming)

    ////////////////////////////////////////////////////////////
    /// \brief Construct an empty string
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~UnicodeString();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    UnicodeString(const UnicodeString&);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    UnicodeString& operator=(const UnicodeString&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    UnicodeString(UnicodeString&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    UnicodeString& operator=(UnicodeString&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Disallow construction from `nullptr` literal
    ///
    ////////////////////////////////////////////////////////////
    UnicodeString(decltype(nullptr))                              = delete;
    UnicodeString(decltype(nullptr), const priv::FacetLike auto&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a single ANSI character, converting to UTF-32 (using `facet` if provided)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(char ansiChar);
    [[nodiscard]] UnicodeString(char ansiChar, const priv::FacetLike auto& facet);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a single wide character
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(wchar_t wideChar);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a single UTF-32 character
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(char32_t utf32Char);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a null-terminated C-style ANSI string, converting to UTF-32 (using `facet` if provided)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(const char* ansiString);
    [[nodiscard]] UnicodeString(const char* ansiString, const priv::FacetLike auto& facet);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an ANSI string, converting to UTF-32 (using `facet` if provided)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(const priv::AnsiStringLike auto& ansiString);
    [[nodiscard]] UnicodeString(const priv::AnsiStringLike auto& ansiString, const priv::FacetLike auto& facet);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a null-terminated C-style wide string
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(const wchar_t* wideString);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a wide string
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(const priv::WStringLike auto& wideString);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a null-terminated C-style UTF-32 string
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(const char32_t* utf32String);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a UTF-32 string
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString(const priv::U32StringLike auto& utf32String);

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to an ANSI string in the encoding defined by `facet`
    ///
    /// Characters that do not fit in the target encoding are discarded from the returned string.
    ///
    /// \see `toWideString`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::AnsiStringLike TString>
    [[nodiscard]] TString toAnsiString() const;

    template <priv::AnsiStringLike TString>
    [[nodiscard]] TString toAnsiString(const priv::FacetLike auto& facet) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a wide string
    ///
    /// Characters that do not fit in the target encoding are discarded from the returned string.
    ///
    /// \see `toAnsiString`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::WStringLike TString>
    [[nodiscard]] TString toWideString() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-8 string
    ///
    /// \see `toUtf16`, `toUtf32`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::U8StringLike TString>
    [[nodiscard]] TString toUtf8() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-16 string
    ///
    /// \see `toUtf8`, `toUtf32`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::U16StringLike TString>
    [[nodiscard]] TString toUtf16() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-32 string
    ///
    /// This is a no-op copy, since the string is already stored as UTF-32 internally.
    ///
    /// \see `toUtf8`, `toUtf16`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::U32StringLike TString>
    [[nodiscard]] TString toUtf32() const;

    ////////////////////////////////////////////////////////////
    /// \brief Append `rhs` to this string
    ///
    ////////////////////////////////////////////////////////////
    UnicodeString& operator+=(const UnicodeString& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Read-only access to the character at `index`
    ///
    /// \pre `index` is in range
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] char32_t operator[](base::SizeT index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Read-write access to the character at `index`
    ///
    /// \pre `index` is in range
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] char32_t& operator[](base::SizeT index);

    ////////////////////////////////////////////////////////////
    /// \brief Append a single UTF-32 character to the end of the string
    ///
    ////////////////////////////////////////////////////////////
    void pushBack(char32_t character);

    ////////////////////////////////////////////////////////////
    /// \brief Replace the contents with the characters in the half-open range `[begin, end)`
    ///
    ////////////////////////////////////////////////////////////
    void assign(const char32_t* begin, const char32_t* end);

    ////////////////////////////////////////////////////////////
    /// \brief Remove all characters from the string
    ///
    /// \see `isEmpty`, `erase`
    ///
    ////////////////////////////////////////////////////////////
    void clear();

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of characters in the string
    ///
    /// \see `isEmpty`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the string is empty
    ///
    /// \see `clear`, `getSize`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isEmpty() const;

    ////////////////////////////////////////////////////////////
    /// \brief Erase `count` characters starting from `position`
    ///
    ////////////////////////////////////////////////////////////
    void erase(base::SizeT position, base::SizeT count = 1);

    ////////////////////////////////////////////////////////////
    /// \brief Insert the characters of `str` starting from `position`
    ///
    ////////////////////////////////////////////////////////////
    void insert(base::SizeT position, const UnicodeString& str);

    ////////////////////////////////////////////////////////////
    /// \brief Find a sequence of characters in the string, starting from `start`
    ///
    /// \return Position of `str` in the string, or `UnicodeString::InvalidPos` if not found
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT find(const UnicodeString& str, base::SizeT start = 0) const;

    ////////////////////////////////////////////////////////////
    /// \brief Replace the substring at `position` spanning `length` characters with `replaceWith`
    ///
    /// \param length Number of characters to replace, or `InvalidPos` to replace until the end of the string
    ///
    ////////////////////////////////////////////////////////////
    void replaceRange(base::SizeT position, base::SizeT length, const UnicodeString& replaceWith);

    ////////////////////////////////////////////////////////////
    /// \brief Replace all occurrences of `searchFor` in this string with `replaceWith`
    ///
    ////////////////////////////////////////////////////////////
    void replaceAllOccurrences(const UnicodeString& searchFor, const UnicodeString& replaceWith);

    ////////////////////////////////////////////////////////////
    /// \brief Return the substring starting at `position` and spanning `length` characters
    ///
    /// \param length Number of characters to include, clamped to the string length;
    ///               pass `InvalidPos` to include all characters until the end of the string
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] UnicodeString substring(base::SizeT position, base::SizeT length = InvalidPos) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a read-only pointer to the null-terminated C-style array of characters
    ///
    /// The returned pointer is temporary and meant only for immediate use; do not store it.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char32_t* getData() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return a read-write iterator to the beginning of the string
    ///
    /// \see `end`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Iterator begin();

    ////////////////////////////////////////////////////////////
    /// \brief Return a read-only iterator to the beginning of the string
    ///
    /// \see `end`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ConstIterator begin() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return a read-write iterator to one past the last character
    ///
    /// The end iterator must never be dereferenced.
    ///
    /// \see `begin`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Iterator end();

    ////////////////////////////////////////////////////////////
    /// \brief Return a read-only iterator to one past the last character
    ///
    /// The end iterator must never be dereferenced.
    ///
    /// \see `begin`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ConstIterator end() const;

private:
    friend SFML_SYSTEM_API bool operator==(const UnicodeString& lhs, const UnicodeString& rhs);
    friend SFML_SYSTEM_API bool operator<(const UnicodeString& lhs, const UnicodeString& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve the string stored in the PImpl
    ///
    ////////////////////////////////////////////////////////////
    void* getImplString();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 64> m_impl; //!< Implementation details
};

////////////////////////////////////////////////////////////
/// \brief Equality comparison between two strings
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator==(const UnicodeString& lhs, const UnicodeString& rhs);

////////////////////////////////////////////////////////////
/// \brief Inequality comparison between two strings
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator!=(const UnicodeString& lhs, const UnicodeString& rhs);

////////////////////////////////////////////////////////////
/// \brief Lexicographically less-than comparison between two strings
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator<(const UnicodeString& lhs, const UnicodeString& rhs);

////////////////////////////////////////////////////////////
/// \brief Lexicographically greater-than comparison between two strings
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator>(const UnicodeString& lhs, const UnicodeString& rhs);

////////////////////////////////////////////////////////////
/// \brief Lexicographically less-than-or-equal comparison between two strings
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator<=(const UnicodeString& lhs, const UnicodeString& rhs);

////////////////////////////////////////////////////////////
/// \brief Lexicographically greater-than-or-equal comparison between two strings
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator>=(const UnicodeString& lhs, const UnicodeString& rhs);

////////////////////////////////////////////////////////////
/// \brief Concatenation of two strings
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API UnicodeString operator+(const UnicodeString& lhs, const UnicodeString& rhs);

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::UnicodeString
/// \ingroup system
///
/// `sf::UnicodeString` is a UTF-32 string class that can store any
/// character (European, Chinese, Arabic, Hebrew, etc.) and transparently
/// converts to/from ANSI and wide strings, so standard string classes
/// remain compatible with APIs taking a `sf::UnicodeString`.
///
/// \code
/// sf::UnicodeString s;
///
/// std::string s1 = s.toAnsiString<std::string>();  // converted to ANSI string
/// std::wstring s2 = s.toWideString<std::wstring>(); // converted to wide string
/// s = "hello";         // automatically converted from ANSI string
/// s = L"hello";        // automatically converted from wide string
/// s += 'a';            // automatically converted from ANSI string
/// s += L'a';           // automatically converted from wide string
/// \endcode
///
/// Conversions involving ANSI strings use the default user facet, but
/// a custom facet can be supplied:
/// \code
/// std::facet facet;
/// sf::UnicodeString s;
/// ...
/// std::string s1 = s.toAnsiString(facet);
/// s = sf::UnicodeString("hello", facet);
/// \endcode
///
/// This class is a convenience wrapper that exposes the most common
/// `std::string` operations (random access, iteration, append, compare,
/// etc.); for heavy string processing prefer a more specialized class.
/// For low-level Unicode handling, see the `sf::Utf` classes.
///
////////////////////////////////////////////////////////////
