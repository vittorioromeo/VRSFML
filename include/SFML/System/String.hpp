#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsSame.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
concept LocaleLike = requires
{
    typename T::category;
};

////////////////////////////////////////////////////////////
template <typename T>
concept AnsiStringLike = sf::base::isSame<typename T::value_type, char>;

////////////////////////////////////////////////////////////
template <typename T>
concept WStringLike = sf::base::isSame<typename T::value_type, wchar_t>;

////////////////////////////////////////////////////////////
template <typename T>
concept U8StringLike = sf::base::isSame<typename T::value_type, char8_t>;

////////////////////////////////////////////////////////////
template <typename T>
concept U16StringLike = sf::base::isSame<typename T::value_type, char16_t>;

////////////////////////////////////////////////////////////
template <typename T>
concept U32StringLike = sf::base::isSame<typename T::value_type, char32_t>;

} // namespace sf::priv


namespace sf
{
class StringUtfUtils;

////////////////////////////////////////////////////////////
/// \brief Utility string class that automatically handles
///        conversions between types and encodings
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_SYSTEM_API String
{
    friend StringUtfUtils;

public:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    using Iterator      = char32_t*;       //!< Iterator type
    using ConstIterator = const char32_t*; //!< Read-only iterator type

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    // NOLINTBEGIN(readability-identifier-naming)
    /// Represents an invalid position in the string
    static const base::SizeT InvalidPos;
    // NOLINTEND(readability-identifier-naming)

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructor creates an empty string.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~String();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    String(const String&);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    String& operator=(const String&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    String(String&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    String& operator=(String&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted `std::nullptr_t` constructor
    ///
    /// Disallow construction from `nullptr` literal
    ///
    ////////////////////////////////////////////////////////////
    String(decltype(nullptr))                               = delete;
    String(decltype(nullptr), const priv::LocaleLike auto&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a single ANSI character and a locale
    ///
    /// The source character is converted to UTF-32 according
    /// to the given locale.
    ///
    /// \param ansiChar ANSI character to convert
    /// \param locale   Locale to use for conversion
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(char ansiChar);
    [[nodiscard]] String(char ansiChar, const priv::LocaleLike auto& locale);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from single wide character
    ///
    /// \param wideChar Wide character to convert
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(wchar_t wideChar);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from single UTF-32 character
    ///
    /// \param utf32Char UTF-32 character to convert
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(char32_t utf32Char);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a null-terminated C-style ANSI string and a locale
    ///
    /// The source string is converted to UTF-32 according
    /// to the given locale.
    ///
    /// \param ansiString ANSI string to convert
    /// \param locale     Locale to use for conversion
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(const char* ansiString);
    [[nodiscard]] String(const char* ansiString, const priv::LocaleLike auto& locale);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an ANSI string and a locale
    ///
    /// The source string is converted to UTF-32 according
    /// to the given locale.
    ///
    /// \param ansiString ANSI string to convert
    /// \param locale     Locale to use for conversion
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(const priv::AnsiStringLike auto& ansiString);
    [[nodiscard]] String(const priv::AnsiStringLike auto& ansiString, const priv::LocaleLike auto& locale);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from null-terminated C-style wide string
    ///
    /// \param wideString Wide string to convert
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(const wchar_t* wideString);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a wide string
    ///
    /// \param wideString Wide string to convert
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(const priv::WStringLike auto& wideString);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a null-terminated C-style UTF-32 string
    ///
    /// \param utf32String UTF-32 string to assign
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(const char32_t* utf32String);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an UTF-32 string
    ///
    /// \param utf32String UTF-32 string to assign
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String(const priv::U32StringLike auto& utf32String);

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to an ANSI string
    ///
    /// The UTF-32 string is converted to an ANSI string in
    /// the encoding defined by \a `locale`.
    /// Characters that do not fit in the target encoding are
    /// discarded from the returned string.
    ///
    /// \param locale Locale to use for conversion
    ///
    /// \return Converted ANSI string
    ///
    /// \see `toWideString`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::AnsiStringLike TString>
    [[nodiscard]] TString toAnsiString() const;

    template <priv::AnsiStringLike TString>
    [[nodiscard]] TString toAnsiString(const priv::LocaleLike auto& locale) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a wide string
    ///
    /// Characters that do not fit in the target encoding are
    /// discarded from the returned string.
    ///
    /// \return Converted wide string
    ///
    /// \see `toAnsiString`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::WStringLike TString>
    [[nodiscard]] TString toWideString() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-8 string
    ///
    /// \return Converted UTF-8 string
    ///
    /// \see `toUtf16`, `toUtf32`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::U8StringLike TString>
    [[nodiscard]] TString toUtf8() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-16 string
    ///
    /// \return Converted UTF-16 string
    ///
    /// \see `toUtf8`, `toUtf32`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::U16StringLike TString>
    [[nodiscard]] TString toUtf16() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-32 string
    ///
    /// This function doesn't perform any conversion, since the
    /// string is already stored as UTF-32 internally.
    ///
    /// \return Converted UTF-32 string
    ///
    /// \see `toUtf8`, `toUtf16`
    ///
    ////////////////////////////////////////////////////////////
    template <priv::U32StringLike TString>
    [[nodiscard]] TString toUtf32() const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of `operator+=` to append an UTF-32 string
    ///
    /// \param right String to append
    ///
    /// \return Reference to self
    ///
    ////////////////////////////////////////////////////////////
    String& operator+=(const String& right);

    ////////////////////////////////////////////////////////////
    /// \brief Overload of `operator[]` to access a character by its position
    ///
    /// This function provides read-only access to characters.
    /// Note: the behavior is undefined if \a `index` is out of range.
    ///
    /// \param index Index of the character to get
    ///
    /// \return Character at position \a `index`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] char32_t operator[](base::SizeT index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of `operator[]` to access a character by its position
    ///
    /// This function provides read and write access to characters.
    /// Note: the behavior is undefined if \a `index` is out of range.
    ///
    /// \param index Index of the character to get
    ///
    /// \return Reference to the character at position \a `index`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] char32_t& operator[](base::SizeT index);

    ////////////////////////////////////////////////////////////
    /// \brief Clear the string
    ///
    /// This function removes all the characters from the string.
    ///
    /// \see `isEmpty`, `erase`
    ///
    ////////////////////////////////////////////////////////////
    void clear();

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the string
    ///
    /// \return Number of characters in the string
    ///
    /// \see `isEmpty`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the string is empty or not
    ///
    /// \return `true` if the string is empty (i.e. contains no character)
    ///
    /// \see `clear`, `getSize`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isEmpty() const;

    ////////////////////////////////////////////////////////////
    /// \brief Erase one or more characters from the string
    ///
    /// This function removes a sequence of \a `count` characters
    /// starting from \a `position`.
    ///
    /// \param position Position of the first character to erase
    /// \param count    Number of characters to erase
    ///
    ////////////////////////////////////////////////////////////
    void erase(base::SizeT position, base::SizeT count = 1);

    ////////////////////////////////////////////////////////////
    /// \brief Insert one or more characters into the string
    ///
    /// This function inserts the characters of \a `str`
    /// into the string, starting from \a `position`.
    ///
    /// \param position Position of insertion
    /// \param str      Characters to insert
    ///
    ////////////////////////////////////////////////////////////
    void insert(base::SizeT position, const String& str);

    ////////////////////////////////////////////////////////////
    /// \brief Find a sequence of one or more characters in the string
    ///
    /// This function searches for the characters of \a `str`
    /// in the string, starting from \a `start`.
    ///
    /// \param str   Characters to find
    /// \param start Where to begin searching
    ///
    /// \return Position of \a `str` in the string, or `String::InvalidPos` if not found
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT find(const String& str, base::SizeT start = 0) const;

    ////////////////////////////////////////////////////////////
    /// \brief Replace a substring with another string
    ///
    /// This function replaces the substring that starts at index \a `position`
    /// and spans \a `length` characters with the string \a `replaceWith`.
    ///
    /// \param position    Index of the first character to be replaced
    /// \param length      Number of characters to replace. You can pass InvalidPos to
    ///                    replace all characters until the end of the string.
    /// \param replaceWith String that replaces the given substring.
    ///
    ////////////////////////////////////////////////////////////
    void replace(base::SizeT position, base::SizeT length, const String& replaceWith);

    ////////////////////////////////////////////////////////////
    /// \brief Replace all occurrences of a substring with a replacement string
    ///
    /// This function replaces all occurrences of \a `searchFor` in this string
    /// with the string \a `replaceWith`.
    ///
    /// \param searchFor   The value being searched for
    /// \param replaceWith The value that replaces found \a `searchFor` values
    ///
    ////////////////////////////////////////////////////////////
    void replace(const String& searchFor, const String& replaceWith);

    ////////////////////////////////////////////////////////////
    /// \brief Return a part of the string
    ///
    /// This function returns the substring that starts at index \a `position`
    /// and spans \a `length` characters.
    ///
    /// \param position Index of the first character
    /// \param length   Number of characters to include in the substring (if
    ///                 the string is shorter, as many characters as possible
    ///                 are included). `InvalidPos` can be used to include all
    ///                 characters until the end of the string.
    ///
    /// \return String object containing a substring of this object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] String substring(base::SizeT position, base::SizeT length = InvalidPos) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a pointer to the C-style array of characters
    ///
    /// This functions provides a read-only access to a
    /// null-terminated C-style representation of the string.
    /// The returned pointer is temporary and is meant only for
    /// immediate use, thus it is not recommended to store it.
    ///
    /// \return Read-only pointer to the array of characters
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char32_t* getData() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the beginning of the string
    ///
    /// \return Read-write iterator to the beginning of the string characters
    ///
    /// \see `end`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Iterator begin();

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the beginning of the string
    ///
    /// \return Read-only iterator to the beginning of the string characters
    ///
    /// \see `end`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ConstIterator begin() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the end of the string
    ///
    /// The end iterator refers to 1 position past the last character;
    /// thus it represents an invalid character and should never be
    /// accessed.
    ///
    /// \return Read-write iterator to the end of the string characters
    ///
    /// \see `begin`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Iterator end();

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the end of the string
    ///
    /// The end iterator refers to 1 position past the last character;
    /// thus it represents an invalid character and should never be
    /// accessed.
    ///
    /// \return Read-only iterator to the end of the string characters
    ///
    /// \see `begin`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ConstIterator end() const;

private:
    friend SFML_SYSTEM_API bool operator==(const String& left, const String& right);
    friend SFML_SYSTEM_API bool operator<(const String& left, const String& right);

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
/// \relates String
/// \brief Overload of `operator==` to compare two UTF-32 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return `true` if both strings are equal
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator==(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of `operator!=` to compare two UTF-32 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return `true` if both strings are different
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator!=(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of `operator<` to compare two UTF-32 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return `true` if \a `left` is lexicographically before \a `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator<(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of `operator>` to compare two UTF-32 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return `true` if \a `left` is lexicographically after \a `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator>(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of `operator<=` to compare two UTF-32 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return `true` if \a `left` is lexicographically before or equivalent to \a `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator<=(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of `operator>=` to compare two UTF-32 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return `true` if \a `left` is lexicographically after or equivalent to \a `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API bool operator>=(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of binary `operator+` to concatenate two strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return Concatenated string
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API String operator+(const String& left, const String& right);

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::String
/// \ingroup system
///
/// `sf::String` is a utility string class defined mainly for
/// convenience. It is a Unicode string (implemented using
/// UTF-32), thus it can store any character in the world
/// (European, Chinese, Arabic, Hebrew, etc.).
///
/// It can handle conversions from/to ANSI and wide strings,
/// so that you can work with standard string classes and still
/// be compatible with functions taking a `sf::String`.
///
/// \code
/// sf::String s;
///
/// std::string s1 = s.toAnsiString<std::string>();  // converted to ANSI string
/// std::wstring s2 = s.toWideString<std::wstring>(); // converted to wide string
/// s = "hello";         // automatically converted from ANSI string
/// s = L"hello";        // automatically converted from wide string
/// s += 'a';            // automatically converted from ANSI string
/// s += L'a';           // automatically converted from wide string
/// \endcode
///
/// Conversions involving ANSI strings use the default user locale. However
/// it is possible to use a custom locale if necessary:
/// \code
/// std::locale locale;
/// sf::String s;
/// ...
/// std::string s1 = s.toAnsiString(locale);
/// s = sf::String("hello", locale);
/// \endcode
///
/// `sf::String` defines the most important functions of the
/// standard `std::string` class: removing, random access, iterating,
/// appending, comparing, etc. However it is a simple class
/// provided for convenience, and you may have to consider using
/// a more optimized class if your program requires complex string
/// handling. The automatic conversion functions will then take
/// care of converting your string to `sf::String` whenever SFML
/// requires it.
///
/// Please note that SFML also defines a low-level, generic
/// interface for Unicode handling, see the `sf::Utf` classes.
///
////////////////////////////////////////////////////////////
