#pragma once

#include "ShowcaseExample.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/StarShapeData.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"


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
    sf::ArrowShapeData            m_sdArrow;
    sf::CircleShapeData           m_sdCircle;
    sf::CurvedArrowShapeData      m_sdCurvedArrow;
    sf::EllipseShapeData          m_sdEllipse;
    sf::PieSliceShapeData         m_sdPieSlice;
    sf::RectangleShapeData        m_sdRectangle;
    sf::RingShapeData             m_sdRingShape;
    sf::RingPieSliceShapeData     m_sdRingPieSlice;
    sf::RoundedRectangleShapeData m_sdRoundedRectangle;
    sf::StarShapeData             m_sdStar;

    ////////////////////////////////////////////////////////////
    int m_shapeIndex = 0;

    ////////////////////////////////////////////////////////////
    sf::Vec2f m_position{256.f, 256.f};
    sf::Vec2f m_origin;
    sf::Angle m_rotation;
    sf::Vec2f m_scale{1.f, 1.f};
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
