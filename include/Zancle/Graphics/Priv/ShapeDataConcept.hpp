#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Trait/IsSame.hpp"


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
    ZB_IS_SAME(T, ArrowShapeData) ||            //
    ZB_IS_SAME(T, ChevronShapeData) ||          //
    ZB_IS_SAME(T, CircleShapeData) ||           //
    ZB_IS_SAME(T, CogShapeData) ||              //
    ZB_IS_SAME(T, CrossShapeData) ||            //
    ZB_IS_SAME(T, CurvedArrowShapeData) ||      //
    ZB_IS_SAME(T, EllipseShapeData) ||          //
    ZB_IS_SAME(T, HeartShapeData) ||            //
    ZB_IS_SAME(T, PieSliceShapeData) ||         //
    ZB_IS_SAME(T, RectangleShapeData) ||        //
    ZB_IS_SAME(T, RingShapeData) ||             //
    ZB_IS_SAME(T, RingPieSliceShapeData) ||     //
    ZB_IS_SAME(T, RoundedRectangleShapeData) || //
    ZB_IS_SAME(T, StarShapeData) ||             //
    ZB_IS_SAME(T, TrapezoidShapeData);

} // namespace za::priv
