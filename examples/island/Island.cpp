////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/System/Vector3.hpp"

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <algorithm>
#include <array>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <vector>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>


namespace
{
// Width and height of the application window
constexpr sf::Vector2u windowSize(800, 600);

// Resolution of the generated terrain
constexpr sf::Vector2u resolution(800, 600);

// Thread pool parameters
constexpr unsigned int threadCount  = 4;
constexpr unsigned int blockCount   = 32;
constexpr unsigned int rowBlockSize = (resolution.y / blockCount) + 1;

struct WorkItem
{
    sf::Vertex*  targetBuffer{};
    unsigned int index{};
};

struct ThreadPool
{
    std::mutex               poolMutex;
    std::vector<std::thread> threads;
    std::queue<WorkItem>     workQueue;
    unsigned int             pendingWorkCount    = 0u;
    bool                     bufferUploadPending = false;
    bool                     finished            = false;

    explicit ThreadPool() = default;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&)      = delete;

    ~ThreadPool()
    {
        {
            const std::lock_guard lock(poolMutex);
            finished = true;
        }

        for (std::thread& t : threads)
            t.join();
    }
};

struct Setting
{
    const char* name{};
    float*      value{};
};

// Terrain noise parameters
constexpr int perlinOctaves = 3;

float perlinFrequency     = 7.f;
float perlinFrequencyBase = 4.f;

// Terrain generation parameters
float heightBase          = 0.f;
float edgeFactor          = 0.9f;
float edgeDropoffExponent = 1.5f;

float snowcapHeight = 0.6f;

// Terrain lighting parameters
float heightFactor  = static_cast<float>(windowSize.y) / 2.f;
float heightFlatten = 3.f;
float lightFactor   = 0.7f;


////////////////////////////////////////////////////////////
/// Get the terrain elevation at the given coordinates.
///
////////////////////////////////////////////////////////////
float getElevation(sf::Vector2u position)
{
    const sf::Vector2f normalized = position.toVector2f().componentWiseDiv(resolution.toVector2f()) -
                                    sf::Vector2f(0.5f, 0.5f);

    float elevation = 0.f;

    for (int i = 0; i < perlinOctaves; ++i)
    {
        const sf::Vector2f scaled = normalized * perlinFrequency * static_cast<float>(std::pow(perlinFrequencyBase, i));
        elevation += stb_perlin_noise3(scaled.x, scaled.y, 0, 0, 0, 0) *
                     static_cast<float>(std::pow(perlinFrequencyBase, -i));
    }

    elevation = (elevation + 1.f) / 2.f;

    const float distance = 2.f * normalized.length();
    elevation            = (elevation + heightBase) * (1.f - edgeFactor * std::pow(distance, edgeDropoffExponent));
    elevation            = std::clamp(elevation, 0.f, 1.f);

    return elevation;
}


////////////////////////////////////////////////////////////
/// Get the terrain moisture at the given coordinates.
///
////////////////////////////////////////////////////////////
float getMoisture(sf::Vector2u position)
{
    const sf::Vector2f normalized = position.toVector2f().componentWiseDiv(resolution.toVector2f()) -
                                    sf::Vector2f(0.5f, 0.5f);
    const sf::Vector2f transformed = normalized * 4.f + sf::Vector2f(0.5f, 0.5f);

    const float moisture = stb_perlin_noise3(transformed.x, transformed.y, 0, 0, 0, 0);

    return (moisture + 1.f) / 2.f;
}


////////////////////////////////////////////////////////////
/// Get the lowlands terrain color for the given moisture.
///
////////////////////////////////////////////////////////////
sf::Color colorFromFloats(float r, float g, float b)
{
    return {static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b)};
}

sf::Color getLowlandsTerrainColor(float moisture)
{
    if (moisture < 0.27f)
        return colorFromFloats(240, 240, 180);

    if (moisture < 0.3f)
        return colorFromFloats(240 - (240 * (moisture - 0.27f) / 0.03f),
                               240 - (40 * (moisture - 0.27f) / 0.03f),
                               180 - (180 * (moisture - 0.27f) / 0.03f));

    if (moisture < 0.4f)
        return colorFromFloats(0, 200, 0);

    if (moisture < 0.48f)
        return colorFromFloats(0, 200 - (40 * (moisture - 0.4f) / 0.08f), 0);

    if (moisture < 0.6f)
        return colorFromFloats(0, 160, 0);

    if (moisture < 0.7f)
        return colorFromFloats((34 * (moisture - 0.6f) / 0.1f),
                               160 - (60 * (moisture - 0.6f) / 0.1f),
                               (34 * (moisture - 0.6f) / 0.1f));

    return colorFromFloats(34, 100, 34);
}


////////////////////////////////////////////////////////////
/// Get the highlands terrain color for the given elevation
/// and moisture.
///
////////////////////////////////////////////////////////////
sf::Color getHighlandsTerrainColor(float elevation, float moisture)
{
    const sf::Color lowlandsColor = getLowlandsTerrainColor(moisture);

    const sf::Color color = moisture < 0.6f ? sf::Color(112, 128, 144)
                                            : colorFromFloats(112 + (110 * (moisture - 0.6f) / 0.4f),
                                                              128 + (56 * (moisture - 0.6f) / 0.4f),
                                                              144 - (9 * (moisture - 0.6f) / 0.4f));

    const float factor = std::min((elevation - 0.4f) / 0.1f, 1.f);

    return colorFromFloats(lowlandsColor.r * (1.f - factor) + color.r * factor,
                           lowlandsColor.g * (1.f - factor) + color.g * factor,
                           lowlandsColor.b * (1.f - factor) + color.b * factor);
}


////////////////////////////////////////////////////////////
/// Get the snowcap terrain color for the given elevation
/// and moisture.
///
////////////////////////////////////////////////////////////
sf::Color getSnowcapTerrainColor(float elevation, float moisture)
{
    const sf::Color highlandsColor = getHighlandsTerrainColor(elevation, moisture);

    const float factor = std::min((elevation - snowcapHeight) / 0.05f, 1.f);

    return {static_cast<std::uint8_t>(highlandsColor.r * (1.f - factor) + 255 * factor),
            static_cast<std::uint8_t>(highlandsColor.g * (1.f - factor) + 255 * factor),
            static_cast<std::uint8_t>(highlandsColor.b * (1.f - factor) + 255 * factor)};
}


////////////////////////////////////////////////////////////
/// Get the terrain color for the given elevation and
/// moisture.
///
////////////////////////////////////////////////////////////
sf::Color getTerrainColor(float elevation, float moisture)
{
    if (elevation < 0.11f)
        return {0, 0, static_cast<std::uint8_t>(elevation / 0.11f * 74.f + 181.f)};

    if (elevation < 0.14f)
        return {static_cast<std::uint8_t>(std::pow((elevation - 0.11f) / 0.03f, 0.3f) * 48.f),
                static_cast<std::uint8_t>(std::pow((elevation - 0.11f) / 0.03f, 0.3f) * 48.f),
                255};

    if (elevation < 0.16f)
        return {static_cast<std::uint8_t>((elevation - 0.14f) * 128.f / 0.02f + 48.f),
                static_cast<std::uint8_t>((elevation - 0.14f) * 128.f / 0.02f + 48.f),
                static_cast<std::uint8_t>(127.f + (0.16f - elevation) * 128.f / 0.02f)};

    if (elevation < 0.17f)
        return {240, 230, 140};

    if (elevation < 0.4f)
        return getLowlandsTerrainColor(moisture);

    if (elevation < snowcapHeight)
        return getHighlandsTerrainColor(elevation, moisture);

    return getSnowcapTerrainColor(elevation, moisture);
}


////////////////////////////////////////////////////////////
/// Compute a compressed representation of the surface
/// normal based on the given coordinates, and the elevation
/// of the 4 adjacent neighbours.
///
////////////////////////////////////////////////////////////
sf::Vector2f computeNormal(float left, float right, float bottom, float top)
{
    const sf::Vector3f deltaX(1, 0, (std::pow(right, heightFlatten) - std::pow(left, heightFlatten)) * heightFactor);
    const sf::Vector3f deltaY(0, 1, (std::pow(top, heightFlatten) - std::pow(bottom, heightFlatten)) * heightFactor);

    sf::Vector3f crossProduct = deltaX.cross(deltaY);

    // Scale cross product to make z component 1.f so we can drop it
    crossProduct /= crossProduct.z;

    // Return "compressed" normal
    return {crossProduct.x, crossProduct.y};
}


////////////////////////////////////////////////////////////
/// Compute the vertex representing the terrain at the given
/// coordinates.
///
////////////////////////////////////////////////////////////
sf::Vertex computeVertex(sf::Vector2u position)
{
    static constexpr auto scalingFactors = windowSize.toVector2f().componentWiseDiv(resolution.toVector2f());

    return {.position  = position.toVector2f().componentWiseMul(scalingFactors),
            .color     = getTerrainColor(getElevation(position), getMoisture(position)),
            .texCoords = computeNormal(getElevation(position - sf::Vector2u(1, 0)),
                                       getElevation(position + sf::Vector2u(1, 0)),
                                       getElevation(position + sf::Vector2u(0, 1)),
                                       getElevation(position - sf::Vector2u(0, 1)))};
}


////////////////////////////////////////////////////////////
/// Process a terrain generation work item. Use the vector
/// of vertices as scratch memory and upload the data to
/// the vertex buffer when done.
///
////////////////////////////////////////////////////////////
void processWorkItem(std::vector<sf::Vertex>& vertices, const WorkItem& workItem)
{
    const unsigned int rowStart = rowBlockSize * workItem.index;

    if (rowStart >= resolution.y)
        return;

    const unsigned int rowEnd   = std::min(rowStart + rowBlockSize, resolution.y);
    const unsigned int rowCount = rowEnd - rowStart;

    for (unsigned int y = rowStart; y < rowEnd; ++y)
    {
        for (unsigned int x = 0; x < resolution.x; ++x)
        {
            const unsigned int arrayIndexBase = ((y - rowStart) * resolution.x + x) * 6;

            // Top left corner (first triangle)
            if (x > 0)
            {
                vertices[arrayIndexBase + 0] = vertices[arrayIndexBase - 6 + 5];
            }
            else if (y > rowStart)
            {
                vertices[arrayIndexBase + 0] = vertices[arrayIndexBase - resolution.x * 6 + 1];
            }
            else
            {
                vertices[arrayIndexBase + 0] = computeVertex({x, y});
            }

            // Bottom left corner (first triangle)
            if (x > 0)
            {
                vertices[arrayIndexBase + 1] = vertices[arrayIndexBase - 6 + 2];
            }
            else
            {
                vertices[arrayIndexBase + 1] = computeVertex({x, y + 1});
            }

            // Bottom right corner (first triangle)
            vertices[arrayIndexBase + 2] = computeVertex({x + 1, y + 1});

            // Top left corner (second triangle)
            vertices[arrayIndexBase + 3] = vertices[arrayIndexBase + 0];

            // Bottom right corner (second triangle)
            vertices[arrayIndexBase + 4] = vertices[arrayIndexBase + 2];

            // Top right corner (second triangle)
            if (y > rowStart)
            {
                vertices[arrayIndexBase + 5] = vertices[arrayIndexBase - resolution.x * 6 + 2];
            }
            else
            {
                vertices[arrayIndexBase + 5] = computeVertex({x + 1, y});
            }
        }
    }

    // Copy the resulting geometry from our thread-local buffer into the target buffer
    std::memcpy(workItem.targetBuffer + (resolution.x * rowStart * 6),
                vertices.data(),
                sizeof(sf::Vertex) * resolution.x * rowCount * 6);
}


////////////////////////////////////////////////////////////
/// Worker thread entry point. We use a thread pool to avoid
/// the heavy cost of constantly recreating and starting
/// new threads whenever we need to regenerate the terrain.
///
////////////////////////////////////////////////////////////
void threadFunction(ThreadPool& threadPool)
{
    std::vector<sf::Vertex> vertices(resolution.x * rowBlockSize * 6);

    WorkItem workItem{nullptr, 0};

    // Loop until the application exits
    for (;;)
    {
        workItem.targetBuffer = nullptr;

        // Check if there are new work items in the queue
        {
            const std::lock_guard lock(threadPool.poolMutex);

            if (threadPool.finished)
                return;

            if (!threadPool.workQueue.empty())
            {
                workItem = threadPool.workQueue.front();
                threadPool.workQueue.pop();
            }
        }

        // If we didn't receive a new work item, keep looping
        if (workItem.targetBuffer == nullptr)
        {
            sf::sleep(sf::milliseconds(10));
            continue;
        }

        processWorkItem(vertices, workItem);

        {
            const std::lock_guard lock(threadPool.poolMutex);
            --threadPool.pendingWorkCount;
        }
    }
}


////////////////////////////////////////////////////////////
/// Terrain generation entry point. This queues up the
/// generation work items which the worker threads dequeue
/// and process.
///
////////////////////////////////////////////////////////////
void generateTerrain(ThreadPool& threadPool, sf::Vertex* buffer)
{
    threadPool.bufferUploadPending = true;

    // Make sure the work queue is empty before queuing new work
    for (;;)
    {
        {
            const std::lock_guard lock(threadPool.poolMutex);

            if (threadPool.pendingWorkCount == 0u)
                break;
        }

        sf::sleep(sf::milliseconds(10));
    }

    // Queue all the new work items
    {
        const std::lock_guard lock(threadPool.poolMutex);

        for (unsigned int i = 0u; i < blockCount; ++i)
            threadPool.workQueue.emplace(buffer, i);

        threadPool.pendingWorkCount = blockCount;
    }
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

    // Load the terrain shader
    auto       terrainShader = sf::Shader::loadFromFile("resources/terrain.vert", "resources/terrain.frag").value();
    const auto ulLightFactor = terrainShader.getUniformLocation("lightFactor").value();

    // Load the font
    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    // Create the window of the application
    sf::RenderWindow window({.size{windowSize}, .title = "SFML Island", .resizable = false, .vsync = true});

    // Create all of our graphics resources
    sf::Text hudText(font,
                     {.position         = {5.f, 5.f},
                      .characterSize    = 14,
                      .fillColor        = sf::Color::White,
                      .outlineColor     = sf::Color::Black,
                      .outlineThickness = 2.f});

    sf::Text statusText(font,
                        {.string           = "Generating Terrain...",
                         .characterSize    = 28,
                         .fillColor        = sf::Color::White,
                         .outlineColor     = sf::Color::Black,
                         .outlineThickness = 2.f});

    sf::RenderStates terrainStates;
    sf::VertexBuffer terrain(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Static);

    // Staging buffer for our terrain data that we will upload to our VertexBuffer
    std::vector<sf::Vertex> terrainStagingBuffer;

    // Create a thread pool
    ThreadPool threadPool;

    // Start up our thread pool
    for (unsigned int i = 0; i < threadCount; ++i)
        threadPool.threads.emplace_back([&threadPool] { threadFunction(threadPool); });

    // Create our VertexBuffer with enough space to hold all the terrain geometry
    if (!terrain.create(resolution.x * resolution.y * 6))
    {
        sf::cErr() << "Failed to create vertex buffer" << sf::endL;
        return EXIT_FAILURE;
    }

    // Resize the staging buffer to be able to hold all the terrain geometry
    terrainStagingBuffer.resize(resolution.x * resolution.y * 6);

    // Generate the initial terrain
    generateTerrain(threadPool, terrainStagingBuffer.data());

    // Set up the render states
    terrainStates = sf::RenderStates{.shader = &terrainShader};

    // Center the status text
    statusText.position = (windowSize.toVector2f() - statusText.getLocalBounds().size) / 2.f;

    // Set up an array of pointers to our settings for arrow navigation
    constexpr std::array<Setting, 9> settings = {
        {{"perlinFrequency", &perlinFrequency},
         {"perlinFrequencyBase", &perlinFrequencyBase},
         {"heightBase", &heightBase},
         {"edgeFactor", &edgeFactor},
         {"edgeDropoffExponent", &edgeDropoffExponent},
         {"snowcapHeight", &snowcapHeight},
         {"heightFactor", &heightFactor},
         {"heightFlatten", &heightFlatten},
         {"lightFactor", &lightFactor}}};

    std::size_t currentSetting = 0;

    std::ostringstream oss;
    sf::Clock          clock;

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;

            // Arrow key pressed:
            if (event->is<sf::Event::KeyPressed>())
            {
                switch (event->getIf<sf::Event::KeyPressed>()->code)
                {
                    case sf::Keyboard::Key::Enter:
                        generateTerrain(threadPool, terrainStagingBuffer.data());
                        break;
                    case sf::Keyboard::Key::Down:
                        currentSetting = (currentSetting + 1) % settings.size();
                        break;
                    case sf::Keyboard::Key::Up:
                        currentSetting = (currentSetting + settings.size() - 1) % settings.size();
                        break;
                    case sf::Keyboard::Key::Left:
                        *(settings[currentSetting].value) -= 0.1f;
                        break;
                    case sf::Keyboard::Key::Right:
                        *(settings[currentSetting].value) += 0.1f;
                        break;
                    default:
                        break;
                }
            }
        }

        // Clear, draw graphics objects and display
        window.clear();

        window.draw(statusText);

        {
            const std::lock_guard lock(threadPool.poolMutex);

            // Don't bother updating/drawing the VertexBuffer while terrain is being regenerated
            if (threadPool.pendingWorkCount == 0u)
            {
                // If there is new data pending to be uploaded to the VertexBuffer, do it now
                if (threadPool.bufferUploadPending)
                {
                    if (!terrain.update(terrainStagingBuffer.data()))
                    {
                        sf::cErr() << "Failed to update vertex buffer" << sf::endL;
                        return EXIT_SUCCESS;
                    }

                    threadPool.bufferUploadPending = false;
                }

                terrainShader.setUniform(ulLightFactor, lightFactor);
                window.draw(terrain, terrainStates);
            }
        }

        // Update and draw the HUD text
        oss.str("");
        oss << "Frame:  " << clock.restart().asMilliseconds() << "ms\n"
            << "perlinOctaves:  " << perlinOctaves << "\n\n"
            << "Use the arrow keys to change the values.\nUse the return key to regenerate the terrain.\n\n";

        for (std::size_t i = 0; i < settings.size(); ++i)
            oss << ((i == currentSetting) ? ">>  " : "       ") << settings[i].name << ":  " << *(settings[i].value)
                << '\n';

        hudText.setString(oss.str());

        window.draw(hudText);

        // Display things on screen
        window.display();
    }
}

// TODO P1: use base thread pool
