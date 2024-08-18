#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"


namespace sf
{
class String;

class SFML_SYSTEM_API StringUtfUtils
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new sf::String from a UTF-8 encoded string
    ///
    /// \param begin Forward iterator to the beginning of the UTF-8 sequence
    /// \param end   Forward iterator to the end of the UTF-8 sequence
    ///
    /// \return A sf::String containing the source string
    ///
    /// \see fromUtf16, fromUtf32
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static String fromUtf8(T begin, T end);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new sf::String from a UTF-16 encoded string
    ///
    /// \param begin Forward iterator to the beginning of the UTF-16 sequence
    /// \param end   Forward iterator to the end of the UTF-16 sequence
    ///
    /// \return A sf::String containing the source string
    ///
    /// \see fromUtf8, fromUtf32
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static String fromUtf16(T begin, T end);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new sf::String from a UTF-32 encoded string
    ///
    /// This function is provided for consistency, it is equivalent to
    /// using the constructors that takes a const char32_t* or
    /// a std::u32string.
    ///
    /// \param begin Forward iterator to the beginning of the UTF-32 sequence
    /// \param end   Forward iterator to the end of the UTF-32 sequence
    ///
    /// \return A sf::String containing the source string
    ///
    /// \see fromUtf8, fromUtf16
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static String fromUtf32(T begin, T end);
};

} // namespace sf

#include "SFML/System/StringUtfUtils.inl"


////////////////////////////////////////////////////////////
/// \class sf::String
/// \ingroup system
///
/// sf::String is a utility string class defined mainly for
/// convenience. It is a Unicode string (implemented using
/// UTF-32), thus it can store any character in the world
/// (European, Chinese, Arabic, Hebrew, etc.).
///
/// It automatically handles conversions from/to ANSI and
/// wide strings, so that you can work with standard string
/// classes and still be compatible with functions taking a
/// sf::String.
///
/// \code
/// sf::String s;
///
/// std::string s1 = s;  // automatically converted to ANSI string
/// std::wstring s2 = s; // automatically converted to wide string
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
/// sf::String defines the most important functions of the
/// standard std::string class: removing, random access, iterating,
/// appending, comparing, etc. However it is a simple class
/// provided for convenience, and you may have to consider using
/// a more optimized class if your program requires complex string
/// handling. The automatic conversion functions will then take
/// care of converting your string to sf::String whenever SFML
/// requires it.
///
/// Please note that SFML also defines a low-level, generic
/// interface for Unicode handling, see the sf::Utf classes.
///
////////////////////////////////////////////////////////////
