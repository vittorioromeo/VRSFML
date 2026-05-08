#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/System/UnicodeString.hpp"
#include "SFML/System/Utf.hpp"

#include "SFML/Base/BackInserter.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Builders that turn raw UTF-8/UTF-16/UTF-32 ranges into `sf::UnicodeString`
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API UnicodeStringUtfUtils
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Build a `UnicodeString` from a UTF-8 range `[begin, end)`
    ///
    /// \see fromUtf16, fromUtf32
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static UnicodeString fromUtf8(T begin, T end)
    {
        UnicodeString string;
        Utf8::toUtf32(begin, end, base::BackInserter(string));
        return string;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Build a `UnicodeString` from a UTF-16 range `[begin, end)`
    ///
    /// \see fromUtf8, fromUtf32
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static UnicodeString fromUtf16(T begin, T end)
    {
        UnicodeString string;
        Utf16::toUtf32(begin, end, base::BackInserter(string));
        return string;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Build a `UnicodeString` from a UTF-32 range `[begin, end)`
    ///
    /// Provided for symmetry with the UTF-8/UTF-16 builders; equivalent
    /// to the `const char32_t*` / `std::u32string` constructors.
    ///
    /// \see fromUtf8, fromUtf16
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static UnicodeString fromUtf32(T begin, T end)
    {
        UnicodeString string;
        string.assign(begin, end);
        return string;
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::UnicodeStringUtfUtils
/// \ingroup system
///
/// This class provides static functions for creating `sf::UnicodeString`
/// from UTF-8, UTF-16, or UTF-32 encoded byte sequences.
///
/// It is useful for converting raw UTF data into `sf::UnicodeString`
/// objects, which can then be used with SFML functions.
///
////////////////////////////////////////////////////////////
