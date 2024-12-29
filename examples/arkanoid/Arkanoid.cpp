#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include <utility>
#include <vector>

#include <cmath>
#include <cstddef>

constexpr sf::Vector2f resolution{800.f, 600.f};

class Game
{
private:
    const float        m_ballSpeed{3.f};
    const float        m_playerSpeed{6.f};
    const sf::Vector2f m_brickSize{50.f, 24.f};

    sf::CircleShape m_ball;
    sf::Vector2f    m_ballVelocity;

    sf::RectangleShape m_player;
    sf::Vector2f       m_playerVelocity;

    std::vector<sf::RectangleShape> m_bricks;

    void createBrickGrid()
    {
        const sf::Vector2f offset{50.f, 50.f};

        const std::size_t nBricksPerRow = 12;
        const std::size_t nRows         = 4;

        const float spacing = 120.f / 14.f;

        sf::Vector2f next{0.f, 0.f};

        for (std::size_t y = 0; y < nRows; ++y)
        {
            for (std::size_t x = 0; x <= nBricksPerRow; ++x)
            {
                m_bricks.emplace_back(
                    sf::RectangleShape::Settings{.position         = offset + next,
                                                 .origin           = m_brickSize / 2.f,
                                                 .fillColor        = sf::Color::DarkGreen,
                                                 .outlineColor     = sf::Color::Green,
                                                 .outlineThickness = 2.f,
                                                 .size             = m_brickSize});

                next.x += spacing + m_brickSize.x;
            }

            next.x = 0;
            next.y += spacing + m_brickSize.y;
        }
    }

    bool testIntersectionBetweenBallAndRectangle(const sf::RectangleShape& rect)
    {
        const auto [ballLeft, ballTop]     = m_ball.getTopLeft();
        const auto [ballRight, ballBottom] = m_ball.getBottomRight();

        const auto [rectLeft, rectTop]     = rect.getTopLeft();
        const auto [rectRight, rectBottom] = rect.getBottomRight();

        const bool ballIsLeftOfRect  = ballRight < rectLeft;
        const bool ballIsRightOfRect = ballLeft > rectRight;
        const bool ballIsAboveRect   = ballBottom < rectTop;
        const bool ballIsBelowRect   = ballTop > rectBottom;

        return !ballIsLeftOfRect && !ballIsRightOfRect && !ballIsAboveRect && !ballIsBelowRect;
    }

    bool performBallBrickCollisionResolution(const sf::RectangleShape& brick)
    {
        if (!testIntersectionBetweenBallAndRectangle(brick))
            return false;

        const auto [ballLeft, ballTop]     = m_ball.getTopLeft();
        const auto [ballRight, ballBottom] = m_ball.getBottomRight();

        const auto [brickLeft, brickTop]     = brick.getTopLeft();
        const auto [brickRight, brickBottom] = brick.getBottomRight();

        const float overlapLeft{ballRight - brickLeft};
        const float overlapRight{brickRight - ballLeft};
        const float overlapTop{ballBottom - brickTop};
        const float overlapBottom{brickBottom - ballTop};

        const bool ballFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));
        const bool ballFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

        const float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
        const float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

        if (std::abs(minOverlapX) < std::abs(minOverlapY))
            m_ballVelocity.x = ballFromLeft ? -m_ballSpeed : m_ballSpeed;
        else
            m_ballVelocity.y = ballFromTop ? -m_ballSpeed : m_ballSpeed;

        return true;
    }

    void updateBallMovement()
    {
        m_ball.position = m_ball.position + m_ballVelocity;
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
            m_ball.setCenterLeft({boundaryLeft, m_ball.position.y});
        }
        else if (ballRight > boundaryRight)
        {
            m_ballVelocity.x *= -1.f;
            m_ball.setCenterRight({boundaryRight, m_ball.position.y});
        }

        if (ballTop < boundaryTop)
        {
            m_ballVelocity.y *= -1.f;
            m_ball.setTopCenter({m_ball.position.x, boundaryTop});
        }
        else if (ballBottom > boundaryBottom)
        {
            m_ballVelocity.y *= -1.f;
            m_ball.setBottomCenter({m_ball.position.x, boundaryBottom});
        }
    }

    void updateBallCollisionsAgainstPlayer()
    {
        if (!testIntersectionBetweenBallAndRectangle(m_player))
            return;

        m_ballVelocity.y = -m_ballSpeed;

        if (m_ball.position.x < m_player.position.x)
            m_ballVelocity.x = -m_ballSpeed;
        else
            m_ballVelocity.x = m_ballSpeed;
    }

    void updateBallCollisionsAgainstBricks()
    {
        for (auto it = m_bricks.begin(); it != m_bricks.end(); ++it)
        {
            if (performBallBrickCollisionResolution(*it))
            {
                std::swap(*it, m_bricks.back());
                m_bricks.pop_back();
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
    m_ballVelocity{m_ballSpeed, m_ballSpeed},
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
        updateBallMovement();
        updateBallCollisionsAgainstBoundaries();
        updateBallCollisionsAgainstPlayer();
        updateBallCollisionsAgainstBricks();

        //
        // Player input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            m_playerVelocity.x = -m_playerSpeed;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            m_playerVelocity.x = m_playerSpeed;
        else
            m_playerVelocity.x = 0;

        //
        // Player movement
        m_player.position = m_player.position + m_playerVelocity;

        //
        // Player collision versus boundaries
        const float playerHalfWidth = m_player.getSize().x / 2.f;
        const float playerLeft      = m_player.position.x - playerHalfWidth;
        const float playerRight     = m_player.position.x + playerHalfWidth;

        const float boundaryLeft  = 0.f;
        const float boundaryRight = resolution.x;

        if (playerLeft < boundaryLeft)
            m_player.position.x = boundaryLeft + playerHalfWidth;
        else if (playerRight > boundaryRight)
            m_player.position.x = boundaryRight - playerHalfWidth;
    }

    void drawOnto(sf::RenderTarget& renderTarget)
    {
        renderTarget.draw(m_ball, /* texture */ nullptr);
        renderTarget.draw(m_player, /* texture */ nullptr);

        for (const sf::RectangleShape& brick : m_bricks)
            renderTarget.draw(brick, /* texture */ nullptr);
    }
};

int main()
{
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    sf::RenderWindow window(
        {.size            = resolution.toVector2u(),
         .title           = "Arkanoid",
         .resizable       = false,
         .vsync           = true,
         .frametimeLimit  = 144u,
         .contextSettings = {.antiAliasingLevel = 8u}});

    // ------------------------------------------------------------------------

    Game game;

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;
        }

        game.update();

        window.clear();
        game.drawOnto(window);
        window.display();
    }

    // ------------------------------------------------------------------------

    return 0;
}
