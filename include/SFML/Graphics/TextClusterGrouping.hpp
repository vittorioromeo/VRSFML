#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Enumeration of text cluster grouping options
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextClusterGrouping : unsigned char
{
    Grapheme,  //!< Group clusters by grapheme
    Character, //!< Group clusters by character
    None       //!< Do not group clusters
};

} // namespace sf
