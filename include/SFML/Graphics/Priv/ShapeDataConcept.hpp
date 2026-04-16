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
struct ChevronShapeData;
struct CircleShapeData;
struct CogShapeData;
struct CrossShapeData;
struct CurvedArrowShapeData;
struct EllipseShapeData;
struct HeartShapeData;
struct PieSliceShapeData;
struct RectangleShapeData;
struct RingShapeData;
struct RingPieSliceShapeData;
struct RoundedRectangleShapeData;
struct StarShapeData;
struct TrapezoidShapeData;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
concept ShapeDataConcept =                             //
    SFML_BASE_IS_SAME(T, ArrowShapeData) ||            //
    SFML_BASE_IS_SAME(T, ChevronShapeData) ||          //
    SFML_BASE_IS_SAME(T, CircleShapeData) ||           //
    SFML_BASE_IS_SAME(T, CogShapeData) ||              //
    SFML_BASE_IS_SAME(T, CrossShapeData) ||            //
    SFML_BASE_IS_SAME(T, CurvedArrowShapeData) ||      //
    SFML_BASE_IS_SAME(T, EllipseShapeData) ||          //
    SFML_BASE_IS_SAME(T, HeartShapeData) ||            //
    SFML_BASE_IS_SAME(T, PieSliceShapeData) ||         //
    SFML_BASE_IS_SAME(T, RectangleShapeData) ||        //
    SFML_BASE_IS_SAME(T, RingShapeData) ||             //
    SFML_BASE_IS_SAME(T, RingPieSliceShapeData) ||     //
    SFML_BASE_IS_SAME(T, RoundedRectangleShapeData) || //
    SFML_BASE_IS_SAME(T, StarShapeData) ||             //
    SFML_BASE_IS_SAME(T, TrapezoidShapeData);

} // namespace sf::priv
