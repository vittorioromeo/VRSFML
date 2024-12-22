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

const sf::Vector2f resolution{800.f, 600.f};

class Game
{
private:
    const float        m_ballSpeed{6.f};
    const float        m_playerSpeed{12.f};
    const sf::Vector2f m_brickSize{50.f, 24.f};

    sf::CircleShape m_ball;
    sf::Vector2f    m_ballVelocity;

    sf::RectangleShape m_player;
    sf::Vector2f       m_playerVelocity;

    std::vector<sf::RectangleShape> m_bricks;


    void createBrickGrid()
    {
        const float offsetX = 50.f;
        const float offsetY = 50.f;

        const std::size_t nBricksPerRow = 13;
        const std::size_t nRows         = 4;

        const float spacing = 50.f / 14.f;

        float nextX = 0.f;
        float nextY = 0.f;

        for (std::size_t y = 0; y < nRows; ++y)
        {
            for (std::size_t x = 0; x <= nBricksPerRow; ++x)
            {
                m_bricks.emplace_back(sf::RectangleShape::Settings{.position  = {offsetX + nextX, offsetY + nextY},
                                                                 .origin    = m_brickSize / 2.f,
                                                                 .fillColor = sf::Color::White,
                                                                 .size      = m_brickSize});

                nextX += spacing + m_brickSize.x;
            }

            nextX = 0;
            nextY += spacing + m_brickSize.y;
        }
    }

    bool testIntersectionBetweenBallAndRectangle(const sf::RectangleShape& rect)
    {
        const float ballLeft   = m_ball.position.x - m_ball.getRadius();
        const float ballRight  = m_ball.position.x + m_ball.getRadius();
        const float ballTop    = m_ball.position.y - m_ball.getRadius();
        const float ballBottom = m_ball.position.y + m_ball.getRadius();

        const float rectHalfWidth  = rect.getSize().x / 2.f;
        const float rectHalfHeight = rect.getSize().y / 2.f;
        const float rectLeft       = rect.position.x - rectHalfWidth;
        const float rectRight      = rect.position.x + rectHalfWidth;
        const float rectTop        = rect.position.y - rectHalfHeight;
        const float rectBottom     = rect.position.y + rectHalfHeight;

        const bool ballIsLeftOfRect  = ballRight < rectLeft;
        const bool ballIsRightOfRect = ballLeft > rectRight;
        const bool ballIsAboveRect   = ballBottom < rectTop;
        const bool ballIsBelowRect   = ballTop > rectBottom;

        const bool anyIntersection = !ballIsLeftOfRect &&  //
                                     !ballIsRightOfRect && //
                                     !ballIsAboveRect &&   //
                                     !ballIsBelowRect;
        return anyIntersection;
    }

    // Collisione tra pallina e mattoncino
    bool performBallBrickCollisionResolution(const sf::RectangleShape& brick)
    {
        if (!testIntersectionBetweenBallAndRectangle(brick))
        {
            // Nessuna collisione, usciamo.
            return false;
        }

        const float ballLeft   = m_ball.position.x - m_ball.getRadius();
        const float ballRight  = m_ball.position.x + m_ball.getRadius();
        const float ballTop    = m_ball.position.y - m_ball.getRadius();
        const float ballBottom = m_ball.position.y + m_ball.getRadius();

        const float brickHalfWidth  = brick.getSize().x / 2.f;
        const float brickHalfHeight = brick.getSize().y / 2.f;
        const float brickLeft       = brick.position.x - brickHalfWidth;
        const float brickRight      = brick.position.x + brickHalfWidth;
        const float brickTop        = brick.position.y - brickHalfHeight;
        const float brickBottom     = brick.position.y + brickHalfHeight;

        // Calcoliamo l'intersezione da ogni direzione.
        const float overlapLeft{ballRight - brickLeft};
        const float overlapRight{brickRight - ballLeft};
        const float overlapTop{ballBottom - brickTop};
        const float overlapBottom{brickBottom - ballTop};

        // Se la magnitudine dell'intersezione da sinistra è minore di quella da
        // destra, possiamo concludere che il mattoncino è stato colpito da
        // sinistra.
        const bool ballFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));

        // Stessa idea per sopra/sotto.
        const bool ballFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

        // Conserviamo l'intersezione minima per asse.
        const float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
        const float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

        // Se la magnitudine dell'intersezione sull'asse X è minore di quella
        // sull'asse Y, possiamo concludere che la pallina ha colpito il
        // mattoncino orizzontalmente, altrimenti verticalmente.

        // In base alla nostra conclusione, possiamo decidere se far rimbalzare
        // la pallina orizzontalmente o verticalmente, creando un rimbalzo
        // "realistico".
        if (std::abs(minOverlapX) < std::abs(minOverlapY))
        {
            m_ballVelocity.x = ballFromLeft ? -m_ballSpeed : m_ballSpeed;
        }
        else
        {
            m_ballVelocity.y = ballFromTop ? -m_ballSpeed : m_ballSpeed;
        }

        // Collisione!
        return true;
    }

    void updateBallMovement()
    {
        m_ball.position = m_ball.position + m_ballVelocity;
    }

    void updateBallCollisionsAgainstBoundaries()
    {
        const float ballLeft   = m_ball.position.x - m_ball.getRadius();
        const float ballRight  = m_ball.position.x + m_ball.getRadius();
        const float ballTop    = m_ball.position.y - m_ball.getRadius();
        const float ballBottom = m_ball.position.y + m_ball.getRadius();

        const float boundaryLeft   = 0.f;
        const float boundaryRight  = resolution.x;
        const float boundaryTop    = 0.f;
        const float boundaryBottom = resolution.y;

        if (ballLeft < boundaryLeft)
        {
            m_ballVelocity.x  = -m_ballVelocity.x;
            m_ball.position.x = boundaryLeft + m_ball.getRadius();
        }
        else if (ballRight > boundaryRight)
        {
            m_ballVelocity.x  = -m_ballVelocity.x;
            m_ball.position.x = boundaryRight - m_ball.getRadius();
        }

        if (ballTop < boundaryTop)
        {
            m_ballVelocity.y  = -m_ballVelocity.y;
            m_ball.position.y = boundaryTop + m_ball.getRadius();
        }
        else if (ballBottom > boundaryBottom)
        {
            m_ballVelocity.y  = -m_ballVelocity.y;
            m_ball.position.y = boundaryBottom - m_ball.getRadius();
        }
    }

    void updateBallCollisionsAgainstPlayer()
    {
        if (!testIntersectionBetweenBallAndRectangle(m_player))
        {
            return;
        }

        m_ballVelocity.y = -m_ballSpeed;

        if (m_ball.position.x < m_player.position.x)
        {
            m_ballVelocity.x = -m_ballSpeed;
        }
        else
        {
            m_ballVelocity.x = m_ballSpeed;
        }
    }

    // Gestiamo tutte le collisioni tra pallina e mattoncini.
    void updateBallCollisionsAgainstBricks()
    {
        // Iteriamo su tutti i mattoncini...
        for (auto it = m_bricks.begin(); it != m_bricks.end(); ++it)
        {
            // Se c'è una collisione, swappiamo il mattoncino con quello alla
            // fine del `vector`, e chiamiamo `pop_back`.
            if (performBallBrickCollisionResolution(*it))
            {
                std::swap(*it, m_bricks.back());
                m_bricks.pop_back();
            }
        }
    }

    void updatePlayerInput()
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            m_playerVelocity.x = -m_playerSpeed;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            m_playerVelocity.x = m_playerSpeed;
        }
        else
        {
            m_playerVelocity.x = 0;
        }
    }

    void updatePlayerMovement()
    {
        m_player.position = m_player.position + m_playerVelocity;
    }

    void updatePlayerCollisionsAgainstBoundaries()
    {
        const float playerHalfWidth = m_player.getSize().x / 2.f;
        const float playerLeft      = m_player.position.x - playerHalfWidth;
        const float playerRight     = m_player.position.x + playerHalfWidth;

        const float boundaryLeft  = 0.f;
        const float boundaryRight = resolution.x;

        const auto setPlayerX = [this](const float x)
        {
            sf::Vector2f newPosition = m_player.position;
            newPosition.x            = x;
            m_player.position          = newPosition;
        };

        if (playerLeft < boundaryLeft)
        {
            setPlayerX(boundaryLeft + playerHalfWidth);
        }
        else if (playerRight > boundaryRight)
        {
            setPlayerX(boundaryRight - playerHalfWidth);
        }
    }

public:
    Game() :
    m_ball{{.position = resolution / 2.f, .origin = {6.f, 6.f}, .fillColor = sf::Color::White, .radius = 12.f}},
    m_ballVelocity{m_ballSpeed, m_ballSpeed},
    m_player{{.position  = {resolution.x / 2.f, resolution.y - 24.f * 2},
            .origin    = {64.f, 12.f},
            .fillColor = sf::Color::White,
            .size      = {128.f, 24.f}}},
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

        updatePlayerInput();
        updatePlayerMovement();
        updatePlayerCollisionsAgainstBoundaries();
    }

    void draw(sf::RenderTarget& renderTarget)
    {
        renderTarget.draw(m_ball, /* texture */ nullptr);
        renderTarget.draw(m_player, /* texture */ nullptr);

        for (const sf::RectangleShape& brick : m_bricks)
        {
            renderTarget.draw(brick, /* texture */ nullptr);
        }
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
         .frametimeLimit  = 60u,
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
        game.draw(window);
        window.display();
    }

    // ------------------------------------------------------------------------

    return 0;
}
