#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
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
} // namespace za


namespace za::priv
{
////////////////////////////////////////////////////////////
template <typename T>
concept ShapeDataConcept =                      //
    ZA_IS_SAME(T, ArrowShapeData) ||            //
    ZA_IS_SAME(T, ChevronShapeData) ||          //
    ZA_IS_SAME(T, CircleShapeData) ||           //
    ZA_IS_SAME(T, CogShapeData) ||              //
    ZA_IS_SAME(T, CrossShapeData) ||            //
    ZA_IS_SAME(T, CurvedArrowShapeData) ||      //
    ZA_IS_SAME(T, EllipseShapeData) ||          //
    ZA_IS_SAME(T, HeartShapeData) ||            //
    ZA_IS_SAME(T, PieSliceShapeData) ||         //
    ZA_IS_SAME(T, RectangleShapeData) ||        //
    ZA_IS_SAME(T, RingShapeData) ||             //
    ZA_IS_SAME(T, RingPieSliceShapeData) ||     //
    ZA_IS_SAME(T, RoundedRectangleShapeData) || //
    ZA_IS_SAME(T, StarShapeData) ||             //
    ZA_IS_SAME(T, TrapezoidShapeData);

} // namespace za::priv
