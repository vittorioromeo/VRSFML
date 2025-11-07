////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../bubble_idle/RNGFast.hpp"          // TODO P1: avoid the relative path...?
#include "../bubble_idle/TextEffectWiggle.hpp" // TODO P1: avoid the relative path...?

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Touch.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/UnicodeString.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/String.hpp"

#include "ExampleUtils.hpp"

#ifdef SFML_SYSTEM_IOS
    #include "SFML/Main.hpp"
#endif

namespace
{
sf::Path resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources";
#endif
}
} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    RNGFast rng(/* seed */ 1234);

    // Define some constants
    constexpr sf::Vec2f gameSize{800.f, 600.f};
    constexpr sf::Vec2f paddleSize{25.f, 100.f};
    constexpr float     ballRadius = 10.f;

    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    // Create the window of the application
    auto window = makeDPIScaledRenderWindow({
        .size         = gameSize.toVec2u(),
        .bitsPerPixel = 32u,
        .title        = "SFML Tennis",
        .resizable    = true,
        .vsync        = true,
    });

    // Create an audio context and get the default playback device
    auto               audioContext = sf::AudioContext::create().value();
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    // Load the sounds used in the game
    const auto ballSoundBuffer = sf::SoundBuffer::loadFromFile(resourcesDir() / "ball.wav").value();
    sf::Sound  ballSound(playbackDevice, ballSoundBuffer);

    // Create the SFML logo texture:
    const auto sfmlLogoTexture = sf::Texture::loadFromFile(resourcesDir() / "sfml_logo.png").value();

    // Create the left paddle
    sf::RectangleShape leftPaddle{
        {.origin           = paddleSize / 2.f,
         .fillColor        = {100u, 100u, 200u},
         .outlineColor     = sf::Color::Black,
         .outlineThickness = 3.f,
         .size             = paddleSize - sf::Vec2f{3.f, 3.f}}};

    // Create the right paddle
    sf::RectangleShape rightPaddle = leftPaddle;
    rightPaddle.setFillColor({200u, 100u, 100u});

    // Create the ball
    sf::CircleShape ball{{.origin           = {ballRadius / 2.f, ballRadius / 2.f},
                          .fillColor        = sf::Color::White,
                          .outlineColor     = sf::Color::Black,
                          .outlineThickness = 2.f,
                          .radius           = ballRadius - 3.f}};

    // Open the text font
    const auto font = sf::Font::openFromFile(resourcesDir() / "tuffy.ttf").value();

    // Initialize the pause message
    TextEffectWiggle wiggleTextEffect(10.f, 1.75f);
    sf::Text         pauseMessage(font,
                                  {
                                      .position = {170.f, 200.f},
#ifdef SFML_SYSTEM_IOS
                              .string = "Welcome to SFML Tennis!\nTouch the screen to start the game.",
#else
                              .string = "Welcome to SFML Tennis!\n\nPress space to start the game.",
#endif
                              .characterSize = 40u,

                              .fillColor        = sf::Color::White,
                              .outlineColor     = sf::Color::Black,
                              .outlineThickness = 2.f,
                          });

    // Define game constants
    constexpr float paddleSpeed = 400.f;
    constexpr float ballSpeed   = 400.f;

    // Define the paddles properties
    sf::Clock      aiTimer;
    const sf::Time aiTime           = sf::seconds(0.1f);
    float          rightPaddleSpeed = 0.f;
    sf::Angle      ballAngle        = sf::degrees(0); // to be changed later

    sf::Clock clock;
    bool      isPlaying = false;

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, gameSize, window))
                continue;

            // Space key pressed: play
            if ((event->is<sf::Event::KeyPressed>() &&
                 event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Space) ||
                event->is<sf::Event::TouchBegan>())
            {
                if (!isPlaying)
                {
                    // (re)start the game
                    isPlaying = true;
                    clock.restart();

                    // Reset the position of the paddles and ball
                    leftPaddle.position  = {10.f + paddleSize.x / 2.f, gameSize.y / 2.f};
                    rightPaddle.position = {gameSize.x - 10.f - paddleSize.x / 2.f, gameSize.y / 2.f};
                    ball.position        = gameSize / 2.f;

                    // Reset the ball angle
                    do
                    {
                        // Make sure the ball initial angle is not too much vertical
                        ballAngle = sf::degrees(rng.getF(0.f, 360.f));
                    } while (sf::base::fabs(sf::base::cos(ballAngle.asRadians())) < 0.7f);
                }
            }
        }

        const float deltaTime = clock.restart().asSeconds();

        if (isPlaying)
        {
            // Move the player's paddle
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && (leftPaddle.position.y - paddleSize.y / 2 > 5.f))
            {
                leftPaddle.position.y += -paddleSpeed * deltaTime;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) &&
                     (leftPaddle.position.y + paddleSize.y / 2 < gameSize.y - 5.f))
            {
                leftPaddle.position.y += paddleSpeed * deltaTime;
            }

            if (sf::Touch::isDown(0))
            {
                const sf::Vec2i pos       = sf::Touch::getPosition(0);
                const sf::Vec2f mappedPos = window.mapPixelToCoords(pos);
                leftPaddle.position.y     = mappedPos.y;
            }

            // Move the computer's paddle
            if (((rightPaddleSpeed < 0.f) && (rightPaddle.position.y - paddleSize.y / 2 > 5.f)) ||
                ((rightPaddleSpeed > 0.f) && (rightPaddle.position.y + paddleSize.y / 2 < gameSize.y - 5.f)))
            {
                rightPaddle.position.y += rightPaddleSpeed * deltaTime;
            }

            // Update the computer's paddle direction according to the ball position
            if (aiTimer.getElapsedTime() > aiTime)
            {
                aiTimer.restart();
                if (ball.position.y + ballRadius > rightPaddle.position.y + paddleSize.y / 2)
                    rightPaddleSpeed = paddleSpeed;
                else if (ball.position.y - ballRadius < rightPaddle.position.y - paddleSize.y / 2)
                    rightPaddleSpeed = -paddleSpeed;
                else
                    rightPaddleSpeed = 0.f;
            }

            // Move the ball
            ball.position += sf::Vec2f::fromAngle(ballSpeed * deltaTime, ballAngle);

#ifdef SFML_SYSTEM_IOS
            const sf::base::String inputString = "Touch the screen to restart.";
#else
            const sf::base::String inputString = "Press space to restart or\nescape to exit.";
#endif

            // Check collisions between the ball and the screen
            if (ball.position.x - ballRadius < 0.f)
            {
                isPlaying = false;
                pauseMessage.setString("You Lost!\n\n" + inputString);
            }
            else if (ball.position.x + ballRadius > gameSize.x)
            {
                isPlaying = false;
                pauseMessage.setString("You Won!\n\n" + inputString);
            }

            if (ball.position.y - ballRadius < 0.f)
            {
                ballSound.play();
                ballAngle       = -ballAngle;
                ball.position.y = ballRadius + 0.1f;
            }
            else if (ball.position.y + ballRadius > gameSize.y)
            {
                ballSound.play();
                ballAngle       = -ballAngle;
                ball.position.y = gameSize.y - ballRadius - 0.1f;
            }

            // Check the collisions between the ball and the paddles
            // Left Paddle
            if (ball.position.x - ballRadius < leftPaddle.position.x + paddleSize.x / 2 &&
                ball.position.x - ballRadius > leftPaddle.position.x &&
                ball.position.y + ballRadius >= leftPaddle.position.y - paddleSize.y / 2 &&
                ball.position.y - ballRadius <= leftPaddle.position.y + paddleSize.y / 2)
            {
                if (ball.position.y > leftPaddle.position.y)
                    ballAngle = sf::degrees(180) - ballAngle + sf::degrees(rng.getF(0.f, 20.f));
                else
                    ballAngle = sf::degrees(180) - ballAngle - sf::degrees(rng.getF(0.f, 20.f));

                ballSound.play();
                ball.position.x = leftPaddle.position.x + ballRadius + paddleSize.x / 2 + 0.1f;
            }

            // Right Paddle
            if (ball.position.x + ballRadius > rightPaddle.position.x - paddleSize.x / 2 &&
                ball.position.x + ballRadius < rightPaddle.position.x &&
                ball.position.y + ballRadius >= rightPaddle.position.y - paddleSize.y / 2 &&
                ball.position.y - ballRadius <= rightPaddle.position.y + paddleSize.y / 2)
            {
                if (ball.position.y > rightPaddle.position.y)
                    ballAngle = sf::degrees(180) - ballAngle + sf::degrees(rng.getF(0.f, 20.f));
                else
                    ballAngle = sf::degrees(180) - ballAngle - sf::degrees(rng.getF(0.f, 20.f));

                ballSound.play();
                ball.position.x = rightPaddle.position.x - ballRadius - paddleSize.x / 2 - 0.1f;
            }
        }

        // Clear the window
        window.clear({50u, 50u, 50u});

        if (isPlaying)
        {
            // Draw the paddles and the ball
            window.draw(leftPaddle);
            window.draw(rightPaddle);
            window.draw(ball);
        }
        else
        {
            wiggleTextEffect.advance(deltaTime);
            wiggleTextEffect.apply(pauseMessage);

            // Draw the pause message
            window.draw(pauseMessage);

            wiggleTextEffect.unapply(pauseMessage);

            window.draw(sfmlLogoTexture, {.position = {170.f, 50.f}});
        }

        // Display things on screen
        window.display();
    }

    return 0;
}
