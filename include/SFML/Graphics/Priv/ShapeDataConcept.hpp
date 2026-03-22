#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
struct ArrowShapeData;
struct CircleShapeData;
struct CurvedArrowShapeData;
struct EllipseShapeData;
struct PieSliceShapeData;
struct RectangleShapeData;
struct RingShapeData;
struct RingPieSliceShapeData;
struct RoundedRectangleShapeData;
struct StarShapeData;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
concept ShapeDataConcept =                             //
    SFML_BASE_IS_SAME(T, ArrowShapeData) ||            //
    SFML_BASE_IS_SAME(T, CircleShapeData) ||           //
    SFML_BASE_IS_SAME(T, CurvedArrowShapeData) ||      //
    SFML_BASE_IS_SAME(T, EllipseShapeData) ||          //
    SFML_BASE_IS_SAME(T, PieSliceShapeData) ||         //
    SFML_BASE_IS_SAME(T, RectangleShapeData) ||        //
    SFML_BASE_IS_SAME(T, RingShapeData) ||             //
    SFML_BASE_IS_SAME(T, RingPieSliceShapeData) ||     //
    SFML_BASE_IS_SAME(T, RoundedRectangleShapeData) || //
    SFML_BASE_IS_SAME(T, StarShapeData);

} // namespace sf::priv
