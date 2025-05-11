#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/RectUtils.hpp"

#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"


constexpr sf::Vec2f resolution{800.f, 600.f};

class Game
{
private:
    static constexpr float     ballSpeed{3.f};
    static constexpr float     playerSpeed{6.f};
    static constexpr sf::Vec2f brickSize{50.f, 24.f};

    sf::CircleShape m_ball;
    sf::Vec2f       m_ballVelocity;

    sf::RectangleShape m_player;
    sf::Vec2f          m_playerVelocity;

    sf::base::Vector<sf::RectangleShape> m_bricks;

    void createBrickGrid()
    {
        constexpr sf::Vec2f offset{50.f, 50.f};

        constexpr sf::base::SizeT nBricksPerRow = 13;
        constexpr sf::base::SizeT nRows         = 4;

        constexpr float spacing = 120.f / 14.f;

        sf::Vec2f next{0.f, 0.f};

        for (sf::base::SizeT y = 0; y < nRows; ++y)
        {
            for (sf::base::SizeT x = 0; x < nBricksPerRow; ++x)
            {
                m_bricks.emplaceBack(
                    sf::RectangleShapeData{.position         = offset + next,
                                           .origin           = brickSize / 2.f,
                                           .fillColor        = sf::Color::DarkGreen,
                                           .outlineColor     = sf::Color::Green,
                                           .outlineThickness = 2.f,
                                           .size             = brickSize});

                next.x += spacing + brickSize.x;
            }

            next.x = 0;
            next.y += spacing + brickSize.y;
        }
    }

    bool performBallBrickCollisionResolution(const sf::RectangleShape& brick)
    {
        const auto ballBounds  = m_ball.getGlobalBounds();
        const auto brickBounds = brick.getGlobalBounds();

        if (!sf::findIntersection(brickBounds, ballBounds).hasValue())
            return false;

        const float overlapLeft{ballBounds.getRight() - brickBounds.getLeft()};
        const float overlapRight{brickBounds.getRight() - ballBounds.getLeft()};
        const float overlapTop{ballBounds.getBottom() - brickBounds.getTop()};
        const float overlapBottom{brickBounds.getBottom() - ballBounds.getTop()};

        const bool ballFromLeft(sf::base::fabs(overlapLeft) < sf::base::fabs(overlapRight));
        const bool ballFromTop(sf::base::fabs(overlapTop) < sf::base::fabs(overlapBottom));

        const float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
        const float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

        if (sf::base::fabs(minOverlapX) < sf::base::fabs(minOverlapY))
            m_ballVelocity.x = ballFromLeft ? -ballSpeed : ballSpeed;
        else
            m_ballVelocity.y = ballFromTop ? -ballSpeed : ballSpeed;

        return true;
    }

    void updateBallCollisionsAgainstBoundaries()
    {
        const auto [ballLeft, ballTop]     = m_ball.getTopLeft();
        const auto [ballRight, ballBottom] = m_ball.getBottomRight();

        const float boundaryLeft   = 0.f;
        const float boundaryRight  = resolution.x;
        const float boundaryTop    = 0.f;
        const float boundaryBottom = resolution.y;

        if (ballLeft < boundaryLeft)
        {
            m_ballVelocity.x *= -1.f;
            m_ball.setLeft(boundaryLeft);
        }
        else if (ballRight > boundaryRight)
        {
            m_ballVelocity.x *= -1.f;
            m_ball.setRight(boundaryRight);
        }

        if (ballTop < boundaryTop)
        {
            m_ballVelocity.y *= -1.f;
            m_ball.setTop(boundaryTop);
        }
        else if (ballBottom > boundaryBottom)
        {
            m_ballVelocity.y *= -1.f;
            m_ball.setBottom(boundaryBottom);
        }
    }

    void updateBallCollisionsAgainstPlayer()
    {
        if (!sf::findIntersection(m_player.getGlobalBounds(), m_ball.getGlobalBounds()).hasValue())
            return;

        m_ballVelocity.y = -ballSpeed;

        if (m_ball.position.x < m_player.position.x)
            m_ballVelocity.x = -ballSpeed;
        else
            m_ballVelocity.x = ballSpeed;
    }

    void updateBallCollisionsAgainstBricks()
    {
        for (auto* it = m_bricks.begin(); it != m_bricks.end(); ++it)
        {
            if (performBallBrickCollisionResolution(*it))
            {
                sf::base::swap(*it, m_bricks.back());
                m_bricks.popBack();
                break;
            }
        }
    }

public:
    Game() :
    m_ball{{.position         = resolution / 2.f,
            .origin           = {6.f, 6.f},
            .fillColor        = sf::Color::DarkGreen,
            .outlineColor     = sf::Color::Green,
            .outlineThickness = 2.f,
            .radius           = 12.f}},
    m_ballVelocity{ballSpeed, ballSpeed},
    m_player{{.position         = {resolution.x / 2.f, resolution.y - 24.f * 2},
              .origin           = {64.f, 12.f},
              .fillColor        = sf::Color::DarkGreen,
              .outlineColor     = sf::Color::Green,
              .outlineThickness = 2.f,
              .size             = {128.f, 24.f}}},
    m_playerVelocity{0.f, 0.f}
    {
        createBrickGrid();
    }

    void update()
    {
        //
        // Ball movement
        m_ball.position += m_ballVelocity;

        //
        // Ball collisions
        updateBallCollisionsAgainstBoundaries();
        updateBallCollisionsAgainstPlayer();
        updateBallCollisionsAgainstBricks();

        //
        // Player input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            m_playerVelocity.x = -playerSpeed;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            m_playerVelocity.x = playerSpeed;
        else
            m_playerVelocity.x = 0;

        //
        // Player movement
        m_player.position += m_playerVelocity;

        //
        // Player collision versus boundaries
        const float boundaryLeft  = 0.f;
        const float boundaryRight = resolution.x;

        if (m_player.getLeft() < boundaryLeft)
            m_player.setLeft(boundaryLeft);
        else if (m_player.getRight() > boundaryRight)
            m_player.setRight(boundaryRight);
    }

    void drawOnto(sf::RenderTarget& renderTarget)
    {
        renderTarget.draw(m_ball);
        renderTarget.draw(m_player);

        for (const sf::RectangleShape& brick : m_bricks)
            renderTarget.draw(brick);
    }
};

int main()
{
    //
    //
    // Set up graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    //
    // Set up window
    auto window = makeDPIScaledRenderWindow(
        {.size            = resolution.toVec2u(),
         .title           = "Arkanoid",
         .resizable       = true,
         .vsync           = true,
         .frametimeLimit  = 144u,
         .contextSettings = {.antiAliasingLevel = 8u}});

    //
    //
    // Set up game and simulation loop
    Game game;

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, resolution, window))
                continue;
        }

        game.update();

        window.clear();
        game.drawOnto(window);
        window.display();
    }

    return 0;
}
