#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class String;
}


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Static functions for creating `sf::String` from UTF data.
///
/// This class complements the standard `sf::String` constructors
/// by providing explicit functions for creating strings
/// from raw UTF-8, UTF-16, or UTF-32 encoded byte sequences.
///
////////////////////////////////////////////////////////////
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
/// \class sf::StringUtfUtils
/// \ingroup system
///
/// This class provides static functions for creating `sf::String`
/// from UTF-8, UTF-16, or UTF-32 encoded byte sequences.
///
/// It is useful for converting raw UTF data into `sf::String`
/// objects, which can then be used with SFML functions.
///
////////////////////////////////////////////////////////////
