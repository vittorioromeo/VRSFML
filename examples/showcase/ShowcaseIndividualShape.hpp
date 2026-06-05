#pragma once

#include "ShowcaseExample.hpp"

#include "Zancle/Graphics/ArrowShapeData.hpp"
#include "Zancle/Graphics/ChevronShapeData.hpp"
#include "Zancle/Graphics/CircleShapeData.hpp"
#include "Zancle/Graphics/CogShapeData.hpp"
#include "Zancle/Graphics/CrossShapeData.hpp"
#include "Zancle/Graphics/CurvedArrowShapeData.hpp"
#include "Zancle/Graphics/EllipseShapeData.hpp"
#include "Zancle/Graphics/HeartShapeData.hpp"
#include "Zancle/Graphics/PieSliceShapeData.hpp"
#include "Zancle/Graphics/RectangleShapeData.hpp"
#include "Zancle/Graphics/RingPieSliceShapeData.hpp"
#include "Zancle/Graphics/RingShapeData.hpp"
#include "Zancle/Graphics/RoundedRectangleShapeData.hpp"
#include "Zancle/Graphics/StarShapeData.hpp"
#include "Zancle/Graphics/TrapezoidShapeData.hpp"

#include "Zancle/Geometry/Angle.hpp"
#include "Zancle/Geometry/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
class ExampleIndividualShape : public ShowcaseExample
{
private:
    ////////////////////////////////////////////////////////////
    GameDependencies m_deps;

    ////////////////////////////////////////////////////////////
    float m_time  = 0.f;
    float m_phase = 0.f;

    ///////////////////////////////////////////////////////////
    za::ArrowShapeData            m_sdArrow;
    za::ChevronShapeData          m_sdChevron;
    za::CircleShapeData           m_sdCircle;
    za::CogShapeData              m_sdCog;
    za::CrossShapeData            m_sdCross;
    za::CurvedArrowShapeData      m_sdCurvedArrow;
    za::EllipseShapeData          m_sdEllipse;
    za::HeartShapeData            m_sdHeart;
    za::PieSliceShapeData         m_sdPieSlice;
    za::RectangleShapeData        m_sdRectangle;
    za::RingShapeData             m_sdRingShape;
    za::RingPieSliceShapeData     m_sdRingPieSlice;
    za::RoundedRectangleShapeData m_sdRoundedRectangle;
    za::StarShapeData             m_sdStar;
    za::TrapezoidShapeData        m_sdTrapezoid;

    ////////////////////////////////////////////////////////////
    bool m_showPositionOrigin = false;
    bool m_showBoundingBox    = false;
    bool m_showCentroid       = false;
    int  m_shapeIndex         = 0;

    ////////////////////////////////////////////////////////////
    za::Vec2f m_position{256.f, 256.f};
    za::Vec2f m_origin;
    za::Angle m_rotation;
    za::Vec2f m_scale{1.f, 1.f};
    float     m_outlineThickness = 4.f;
    float     m_miterLimit       = 4.f;

    ////////////////////////////////////////////////////////////
    decltype(auto) callWithActiveShape(auto&& f);

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleIndividualShape(const GameDependencies& deps);

    ////////////////////////////////////////////////////////////
    void update(float deltaTimeMs) override;

    ////////////////////////////////////////////////////////////
    void imgui() override;

    ////////////////////////////////////////////////////////////
    void draw() override;
};
