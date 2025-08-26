#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"
#include "SFML/Graphics/TextClusterGrouping.hpp"
#include "SFML/Graphics/TextLineAlignment.hpp"
#include "SFML/Graphics/TextOrientation.hpp"
#include "SFML/Graphics/TextStyle.hpp"

#include "SFML/System/String.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \struct sf::TextData
/// \ingroup graphics
///
/// \brief Data required to draw text
///
/// `sf::TextData` is a simple struct that aggregates all the visual
/// and transformational properties of a piece of text. This includes
/// the text string itself, character size, styling (like bold or italic),
/// colors, spacing, and transformation attributes (position, rotation,
/// scale, origin).
///
/// Its POD nature makes it suitable for scenarios like batched rendering,
/// where an array of `TextData` might be processed, or for serializing
/// text properties to and from files.
///
/// \see sf::Text, sf::TextStyle, sf::Color, sf::String
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API TextData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;

    String              string{};                                        //!< String to display
    unsigned int        characterSize{30u};                              //!< Base size of characters, in pixels
    float               letterSpacing{1.f};                              //!< Spacing factor between letters
    float               lineSpacing{1.f};                                //!< Spacing factor between lines
    Color               fillColor{Color::White};                         //!< Text fill color
    Color               outlineColor{Color::Black};                      //!< Text outline color
    float               outlineThickness{0.f};                           //!< Thickness of the text's outline
    TextStyle           style{TextStyle::Regular};                       //!< Text style (see Style enum)
    TextLineAlignment   lineAlignment{TextLineAlignment::Default};       //!< Line alignment for a multi-line text
    TextOrientation     textOrientation{TextOrientation::Default};       //!< Text orientation
    TextClusterGrouping clusterGrouping{TextClusterGrouping::Character}; //!< Cluster grouping algorithm
};

} // namespace sf
