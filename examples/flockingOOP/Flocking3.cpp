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

#include <algorithm> // For std::min/max
#include <iostream>
#include <latch>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <cmath>

// --- Configuration Constants ---
constexpr int   SCREEN_WIDTH      = 1920;
constexpr int   SCREEN_HEIGHT     = 1080;
constexpr int   NUM_BOIDS         = 30000; // A good number to show the benefit of the grid
constexpr float BOID_SIZE         = 4.0f;
constexpr float NEIGHBOR_RADIUS   = 50.0f;
constexpr float SEPARATION_RADIUS = 12.0f; // A smaller radius for strong repulsion
constexpr float MAX_SPEED         = 400.0f;
constexpr float MAX_FORCE         = 150.0f;

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
        return v / mag;
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

// Forward declare for the grid
class Entity;

// --- NEW: Spatial Grid for the OOP Approach ---
// This class partitions the 2D space to accelerate neighbor searches.
// CRUCIAL FOR THE TALK: Notice it stores raw `Entity*` pointers. It does not own the
// objects, but this pointer-based approach has performance implications (cache misses)
// that you will contrast with the DOD/index-based grid.
class SpatialGridOOP
{
private:
    int                               m_cols;
    int                               m_rows;
    float                             m_cellSize;
    std::vector<std::vector<Entity*>> m_cells;

public:
    SpatialGridOOP(float screenWidth, float screenHeight, float cellSize) : m_cellSize(cellSize)
    {
        m_cols = static_cast<int>(std::ceil(screenWidth / m_cellSize));
        m_rows = static_cast<int>(std::ceil(screenHeight / m_cellSize));
        m_cells.resize(m_rows * m_cols);
    }

    void clear()
    {
        for (auto& cell : m_cells)
        {
            cell.clear();
        }
    }

    void add(Entity* entity); // Implemented after Entity is fully defined

    void getPotentialNeighbors(Entity* entity, std::vector<Entity*>& out_neighbors);
};


// --- Naive OOP Design ---
class Entity
{
public:
    virtual ~Entity() = default;
    // MODIFIED: Update now takes a smaller list of potential neighbors
    virtual void      update(float dt, const std::vector<Entity*>& potential_neighbors) = 0;
    virtual void      draw(sf::RenderWindow& window)                                    = 0;
    virtual sf::Vec2f getPosition() const                                               = 0;
    virtual sf::Vec2f getVelocity() const                                               = 0;
};

std::size_t index2Dto1D(int x, int y, int cols)
{
    return y * cols + x;
}

// Now we can implement the grid methods that depend on Entity
void SpatialGridOOP::add(Entity* entity)
{
    sf::Vec2f pos   = entity->getPosition();
    int       cellX = static_cast<int>(pos.x / m_cellSize);
    int       cellY = static_cast<int>(pos.y / m_cellSize);

    cellX = std::max(0, std::min(m_cols - 1, cellX));
    cellY = std::max(0, std::min(m_rows - 1, cellY));

    m_cells[index2Dto1D(cellX, cellY, m_cols)].push_back(entity);
}

void SpatialGridOOP::getPotentialNeighbors(Entity* entity, std::vector<Entity*>& out_neighbors)
{
    out_neighbors.clear();
    sf::Vec2f pos   = entity->getPosition();
    int       cellX = static_cast<int>(pos.x / m_cellSize);
    int       cellY = static_cast<int>(pos.y / m_cellSize);

    for (int y = cellY - 1; y <= cellY + 1; ++y)
    {
        for (int x = cellX - 1; x <= cellX + 1; ++x)
        {
            if (x >= 0 && x < m_cols && y >= 0 && y < m_rows)
            {
                auto& cell = m_cells[index2Dto1D(x, y, m_cols)];
                out_neighbors.insert(out_neighbors.end(), cell.begin(), cell.end());
            }
        }
    }
}


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
        std::random_device               rd;
        std::mt19937                     gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        velocity     = vmath::normalize({(float)dis(gen), (float)dis(gen)}) * MAX_SPEED;
        acceleration = {0, 0};


        shape.setFillColor(sf::Color::Cyan);
        shape.origin = {0, -BOID_SIZE};
    }

    sf::Vec2f getPosition() const override
    {
        return position;
    }
    sf::Vec2f getVelocity() const override
    {
        return velocity;
    }

    // MODIFIED: Takes the smaller neighbor list and has cleaned-up logic.
    void update(float dt, const std::vector<Entity*>& potential_neighbors) override
    {
        sf::Vec2f separation_sum;
        sf::Vec2f alignment_sum;
        sf::Vec2f cohesion_sum;
        int       separation_neighbors = 0;
        int       flock_neighbors      = 0;

        for (Entity* other_entity : potential_neighbors)
        {
            if (other_entity == this)
                continue;

            float dist = vmath::magnitude(other_entity->getPosition() - this->position);

            if (dist > 0 && dist < SEPARATION_RADIUS)
            {
                sf::Vec2f diff = this->position - other_entity->getPosition();
                separation_sum += vmath::normalize(diff) / (dist * dist);
                separation_neighbors++;
            }

            if (dist > 0 && dist < NEIGHBOR_RADIUS)
            {
                alignment_sum += other_entity->getVelocity();
                cohesion_sum += other_entity->getPosition();
                flock_neighbors++;
            }
        }

        if (separation_neighbors > 0)
        {
            separation_sum /= (float)separation_neighbors;
            if (vmath::magnitude(separation_sum) > 0)
            {
                sf::Vec2f steer = vmath::normalize(separation_sum) * MAX_SPEED - velocity;
                vmath::limit(steer, MAX_FORCE * 2.0f);
                acceleration += steer * 1.25f;
            }
        }

        if (flock_neighbors > 0)
        {
            alignment_sum /= (float)flock_neighbors;
            sf::Vec2f steer = vmath::normalize(alignment_sum) * MAX_SPEED - velocity;
            vmath::limit(steer, MAX_FORCE);
            acceleration += steer * 1.0f;

            cohesion_sum /= (float)flock_neighbors;
            steer = vmath::normalize(cohesion_sum - position) * MAX_SPEED - velocity;
            vmath::limit(steer, MAX_FORCE);
            acceleration += steer * 1.0f;
        }

        velocity += acceleration * dt;
        vmath::limit(velocity, MAX_SPEED);
        position += velocity * dt;
        acceleration = {0, 0};

        if (position.x < 0)
            position.x += SCREEN_WIDTH;
        if (position.y < 0)
            position.y += SCREEN_HEIGHT;
        if (position.x > SCREEN_WIDTH)
            position.x -= SCREEN_WIDTH;
        if (position.y > SCREEN_HEIGHT)
            position.y -= SCREEN_HEIGHT;

        shape.position = (position);
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
    // --- NEW: Create the Spatial Grid and a reusable neighbor vector ---
    SpatialGridOOP       grid(SCREEN_WIDTH, SCREEN_HEIGHT, NEIGHBOR_RADIUS);
    std::vector<Entity*> potential_neighbors;
    potential_neighbors.reserve(256); // Pre-allocate some memory

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

        float current_time = clock.getElapsedTime().asSeconds();
        float dt           = current_time - last_time;
        last_time          = current_time;
        float fps          = (dt > 0) ? (1.0f / dt) : 0.0f;

        // --- NEW: Main Update Loop using the Grid ---
        // 1. Clear the grid and rebuild it with current entity positions.
        grid.clear();
        for (auto& entity : entities)
        {
            grid.add(entity.get());
        }

        // 2. Update each entity, providing it ONLY with its potential neighbors.
        for (auto& entity : entities)
        {
            grid.getPotentialNeighbors(entity.get(), potential_neighbors);
            entity->update(dt, potential_neighbors);
        }

        text.setString(
            "Boids: " + std::to_string(NUM_BOIDS) + "\nFPS: " + std::to_string((int)fps) + "\nMode: OOP + Grid");

        window.clear(sf::Color(30, 30, 50));
        for (auto& entity : entities)
        {
            entity->draw(window);
        }
        window.draw(text);
        window.display();
    }

    return 0;
}
