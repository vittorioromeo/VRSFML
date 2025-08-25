#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/SoA.hpp"     // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/InterferenceSize.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/ThreadPool.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <iostream>
#include <latch>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <cmath>

// --- Configuration Constants ---
constexpr int   SCREEN_WIDTH      = 800;
constexpr int   SCREEN_HEIGHT     = 600;
constexpr int   NUM_BOIDS         = 30000; // Start high to see the slowdown immediately!
constexpr float BOID_SIZE         = 4.0f;
constexpr float NEIGHBOR_RADIUS   = 75.0f;
constexpr float MAX_SPEED         = 350.0f;
constexpr float MAX_FORCE         = 200.0f;
constexpr float SEPARATION_RADIUS = 2.5f; // A smaller radius for strong repulsion

// --- Vector Math Helpers ---
namespace vmath
{
inline float magnitude(sf::Vec2f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline sf::Vec2f normalize(sf::Vec2f v)
{
    float mag = magnitude(v);
    if (mag > 0)
    {
        return v / mag;
    }
    return v;
}

inline void limit(sf::Vec2f& v, float max)
{
    if (magnitude(v) > max)
    {
        v = normalize(v) * max;
    }
}
} // namespace vmath

// --- Naive OOP Design ---
// 1. Abstract base class for any object in our simulation.
// 2. Uses virtual functions, a classic OOP pattern.
class Entity
{
public:
    virtual ~Entity()                                                                            = default;
    virtual void      update(float dt, const std::vector<std::unique_ptr<Entity>>& all_entities) = 0;
    virtual void      draw(sf::RenderWindow& window)                                             = 0;
    virtual sf::Vec2f getPosition() const                                                        = 0;
    virtual sf::Vec2f getVelocity() const                                                        = 0;
};


// 3. The Boid class encapsulates all data (sim & rendering) and logic for a single boid.
class Boid : public Entity
{
private:
    sf::Vec2f       position;
    sf::Vec2f       velocity;
    sf::Vec2f       acceleration;
    sf::CircleShape shape{{.radius = BOID_SIZE, .pointCount = 3u}};

public:
    Boid(float x, float y)
    {
        position = {x, y};
        // Start with a random velocity
        std::random_device               rd;
        std::mt19937                     gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        velocity     = vmath::normalize({(float)dis(gen), (float)dis(gen)}) * MAX_SPEED;
        acceleration = {0, 0};

        // Create the triangle shape for rendering
        // shape.setPointCount(3);
        // shape.setPoint(0, {0, -BOID_SIZE * 2});
        // shape.setPoint(1, {-BOID_SIZE, BOID_SIZE});
        // shape.setPoint(2, {BOID_SIZE, BOID_SIZE});
        shape.setFillColor(sf::Color::Cyan);
        // shape.origin = {0, -BOID_SIZE}; // Set origin for rotation
    }

    sf::Vec2f getPosition() const override
    {
        return position;
    }
    sf::Vec2f getVelocity() const override
    {
        return velocity;
    }

    void update(float dt, const std::vector<std::unique_ptr<Entity>>& all_entities) override
    {
        // --- Flocking Logic ---
        // This is the N^2 performance killer. For every boid, we iterate through every other boid.
        sf::Vec2f separation_sum;
        sf::Vec2f alignment_sum;
        sf::Vec2f cohesion_sum;
        int       neighbors_count      = 0;
        int       separation_neighbors = 0;

        for (const auto& other_entity : all_entities)
        {
            if (other_entity.get() == this)
                continue;

            float dist = vmath::magnitude(other_entity->getPosition() - this->position);

            // --- NEW: Stronger, dedicated separation logic ---
            if (dist > 0 && dist < SEPARATION_RADIUS)
            {
                sf::Vec2f diff = this->position - other_entity->getPosition();
                // The key change: Weight by 1/dist^2 for a much stronger push at close range.
                separation_sum += vmath::normalize(diff) / (dist * dist);
                separation_neighbors++;
            }

            if (dist > 0 && dist < NEIGHBOR_RADIUS)
            {
                // Separation: Steer away from neighbors
                sf::Vec2f diff = this->position - other_entity->getPosition();
                separation_sum += vmath::normalize(diff) / dist; // Weight by distance

                // Alignment: Steer towards the average velocity of neighbors
                alignment_sum += other_entity->getVelocity();

                // Cohesion: Steer towards the average position of neighbors
                cohesion_sum += other_entity->getPosition();

                neighbors_count++;
            }
        }

        if (separation_neighbors > 0)
        {
            separation_sum /= (float)separation_neighbors;
            if (vmath::magnitude(separation_sum) > 0)
            {
                sf::Vec2f separation_steer = vmath::normalize(separation_sum) * MAX_SPEED - velocity;
                vmath::limit(separation_steer, MAX_FORCE * 2.0f); // Allow separation to be stronger than other forces
                acceleration += separation_steer * 1.f;          // Give it a very high weight
            }
        }

        if (neighbors_count > 0)
        {
            // Separation
            separation_sum /= (float)neighbors_count;
            if (vmath::magnitude(separation_sum) > 0)
            {
                sf::Vec2f separation_steer = vmath::normalize(separation_sum) * MAX_SPEED - velocity;
                vmath::limit(separation_steer, MAX_FORCE);
                acceleration += separation_steer * 1.5f; // Weight separation more
            }

            // Alignment
            alignment_sum /= (float)neighbors_count;
            sf::Vec2f alignment_steer = vmath::normalize(alignment_sum) * MAX_SPEED - velocity;
            vmath::limit(alignment_steer, MAX_FORCE);
            acceleration += alignment_steer * 1.0f;

            // Cohesion
            cohesion_sum /= (float)neighbors_count;
            sf::Vec2f cohesion_steer = vmath::normalize(cohesion_sum - position) * MAX_SPEED - velocity;
            vmath::limit(cohesion_steer, MAX_FORCE);
            acceleration += cohesion_steer * 1.0f;
        }

        // --- Physics Update ---
        velocity += acceleration * dt;
        vmath::limit(velocity, MAX_SPEED);
        position += velocity * dt;
        acceleration = {0, 0}; // Reset acceleration each frame

        // --- Screen Wrap ---
        if (position.x < 0)
            position.x += SCREEN_WIDTH;

        if (position.y < 0)
            position.y += SCREEN_HEIGHT;

        if (position.x > SCREEN_WIDTH)
            position.x -= SCREEN_WIDTH;

        if (position.y > SCREEN_HEIGHT)
            position.y -= SCREEN_HEIGHT;

        // --- Update Shape for Drawing ---
        shape.position = position;
        shape.rotation = velocity.angle() + sf::degrees(45.f);
    }

    void draw(sf::RenderWindow& window) override
    {
        window.draw(shape);
    }
};

constexpr sf::Vec2f resolution{800.f, 600.f};

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


    // --- Data Storage: The "Bad" Way ---
    // 4. `std::vector` of unique pointers. This guarantees each Boid is a separate heap
    //    allocation, scattering data all over memory and leading to cache misses.
    std::vector<std::unique_ptr<Entity>> entities;
    std::random_device                   rd;
    std::mt19937                         gen(rd());
    std::uniform_real_distribution<>     x_dist(0, SCREEN_WIDTH);
    std::uniform_real_distribution<>     y_dist(0, SCREEN_HEIGHT);

    for (int i = 0; i < NUM_BOIDS; ++i)
    {
        entities.push_back(std::make_unique<Boid>(x_dist(gen), y_dist(gen)));
    }

    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();


    // --- UI Text for FPS and Boid Count ---
    sf::Text text(font,
                  {
                      .characterSize = 24,
                      .fillColor     = sf::Color::White,
                  });

    sf::Clock clock;
    float     last_time = 0;

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, resolution, window))
                continue;
        }

        // --- Delta Time Calculation for smooth movement ---
        float current_time = clock.getElapsedTime().asSeconds();
        float dt           = current_time - last_time;
        last_time          = current_time;
        float fps          = 1.0f / dt;

        // --- Main Update Loop ---
        // 5. Virtual dispatch for every single boid on every frame.
        for (auto& entity : entities)
        {
            entity->update(dt, entities);
        }

        // --- UI Update ---
        text.setString("Boids: " + std::to_string(NUM_BOIDS) + "\nFPS: " + std::to_string((int)fps));

        // --- Drawing ---
        window.clear(sf::Color(30, 30, 50)); // Dark blue background
        for (auto& entity : entities)
        {
            entity->draw(window);
        }
        window.draw(text);
        window.display();
    }
}
