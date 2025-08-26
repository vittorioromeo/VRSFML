#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/TextDirection.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Structure describing a glyph after shaping
///
////////////////////////////////////////////////////////////
struct ShapedGlyph
{
    Glyph glyph;

    Vec2f         position;        //!< Position of the glyph within a text
    base::U32     cluster{};       //!< Cluster ID
    TextDirection textDirection{}; //!< Text direction
    float         baseline{};      //!< The baseline position of the line this glyph is a part of
    base::SizeT   vertexOffset{};  //!< Starting offset of the vertex data belonging to this glyph
    base::SizeT   vertexCount{};   //!< Count of vertices belonging to this glyph
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::ShapedGlyph
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Font`
///
////////////////////////////////////////////////////////////
