////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Shader.hpp"

#include "ExampleUtils/RNGFast.hpp"
#include "ExampleUtils/Scaling.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
RNGFast rng(/* seed */ 1234);


////////////////////////////////////////////////////////////
// Base class for effects
////////////////////////////////////////////////////////////
struct Effect
{
    virtual ~Effect() = default;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
    virtual void update(float time, float x, float y)                          = 0;
};


////////////////////////////////////////////////////////////
// "Pixelate" fragment shader
////////////////////////////////////////////////////////////
class Pixelate : public Effect
{
public:
    explicit Pixelate(sf::Texture&& texture, sf::Shader&& shader) :
        m_texture(SFML_BASE_MOVE(texture)),
        m_shader(SFML_BASE_MOVE(shader)),
        m_ulTexture(m_shader.getUniformLocation("sf_u_texture").value()),
        m_ulPixelThreshold(m_shader.getUniformLocation("pixel_threshold").value())
    {
        m_shader.setUniform(m_ulTexture, sf::Shader::CurrentTexture);
    }

    void update(float /* time */, float x, float y) override
    {
        m_shader.setUniform(m_ulPixelThreshold, (x + y) / 30);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.shader = &m_shader;
        target.draw(m_texture, states);
    }

private:
    sf::Texture                 m_texture;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulTexture;
    sf::Shader::UniformLocation m_ulPixelThreshold;
};


////////////////////////////////////////////////////////////
// "Wave" vertex shader + "blur" fragment shader
////////////////////////////////////////////////////////////
class WaveBlur : public Effect
{
public:
    void update(float time, float x, float y) override
    {
        m_shader.setUniform(m_ulWavePhase, time);
        m_shader.setUniform(m_ulWaveAmplitude, sf::Vec2f(x * 40, y * 40));
        m_shader.setUniform(m_ulBlurRadius, (x + y) * 0.008f);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.shader = &m_shader;
        target.draw(m_text, states);
    }

    explicit WaveBlur(const sf::Font& font, sf::Shader&& shader) :
        m_text(font,
               {.position      = {30.f, 20.f},
                .string        = "Praesent suscipit augue in velit pulvinar hendrerit varius purus aliquam.\n"
                                 "Mauris mi odio, bibendum quis fringilla a, laoreet vel orci. Proin vitae vulputate tortor.\n"
                                 "Praesent cursus ultrices justo, ut feugiat ante vehicula quis.\n"
                                 "Donec fringilla scelerisque mauris et viverra.\n"
                                 "Maecenas adipiscing ornare scelerisque. Nullam at libero elit.\n"
                                 "Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis "
                                 "egestas.\n"
                                 "Nullam leo urna, tincidunt id semper eget, ultricies sed mi.\n"
                                 "Morbi mauris massa, commodo id dignissim vel, lobortis et elit.\n"
                                 "Fusce vel libero sed neque scelerisque venenatis.\n"
                                 "Integer mattis tincidunt quam vitae iaculis.\n"
                                 "Vivamus fringilla sem non velit venenatis fermentum.\n"
                                 "Vivamus varius tincidunt nisi id vehicula.\n"
                                 "Integer ullamcorper, enim vitae euismod rutrum, massa nisl semper ipsum,\n"
                                 "vestibulum sodales sem ante in massa.\n"
                                 "Vestibulum in augue non felis convallis viverra.\n"
                                 "Mauris ultricies dolor sed massa convallis sed aliquet augue fringilla.\n"
                                 "Duis erat eros, porta in accumsan in, blandit quis sem.\n"
                                 "In hac habitasse platea dictumst. Etiam fringilla est id odio dapibus sit amet semper dui "
                                 "laoreet.\n",
                .characterSize = 22u}),
        m_shader(SFML_BASE_MOVE(shader)),
        m_ulWavePhase(m_shader.getUniformLocation("wave_phase").value()),
        m_ulWaveAmplitude(m_shader.getUniformLocation("wave_amplitude").value()),
        m_ulBlurRadius(m_shader.getUniformLocation("blur_radius").value())
    {
    }

private:
    sf::Text                    m_text;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulWavePhase;
    sf::Shader::UniformLocation m_ulWaveAmplitude;
    sf::Shader::UniformLocation m_ulBlurRadius;
};


////////////////////////////////////////////////////////////
// "Storm" vertex shader + "blink" fragment shader
////////////////////////////////////////////////////////////
class StormBlink : public Effect
{
public:
    void update(float time, float x, float y) override
    {
        const float radius = 200 + sf::base::cos(time) * 150;

        m_shader.setUniform(m_ulStormPosition, sf::Vec2f(x * 800, y * 600));
        m_shader.setUniform(m_ulStormInnerRadius, radius / 3);
        m_shader.setUniform(m_ulStormTotalRadius, radius);
        m_shader.setUniform(m_ulBlinkAlpha, 0.5f + sf::base::cos(time * 3) * 0.25f);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.shader = &m_shader;
        target.draw(m_points, sf::PrimitiveType::Points, states);
    }

    explicit StormBlink(sf::Shader&& shader) :
        m_shader(SFML_BASE_MOVE(shader)),
        m_ulStormPosition(m_shader.getUniformLocation("storm_position").value()),
        m_ulStormInnerRadius(m_shader.getUniformLocation("storm_inner_radius").value()),
        m_ulStormTotalRadius(m_shader.getUniformLocation("storm_total_radius").value()),
        m_ulBlinkAlpha(m_shader.getUniformLocation("blink_alpha").value())
    {
        // Create the points
        for (int i = 0; i < 40'000; ++i)
        {
            const auto x = rng.getF(0.f, 800.f);
            const auto y = rng.getF(0.f, 600.f);

            const auto r = rng.getI<sf::base::U8>(0u, 255u);
            const auto g = rng.getI<sf::base::U8>(0u, 255u);
            const auto b = rng.getI<sf::base::U8>(0u, 255u);

            m_points.emplaceBack(sf::Vec2f{x, y}, sf::Color{r, g, b});
        }
    }

private:
    sf::base::Vector<sf::Vertex> m_points;
    sf::Shader                   m_shader;
    sf::Shader::UniformLocation  m_ulStormPosition;
    sf::Shader::UniformLocation  m_ulStormInnerRadius;
    sf::Shader::UniformLocation  m_ulStormTotalRadius;
    sf::Shader::UniformLocation  m_ulBlinkAlpha;
};


////////////////////////////////////////////////////////////
// "Edge" post-effect fragment shader
////////////////////////////////////////////////////////////
class Edge : public Effect
{
public:
    void update(float time, float x, float y) override
    {
        m_shader.setUniform(m_ulEdgeThreshold, sf::base::clamp(1.f - (x + y) / 2.f, 0.f, 1.f));

        // Render the updated scene to the off-screen surface
        m_surface.clear(sf::Color::White);
        m_surface.draw(m_backgroundTexture, {.position = {135.f, 100.f}});

        // Update the position of the moving entities
        constexpr int numEntities = 6;

        for (int i = 0; i < 6; ++i)
        {
            sf::Sprite entity{.textureRect = {{96.f * static_cast<float>(i), 0.f}, {96.f, 96.f}}};

            entity.position =
                {sf::base::cos(0.25f * (time * static_cast<float>(i) + static_cast<float>(numEntities - i))) * 300 + 350,
                 sf::base::sin(0.25f * (time * static_cast<float>(numEntities - i) + static_cast<float>(i))) * 200 + 250};

            m_surface.draw(entity, {.texture = &m_entityTexture});
        }

        m_surface.display();
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        const sf::Texture& texture = m_surface.getTexture();

        states.shader = &m_shader;
        target.draw(texture, states);
    }

    explicit Edge(sf::RenderTexture&& surface, sf::Texture&& backgroundTexture, sf::Texture&& entityTexture, sf::Shader&& shader) :
        m_surface(SFML_BASE_MOVE(surface)),
        m_backgroundTexture(SFML_BASE_MOVE(backgroundTexture)),
        m_entityTexture(SFML_BASE_MOVE(entityTexture)),
        m_shader(SFML_BASE_MOVE(shader)),
        m_ulEdgeThreshold(m_shader.getUniformLocation("edge_threshold").value())
    {
    }

private:
    sf::RenderTexture           m_surface;
    sf::Texture                 m_backgroundTexture;
    sf::Texture                 m_entityTexture;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulEdgeThreshold;
};


////////////////////////////////////////////////////////////
// "Geometry" geometry shader example
////////////////////////////////////////////////////////////
class Geometry : public Effect
{
public:
    void update(float /* time */, float x, float y) override
    {
        // Reset our transformation matrix
        m_transform = sf::Transform::Identity;

        // Move to the center of the window
        m_transform.translate({400.f, 300.f});

        // Rotate everything based on cursor position
        m_transform.rotate(sf::degrees(x * 360.f));

        // Adjust billboard size to scale between 25 and 75
        const float size = 25 + sf::base::fabs(y) * 50;

        // Update the shader parameter
        m_shader.setUniform(m_ulSize, sf::Vec2f{size, size});
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        // Prepare the render state
        states.shader    = &m_shader;
        states.texture   = &m_logoTexture;
        states.transform = m_transform;

        // Draw the point cloud
        target.draw(m_pointCloud, sf::PrimitiveType::Points, states);
    }

    explicit Geometry(sf::Texture&& logoTexture, sf::Shader&& shader) :
        m_logoTexture(SFML_BASE_MOVE(logoTexture)),
        m_shader(SFML_BASE_MOVE(shader)),
        m_ulSize(m_shader.getUniformLocation("size").value()),
        m_pointCloud(10'000)
    {
        // Move the points in the point cloud to random positions
        for (sf::base::SizeT i = 0; i < 10'000; ++i)
        {
            // Spread the coordinates from -480 to +480 so they'll always fill the viewport at 800x600
            m_pointCloud[i].position = {rng.getF(-480.f, 480.f), rng.getF(-480.f, 480.f)};
        }
    }

private:
    sf::Texture                  m_logoTexture;
    sf::Transform                m_transform;
    sf::Shader                   m_shader;
    sf::Shader::UniformLocation  m_ulSize;
    sf::base::Vector<sf::Vertex> m_pointCloud;
};


////////////////////////////////////////////////////////////
// Effect loading factory functions
////////////////////////////////////////////////////////////
sf::base::Optional<Pixelate> tryLoadPixelate()
{
    auto texture = sf::Texture::loadFromFile("resources/background.jpg");
    if (!texture.hasValue())
        return sf::base::nullOpt;

    auto shader = sf::Shader::loadFromFile(
        {.vertexPath = "resources/billboard.vert", .fragmentPath = "resources/pixelate.frag"});
    if (!shader.hasValue())
        return sf::base::nullOpt;

    return sf::base::makeOptional<Pixelate>(SFML_BASE_MOVE(*texture), SFML_BASE_MOVE(*shader));
}

sf::base::Optional<WaveBlur> tryLoadWaveBlur(const sf::Font& font)
{
    auto shader = sf::Shader::loadFromFile({.vertexPath = "resources/wave.vert", .fragmentPath = "resources/blur.frag"});
    if (!shader.hasValue())
        return sf::base::nullOpt;

    return sf::base::makeOptional<WaveBlur>(font, SFML_BASE_MOVE(*shader));
}

sf::base::Optional<StormBlink> tryLoadStormBlink()
{
    auto shader = sf::Shader::loadFromFile({.vertexPath = "resources/storm.vert", .fragmentPath = "resources/blink.frag"});
    if (!shader.hasValue())
        return sf::base::nullOpt;

    return sf::base::makeOptional<StormBlink>(SFML_BASE_MOVE(*shader));
}

sf::base::Optional<Edge> tryLoadEdge()
{
    // Create the off-screen surface
    auto surface = sf::RenderTexture::create({800, 600});
    if (!surface.hasValue())
        return sf::base::nullOpt;

    surface->setSmooth(true);

    // Load the background texture
    auto backgroundTexture = sf::Texture::loadFromFile("resources/sfml.png");
    if (!backgroundTexture.hasValue())
        return sf::base::nullOpt;

    backgroundTexture->setSmooth(true);

    // Load the entity texture
    auto entityTexture = sf::Texture::loadFromFile("resources/devices.png");
    if (!entityTexture.hasValue())
        return sf::base::nullOpt;

    entityTexture->setSmooth(true);

    // Load the shader
    auto shader = sf::Shader::loadFromFile(
        {.vertexPath = "resources/billboard.vert", .fragmentPath = "resources/edge.frag"});
    if (!shader.hasValue())
        return sf::base::nullOpt;

    shader->setUniform(shader->getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);

    return sf::base::makeOptional<Edge>(SFML_BASE_MOVE(*surface),
                                        SFML_BASE_MOVE(*backgroundTexture),
                                        SFML_BASE_MOVE(*entityTexture),
                                        SFML_BASE_MOVE(*shader));
}

sf::base::Optional<Geometry> tryLoadGeometry()
{
    // Check if geometry shaders are supported
    if (!sf::Shader::isGeometryAvailable())
        return sf::base::nullOpt;

    // Load the logo texture
    auto logoTexture = sf::Texture::loadFromFile("resources/logo.png");
    if (!logoTexture.hasValue())
        return sf::base::nullOpt;

    logoTexture->setSmooth(true);

    // Load the shader
    auto shader = sf::Shader::loadFromFile({.vertexPath   = "resources/billboard.vert",
                                            .fragmentPath = "resources/billboard.frag",
                                            .geometryPath = "resources/billboard.geom"});
    if (!shader.hasValue())
        return sf::base::nullOpt;

    shader->setUniform(shader->getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);

    // Set the render resolution (used for proper scaling)
    shader->setUniform(shader->getUniformLocation("resolution").value(), sf::Vec2f{800, 600});

    return sf::base::makeOptional<Geometry>(SFML_BASE_MOVE(*logoTexture), SFML_BASE_MOVE(*shader));
}

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    // Open the application font
    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    // Create the effects
    sf::base::Optional pixelateEffect   = tryLoadPixelate();
    sf::base::Optional waveBlurEffect   = tryLoadWaveBlur(font);
    sf::base::Optional stormBlinkEffect = tryLoadStormBlink();
    sf::base::Optional edgeEffect       = tryLoadEdge();
    sf::base::Optional geometryEffect   = tryLoadGeometry();

    const sf::base::Array<Effect*, 5> effects{pixelateEffect.asPtr(),
                                              waveBlurEffect.asPtr(),
                                              stormBlinkEffect.asPtr(),
                                              edgeEffect.asPtr(),
                                              geometryEffect.asPtr()};

    const sf::base::Array<sf::base::String, 5>
        effectNames{"Pixelate", "Wave + Blur", "Storm + Blink", "Edge Post-effect", "Geometry Shader Billboards"};

    // Index of currently selected effect
    sf::base::SizeT current = 0u;

    // Create the messages background
    const auto textBackgroundTexture = sf::Texture::loadFromFile("resources/text-background.png").value();

    // Create the description text
    sf::Text description(font,
                         {.position         = {10.f, 530.f},
                          .string           = "Current effect: " + effectNames[current],
                          .characterSize    = 20u,
                          .fillColor        = {80, 80, 80},
                          .outlineColor     = sf::Color::White,
                          .outlineThickness = 1.5f});


    // Create the instructions text
    sf::Text instructions(font,
                          {.position         = {280.f, 555.f},
                           .string           = "Press left and right arrows to change the current shader",
                           .characterSize    = 20u,
                           .fillColor        = {80, 80, 80},
                           .outlineColor     = sf::Color::White,
                           .outlineThickness = 1.5f});

    // Create the main window
    constexpr sf::Vec2f windowSize{800.f, 600.f};

    auto window = makeDPIScaledRenderWindow({
        .size      = windowSize.toVec2u(),
        .title     = "SFML Shader",
        .resizable = true,
        .vsync     = true,
    });

    // Start the game loop
    const sf::Clock clock;

    while (true)
    {
        // Process events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, window))
                continue;

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                switch (keyPressed->code)
                {
                    // Left arrow key: previous shader
                    case sf::Keyboard::Key::Left:
                    {
                        if (current == 0)
                            current = effects.size() - 1;
                        else
                            --current;

                        break;
                    }

                    // Right arrow key: next shader
                    case sf::Keyboard::Key::Right:
                    {
                        if (current == effects.size() - 1)
                            current = 0;
                        else
                            ++current;

                        break;
                    }

                    default:
                        break;
                }

                description.setString("Current effect: " + effectNames[current]);
            }
        }

        // If the current example was loaded successfully...
        if (Effect* currentEffect = effects[current])
        {
            // Update the current example
            const auto [x, y] = sf::Mouse::getPosition(window).toVec2f().componentWiseDiv(window.getSize().toVec2f());
            currentEffect->update(clock.getElapsedTime().asSeconds(), x, y);

            // Clear the window
            window.clear(currentEffect == edgeEffect.asPtr() ? sf::Color::White : sf::Color(50, 50, 50));

            // Draw the current example
            window.draw(*currentEffect);
        }
        else
        {
            // Clear the window to grey to make sure the text is always readable
            window.clear(sf::Color(50, 50, 50));
            window.draw(
                sf::Text{font, {.position = {320.f, 200.f}, .string = "Shader not\nsupported", .characterSize = 36u}});
        }

        // Draw the text
        window.draw(textBackgroundTexture, {.position = {0.f, 520.f}, .color = {255, 255, 255, 200}});
        window.draw(instructions);
        window.draw(description);

        // Finally, display the rendered frame on screen
        window.display();
    }
}
