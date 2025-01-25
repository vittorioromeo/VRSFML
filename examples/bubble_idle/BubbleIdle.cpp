#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/RectUtils.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

#include <imgui.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <cstdio>

namespace
{

constexpr sf::Vector2f resolution{1024.f, 768.f};
constexpr auto         resolutionUInt = resolution.toVector2u();

const sf::Vector2f boundaries{1024.f * 10.f, 768.f};

[[nodiscard]] float getRndFloat(const float min, const float max)
{
    static std::random_device         randomDevice;
    static std::default_random_engine randomEngine(randomDevice());
    return std::uniform_real_distribution<float>{min, max}(randomEngine);
}

[[nodiscard]] sf::Vector2f getRndVector2f(const sf::Vector2f mins, const sf::Vector2f maxs)
{
    return {getRndFloat(mins.x, maxs.x), getRndFloat(mins.y, maxs.y)};
}

[[nodiscard]] sf::Vector2f getRndVector2f(const sf::Vector2f maxs)
{
    return getRndVector2f({0.f, 0.f}, maxs);
}

[[nodiscard, gnu::const]] constexpr float remap(const float x, const float oldMin, const float oldMax, const float newMin, const float newMax)
{
    SFML_BASE_ASSERT(oldMax != oldMin);
    return newMin + ((x - oldMin) / (oldMax - oldMin)) * (newMax - newMin);
}

enum class BubbleType
{
    Normal,
    Star
};

struct Bubble
{
    BubbleType   type;
    sf::Sprite   sprite;
    sf::Vector2f velocity;
    float        radius;
};

struct Particle
{
    sf::Sprite   sprite;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float        opacityDecay;
    float        torque;

    void update(const float deltaTime)
    {
        velocity += acceleration * deltaTime;

        sprite.color.a = static_cast<sf::base::U8>(
            sf::base::clamp(sprite.color.a / 255.f - opacityDecay * deltaTime, 0.f, 1.f) * 255.f);

        sprite.position += velocity;
        sprite.rotation += sf::radians(torque * deltaTime);
    }
};

struct TextParticle
{
    sf::Text     text;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float        opacityDecay;
    float        torque;

    void update(const float deltaTime)
    {
        velocity += acceleration * deltaTime;

        auto fillColor    = text.getFillColor();
        auto outlineColor = text.getOutlineColor();

        const auto opacity = static_cast<sf::base::U8>(
            sf::base::clamp(fillColor.a / 255.f - opacityDecay * deltaTime, 0.f, 1.f) * 255.f);

        fillColor.a    = opacity;
        outlineColor.a = opacity;

        text.setFillColor(fillColor);
        text.setOutlineColor(outlineColor);

        text.position += velocity;
        text.rotation += sf::radians(torque * deltaTime);
    }
};

enum class CatType
{
    Normal,
    Unicorn,
    Devil,
};

struct Cat
{
    CatType type;

    sf::Vector2f position;
    sf::Vector2f rangeOffset = {0.f, 0.f};
    float        wobbleTimer = 0.f;

    sf::Sprite sprite;
    sf::Sprite pawSprite;
    float      cooldown = 0.f;

    sf::Text textName;
    sf::Text textStatus;

    float beingDragged = 0.f;
    int   hits         = 0;

    bool update(const float maxCooldown, const float deltaTime)
    {
        bool mustPop = false;

        wobbleTimer += deltaTime * 0.002f;
        sprite.position.x = position.x;
        sprite.position.y = position.y + std::sin(wobbleTimer * 2.f) * 7.5f;

        cooldown += deltaTime;

        if (cooldown >= maxCooldown)
        {
            mustPop  = true;
            cooldown = maxCooldown;
        }

        return mustPop;
    }
};

} // namespace

int main()
{
    //
    // Create an audio context and get the default playback device
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    //
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    // Create the ImGui context
    sf::ImGui::ImGuiContext imGuiContext;

    //
    // Create the render window
    sf::RenderWindow window(
        {.size            = resolutionUInt,
         .title           = "Bubble Idle",
         .vsync           = true,
         .frametimeLimit  = 144,
         .contextSettings = {.antiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel()}});

    //
    // Initialize the ImGui context on the window
    if (!imGuiContext.init(window))
        return -1;


    //
    //
    // Set up texture atlas
    sf::TextureAtlas textureAtlas{sf::Texture::create({4096u, 4096u}).value()};
    textureAtlas.getTexture().setSmooth(true);

    //
    // Load resources
    const auto fontDailyBubble        = sf::Font::openFromFile("resources/dailybubble.ttf").value();
    const auto fontDailyBubbleAtlased = sf::Font::openFromFile("resources/dailybubble.ttf", &textureAtlas).value();

    const auto soundBufferPop   = sf::SoundBuffer::loadFromFile("resources/pop.wav").value();
    const auto soundBufferShine = sf::SoundBuffer::loadFromFile("resources/shine.ogg").value();

    const auto imgBubble128    = sf::Image::loadFromFile("resources/bubble2.png").value();
    const auto imgBubbleStar   = sf::Image::loadFromFile("resources/bubble3.png").value();
    const auto imgCat          = sf::Image::loadFromFile("resources/cat.png").value();
    const auto imgUniCat       = sf::Image::loadFromFile("resources/unicat.png").value();
    const auto imgDevilCat     = sf::Image::loadFromFile("resources/devilcat.png").value();
    const auto imgCatPaw       = sf::Image::loadFromFile("resources/catpaw.png").value();
    const auto imgUniCatPaw    = sf::Image::loadFromFile("resources/unicatpaw.png").value();
    const auto imgParticle     = sf::Image::loadFromFile("resources/particle.png").value();
    const auto imgStarParticle = sf::Image::loadFromFile("resources/starparticle.png").value();

    const auto txrBubble128    = textureAtlas.add(imgBubble128).value();
    const auto txrBubbleStar   = textureAtlas.add(imgBubbleStar).value();
    const auto txrCat          = textureAtlas.add(imgCat).value();
    const auto txrUniCat       = textureAtlas.add(imgUniCat).value();
    const auto txrDevilCat     = textureAtlas.add(imgDevilCat).value();
    const auto txrCatPaw       = textureAtlas.add(imgCatPaw).value();
    const auto txrUniCatPaw    = textureAtlas.add(imgUniCatPaw).value();
    const auto txrParticle     = textureAtlas.add(imgParticle).value();
    const auto txrStarParticle = textureAtlas.add(imgStarParticle).value();

    ImFont* fontImGuiDailyBubble = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/dailybubble.ttf", 32.f);

    //
    // TODO: organize
    sf::Sound soundPop(soundBufferPop);
    sf::Sound soundShine(soundBufferShine);

    sf::Text moneyText{fontDailyBubble,
                       {.position         = {15.f, 70.f},
                        .string           = "$0",
                        .characterSize    = 32u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = sf::Color::DarkPink,
                        .outlineThickness = 2.f}};

    sf::Text comboText{fontDailyBubble,
                       {.position         = {15.f, 105.f},
                        .string           = "x1",
                        .characterSize    = 24u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = sf::Color::DarkPink,
                        .outlineThickness = 1.5f}};

    const auto makeCatNameText = [&](const char* name)
    {
        return sf::Text{fontDailyBubbleAtlased,
                        {.position         = {},
                         .string           = name,
                         .characterSize    = 24u,
                         .fillColor        = sf::Color::White,
                         .outlineColor     = sf::Color::DarkPink,
                         .outlineThickness = 1.5f}};
    };

    const auto makeCatStatusText = [&]()
    {
        return sf::Text{fontDailyBubbleAtlased,
                        {.position         = {},
                         .string           = "status",
                         .characterSize    = 16u,
                         .fillColor        = sf::Color::White,
                         .outlineColor     = sf::Color::DarkPink,
                         .outlineThickness = 1.f}};
    };

    const float gridSize = 64.f;
    const auto  nCellsX  = static_cast<sf::base::SizeT>(sf::base::ceil(boundaries.x / gridSize)) + 1;
    const auto  nCellsY  = static_cast<sf::base::SizeT>(sf::base::ceil(boundaries.y / gridSize)) + 1;

    auto convert2DTo1D = [](sf::base::SizeT col, sf::base::SizeT row, sf::base::SizeT width)
    { return row * width + col; };

    std::vector<std::vector<sf::base::SizeT>> bubbleGrid;
    bubbleGrid.resize(nCellsX * nCellsY);

    std::vector<sf::base::SizeT> bubbleIdSet;

    std::vector<Particle>     particles;
    std::vector<TextParticle> textParticles;

    //
    // Purchasables
    const auto costFunction = [](float baseCost, float nOwned, float growthFactor)
    { return baseCost * std::pow(growthFactor, nOwned); };

    int rewardPerType[2]{
        1,  // Normal
        25, // Star
    };

    float catCooldownPerType[3]{
        1000.f, // Normal
        2000.f, // Unicorn
        1000.f  // Devil
    };

    float catRangePerType[3]{
        96.f, // Normal
        64.f, // Unicorn
        96.f  // Devil
    };

    //
    // Cat names
    std::vector<std::string>
        catNames{"Gorgonzola", "Provolino",  "Pistacchietto", "Ricottina",  "Mozzarellina",  "Tiramisu",
                 "Cannolino",  "Biscottino", "Cannolina",     "Biscottina", "Pistacchietta", "Provolina",
                 "Arancino",   "Limoncello", "Ciabatta",      "Focaccina",  "Amaretto",      "Pallino",
                 "Birillo",    "Trottola",   "Baffo",         "Poldo",      "Fuffi",         "Birba",
                 "Ciccio",     "Pippo",      "Tappo",         "Briciola",   "Braciola",      "Pulce",
                 "Dante",      "Bolla",      "Fragolina",     "Luppolo",    "Sirena",        "Polvere",
                 "Stellina",   "Lunetta",    "Briciolo",      "Fiammetta",  "Nuvoletta",     "Scintilla",
                 "Piuma",      "Fulmine",    "Arcobaleno",    "Stelluccia", "Lucciola",      "Pepita",
                 "Fiocco",     "Girandola",  "Bombetta",      "Fusillo",    "Cicciobello",   "Palloncino",
                 "Joe Biden",  "Trump",      "Obama",         "De Luca",    "Salvini",       "Renzi",
                 "Nutella",    "Vespa",      "Mandolino",     "Ferrari",    "Pavarotti",     "Espresso",
                 "Sir"};

    std::shuffle(catNames.begin(), catNames.end(), std::mt19937{std::random_device{}()});
    sf::base::SizeT nextCatName = 0u;

    //
    // Game state
    std::vector<Bubble> bubbles;
    int                 bubbleTargetCount = 100;

    std::vector<Cat> cats;


    int   money      = 10000;
    int   combo      = 0;
    float comboTimer = 0.f;

    const auto makeRandomBubble = [&]
    {
        const float scaleFactor = getRndFloat(0.05f, 0.15f) * 0.6f;

        sf::Sprite bubbleSprite{.position    = getRndVector2f(boundaries),
                                .scale       = {scaleFactor, scaleFactor},
                                .origin      = txrBubble128.size / 2.f,
                                .textureRect = txrBubble128};

        const float radius = bubbleSprite.textureRect.size.x / 2.f * bubbleSprite.scale.x;

        return Bubble{BubbleType::Normal, bubbleSprite, getRndVector2f({-0.1f, -0.1f}, {0.1f, 0.1f}), radius};
    };

    bubbles.reserve(1000);

    for (int i = 0; i < bubbleTargetCount; ++i)
        bubbles.emplace_back(makeRandomBubble());

    sf::Clock            fpsClock;
    sf::Clock            deltaClock;
    sf::CPUDrawableBatch cpuDrawableBatch;

    sf::base::Optional<sf::Vector2f> dragPosition;
    float                            scroll = 0.f;

    sf::base::Optional<sf::Vector2f> catDragPosition;


    while (true)
    {
        fpsClock.restart();

        sf::base::Optional<sf::Vector2f> clickPosition;

        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (const auto* e0 = event->getIf<sf::Event::TouchMoved>())
            {
                clickPosition.emplace(e0->position.toVector2f());
            }
            else if (const auto* e1 = event->getIf<sf::Event::MouseButtonPressed>())
            {
                clickPosition.emplace(e1->position.toVector2f());

                if (e1->button == sf::Mouse::Button::Left && !catDragPosition.hasValue())
                {
                    catDragPosition.emplace(e1->position.toVector2f());
                }
                else if (e1->button == sf::Mouse::Button::Left)
                {
                    catDragPosition.reset();

                    for (auto& cat : cats)
                        cat.beingDragged = 0.f;
                }

                if (e1->button == sf::Mouse::Button::Right && !dragPosition.hasValue())
                {
                    dragPosition.emplace(e1->position.toVector2f());
                    dragPosition->x += scroll;
                }
            }
            else if (const auto* e2 = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (e2->button == sf::Mouse::Button::Right)
                    dragPosition.reset();
            }
            else if (const auto* e3 = event->getIf<sf::Event::MouseMoved>())
            {
                if (dragPosition.hasValue())
                {
                    scroll = dragPosition->x - static_cast<float>(e3->position.x);
                }
            }
        }

        const auto deltaTime   = deltaClock.restart();
        const auto deltaTimeMs = static_cast<float>(deltaTime.asMilliseconds());

        constexpr float scrollSpeed = 2.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            dragPosition.reset();
            scroll -= scrollSpeed * deltaTimeMs;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            dragPosition.reset();
            scroll += scrollSpeed * deltaTimeMs;
        }

        scroll = sf::base::clamp(scroll, 0.f, boundaries.x - resolution.x);

        sf::View gameView{.center = {resolution.x / 2.f + scroll * 2.f, resolution.y / 2.f}, .size = resolution};
        gameView.center.x = sf::base::clamp(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f);

        const auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);

        for (int i = static_cast<int>(bubbles.size()); i < bubbleTargetCount; ++i)
            bubbles.emplace_back(makeRandomBubble());

        for (sf::base::SizeT i = 0; i < nCellsX; ++i)
            for (sf::base::SizeT j = 0; j < nCellsY; ++j)
                bubbleGrid[convert2DTo1D(i, j, nCellsX)].clear();

        const auto popBubble = [&](BubbleType bubbleType, int reward, int combo, float x, float y)
        {
            sf::Text t{fontDailyBubble,
                       {.position         = {x, y - 10.f},
                        .string           = "+ $" + std::to_string(reward),
                        .characterSize    = 16u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = sf::Color::DarkPink,
                        .outlineThickness = 1.0f}};

            t.scale += sf::Vector2f{0.1f, 0.1f} * static_cast<float>(combo) / 2.f;

            auto& tp = textParticles.emplace_back(
                TextParticle{t,
                             getRndVector2f({-0.1f, -1.65f}, {0.1f, -1.35f}),
                             {0.f, 0.01f},
                             0.0030f,
                             getRndFloat(-0.002f, 0.002f)});

            tp.text.origin = {tp.text.getLocalBounds().size.x / 2.f, tp.text.getLocalBounds().size.y / 2.f};

            soundPop.play(playbackDevice);
            soundPop.setPitch(remap(static_cast<float>(combo), 1, 10, 1.f, 2.f));

            for (int i = 0; i < 32; ++i)
            {
                particles.emplace_back(
                    Particle{sf::Sprite{.position    = {x, y},
                                        .scale       = getRndVector2f({0.1f, 0.1f}, {0.25f, 0.25f}) * 0.5f,
                                        .origin      = txrParticle.size / 2.f,
                                        .textureRect = txrParticle},
                             getRndVector2f({-0.5f, -2.f}, {0.5f, -0.5f}),
                             {0.f, 0.005f},
                             getRndFloat(0.0005f, 0.0015f),
                             getRndFloat(-0.002f, 0.002f)});
            }

            if (bubbleType == BubbleType::Star)
            {
                for (int i = 0; i < 16; ++i)
                {
                    particles.emplace_back(
                        Particle{sf::Sprite{.position    = {x, y},
                                            .scale       = getRndVector2f({0.1f, 0.1f}, {0.25f, 0.25f}) * 0.25f,
                                            .origin      = txrStarParticle.size / 2.f,
                                            .textureRect = txrStarParticle},
                                 getRndVector2f({-0.5f, -2.f}, {0.5f, -0.5f}),
                                 {0.f, 0.005f},
                                 getRndFloat(0.0005f, 0.0015f),
                                 getRndFloat(-0.002f, 0.002f)});
                }
            }
        };

        for (sf::base::SizeT i = 0; i < bubbles.size(); ++i)
        {
            Bubble& bubble            = bubbles[i];
            bubble.sprite.textureRect = bubble.type == BubbleType::Normal ? txrBubble128 : txrBubbleStar;

            auto& pos = bubble.sprite.position;

            const auto cellX = static_cast<sf::base::SizeT>(pos.x / gridSize);
            const auto cellY = static_cast<sf::base::SizeT>(pos.y / gridSize);

            bubbleGrid[convert2DTo1D(cellX, cellY, nCellsX)].push_back(i);
            pos += bubble.velocity * deltaTimeMs;

            if (pos.x - bubble.radius > boundaries.x)
                pos.x = -bubble.radius;
            else if (pos.x + bubble.radius < 0.f)
                pos.x = boundaries.x + bubble.radius;

            if (pos.y - bubble.radius > boundaries.y)
            {
                pos.y             = -bubble.radius;
                bubble.velocity.y = 0.f;
                bubble.type       = BubbleType::Normal;
            }
            else if (pos.y + bubble.radius < 0.f)
                pos.y = boundaries.y + bubble.radius;

            bubble.velocity.y += 0.00005f * deltaTimeMs;

            if (clickPosition.hasValue())
            {
                const auto [x, y] = window.mapPixelToCoords(clickPosition->toVector2i(), gameView);

                if ((x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y) < bubble.radius * bubble.radius)
                {
                    clickPosition.reset();

                    if (combo == 0)
                    {
                        combo      = 1;
                        comboTimer = 750.f;
                    }
                    else
                    {
                        ++combo;
                        comboTimer += 135.f - static_cast<float>(combo) * 5.f;
                    }

                    const auto reward = rewardPerType[static_cast<int>(bubble.type)] * combo;

                    popBubble(bubble.type, reward, combo, x, y);

                    money += reward;
                    bubbles[i] = makeRandomBubble();

                    continue;
                }
            }
        }

        const auto handleCollision = [&](auto i, auto j)
        {
            auto&      iBubble = bubbles[i];
            auto&      jBubble = bubbles[j];
            const auto iRadius = iBubble.radius;
            const auto jRadius = jBubble.radius;

            const auto [aix, aiy] = iBubble.sprite.position;
            const auto [jx, jy]   = jBubble.sprite.position;

            if ((aix - jx) * (aix - jx) + (aiy - jy) * (aiy - jy) < iRadius * iRadius + jRadius * jRadius)
            {
                // Calculate the overlap between the bubbles
                auto  collisionVector = sf::Vector2f(jx - aix, jy - aiy); // Vector from bubble i to bubble j
                float distance        = sf::base::sqrt(collisionVector.x * collisionVector.x +
                                                collisionVector.y * collisionVector.y); // Distance between centers
                float overlap         = (iRadius + jRadius) - distance;                        // Amount of overlap

                // Normalize the collision vector
                sf::Vector2f collisionNormal = collisionVector / distance;

                // Define a "softness" factor to control how quickly the overlap is resolved
                float softnessFactor = 0.05f; // Adjust this value to control the overlap solver (0.1 = 10% per frame)

                // Calculate the displacement needed to resolve the overlap
                sf::Vector2f displacement = collisionNormal * overlap * softnessFactor;

                // Move the bubbles apart based on their masses (heavier bubbles move less)
                float m1        = iRadius * iRadius; // Mass of bubble i (quadratic scaling)
                float m2        = jRadius * jRadius; // Mass of bubble j (quadratic scaling)
                float totalMass = m1 + m2;

                iBubble.sprite.position -= displacement * (m2 / totalMass); // Move bubble i
                jBubble.sprite.position += displacement * (m1 / totalMass);
            }
        };

        for (int ix = 1; ix < static_cast<int>(nCellsX - 1); ++ix)
        {
            for (int iy = 1; iy < static_cast<int>(nCellsY - 1); ++iy)
            {
                bubbleIdSet.clear();

                for (int ox = -1; ox <= 1; ++ox)
                {
                    for (int oy = -1; oy <= 1; ++oy)
                    {
                        const auto idx = convert2DTo1D(static_cast<sf::base::SizeT>(ix + ox),
                                                       static_cast<sf::base::SizeT>(iy + oy),
                                                       nCellsX);

                        bubbleIdSet.insert(bubbleIdSet.end(), bubbleGrid[idx].begin(), bubbleGrid[idx].end());
                    }
                }

                for (sf::base::SizeT i = 0; i < bubbleIdSet.size(); ++i)
                {
                    for (sf::base::SizeT j = i + 1; j < bubbleIdSet.size(); ++j)
                    {
                        handleCollision(bubbleIdSet[i], bubbleIdSet[j]);
                    }
                }
            }
        }

        for (auto& cat : cats)
        {
            if (catDragPosition.hasValue())
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && cat.sprite.getGlobalBounds().contains(mousePos))
                {
                    if (cat.beingDragged < 250.f)
                        cat.beingDragged += deltaTimeMs;

                    break;
                }
            }

            if (cat.beingDragged > 0.f)
                cat.beingDragged -= deltaTimeMs;
        }

        for (auto& cat : cats)
        {
            if (cat.beingDragged > 250.f)
            {
                cat.cooldown   = -1000.f;
                cat.position   = mousePos;
                cat.position.x = sf::base::clamp(cat.position.x, 0.f, boundaries.x);
                cat.position.y = sf::base::clamp(cat.position.y, 0.f, boundaries.y);
            }
        }

        for (auto& cat : cats)
        {
            cat.textName.position   = cat.position + sf::Vector2f{0.f, 48.f};
            cat.textName.origin     = cat.textName.getLocalBounds().size / 2.f;
            cat.textStatus.position = cat.position + sf::Vector2f{0.f, 68.f};
            cat.textStatus.origin   = cat.textStatus.getLocalBounds().size / 2.f;

            if (cat.type == CatType::Normal)
                cat.textStatus.setString(std::to_string(cat.hits) + " pops");
            else if (cat.type == CatType::Unicorn)
                cat.textStatus.setString(std::to_string(cat.hits) + " shines");
            else if (cat.type == CatType::Devil)
                cat.textStatus.setString("TODO");

            const auto maxCooldown = catCooldownPerType[static_cast<int>(cat.type)];
            const auto range       = catRangePerType[static_cast<int>(cat.type)];

            auto diff = cat.pawSprite.position - cat.position - sf::Vector2f{-20.f, 20.f};
            cat.pawSprite.position -= diff * 0.01f * deltaTimeMs;

            if (cat.cooldown < 0.f)
            {
                cat.pawSprite.color.a = 128;
                cat.sprite.color.a    = 128;
            }
            else
            {
                cat.sprite.color.a = 255;
            }

            if (cat.cooldown == maxCooldown && cat.pawSprite.color.a > 10)
                cat.pawSprite.color.a -= static_cast<sf::base::U8>(0.5f * deltaTimeMs);

            if (cat.update(maxCooldown, deltaTimeMs))
            {
                for (auto& bubble : bubbles)
                {
                    if (cat.type == CatType::Unicorn && bubble.type != BubbleType::Normal)
                        continue;

                    const auto [x, y]   = bubble.sprite.position;
                    const auto [cx, cy] = cat.position + cat.rangeOffset;

                    if ((x - cx) * (x - cx) + (y - cy) * (y - cy) < range * range)
                    {
                        cat.pawSprite.position = {x, y};
                        cat.pawSprite.color.a  = 255;
                        cat.pawSprite.rotation = (bubble.sprite.position - cat.position).angle() + sf::degrees(45);

                        if (cat.type == CatType::Unicorn)
                        {
                            bubble.type       = BubbleType::Star;
                            bubble.velocity.y = getRndFloat(-0.1f, -0.05f);
                            soundShine.play(playbackDevice);
                            ++cat.hits;
                        }
                        else if (cat.type == CatType::Normal)
                        {
                            const auto reward = rewardPerType[static_cast<int>(bubble.type)];

                            money += reward;
                            popBubble(bubble.type, reward, 1, x, y);
                            bubble = makeRandomBubble();
                            ++cat.hits;
                        }
                        else if (cat.type == CatType::Devil)
                        {
                            // TODO

                            const auto reward = rewardPerType[static_cast<int>(bubble.type)];

                            money += reward;
                            popBubble(bubble.type, reward, 1, x, y);
                            bubble = makeRandomBubble();
                        }

                        cat.cooldown = 0.f;
                        break;
                    }
                }
            }
        }

        for (auto& textParticle : textParticles)
            textParticle.update(deltaTimeMs);

        std::erase_if(textParticles, [](const auto& textParticle) { return textParticle.text.getFillColor().a <= 0; });

        for (auto& particle : particles)
            particle.update(deltaTimeMs);

        std::erase_if(particles, [](const auto& particle) { return particle.sprite.color.a <= 0; });

        comboTimer -= deltaTimeMs;

        if (comboTimer <= 0.f)
        {
            combo      = 0;
            comboTimer = 0.f;
            // TODO: play combo end sound
        }

        imGuiContext.update(window, deltaTime);

        ImGui::SetNextWindowPos({resolution.x - 15.f, 15.f}, 0, {1.f, 0.f});
        ImGui::PushFont(fontImGuiDailyBubble);
        ImGui::Begin("Menu",
                     nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        char buffer[256];
        char labelBuffer[512];

        const auto makePurchasableButton = [&](const char* label, float baseCost, float growthFactor, float count)
        {
            bool result = false;

            const auto cost = static_cast<int>(costFunction(baseCost, count, growthFactor));
            std::sprintf(buffer, "$%d##%s", cost, label);

            ImGui::BeginDisabled(money < cost);
            ImGui::Text("%s", label);
            ImGui::SameLine();

            if (ImGui::Button(buffer))
            {
                result = true;
                money -= cost;
            }

            ImGui::SetWindowFontScale(0.65f);
            ImGui::SameLine();
            ImGui::Text("%s", labelBuffer);
            ImGui::SetWindowFontScale(1.f);

            ImGui::EndDisabled();

            return result;
        };

        {
            std::sprintf(labelBuffer, "%d bubbles", bubbleTargetCount);
            if (makePurchasableButton("More bubbles", 75.f, 1.5f, static_cast<float>(bubbleTargetCount) / 100.f))
                bubbleTargetCount = static_cast<int>(static_cast<float>(bubbleTargetCount) * 1.25f);
        }

        {
            auto& rewardNormal = rewardPerType[static_cast<int>(BubbleType::Normal)];

            std::sprintf(labelBuffer, "$%d", rewardNormal);
            if (makePurchasableButton("Bubble value", 500.f, 1.5f, static_cast<float>(rewardNormal)))
                rewardNormal += 1;
        }

        const auto countCatsByType = [&](CatType type)
        {
            return static_cast<int>(
                std::count_if(cats.begin(), cats.end(), [type](const auto& cat) { return cat.type == type; }));
        };

        const auto nCatNormal  = countCatsByType(CatType::Normal);
        const auto nCatUnicorn = countCatsByType(CatType::Unicorn);
        const auto nCatDevil   = countCatsByType(CatType::Devil);

        std::sprintf(labelBuffer, "%d cats", nCatNormal);
        if (makePurchasableButton("Cat", 35, 1.5f, static_cast<float>(nCatNormal)))
        {
            for (auto& cat : cats)
                cat.beingDragged = 0.f;

            catDragPosition.emplace(mousePos);

            cats.emplace_back(
                Cat{.type     = CatType::Normal,
                    .position = mousePos,
                    .sprite = {.position = {200.f, 200.f}, .scale{0.2f, 0.2f}, .origin = txrCat.size / 2.f, .textureRect = txrCat},
                    .pawSprite    = {.position = {200.f, 200.f},
                                     .scale{0.1f, 0.1f},
                                     .origin      = txrCatPaw.size / 2.f,
                                     .textureRect = txrCatPaw},
                    .textName     = makeCatNameText(catNames[nextCatName++ % catNames.size()].c_str()),
                    .textStatus   = makeCatStatusText(),
                    .beingDragged = 3000.f});
        }

        if (nCatNormal > 0)
        {
            auto& maxCooldownNormal = catCooldownPerType[static_cast<int>(CatType::Normal)];
            auto& rangeNormal       = catRangePerType[static_cast<int>(CatType::Normal)];

            std::sprintf(labelBuffer, "%.2fs", static_cast<double>(maxCooldownNormal / 1000.f));
            if (makePurchasableButton("Cat cooldown", 50.f, 1.25f, 10.f + (1000.f - maxCooldownNormal) / 10.f))
                maxCooldownNormal *= 0.95f;

            std::sprintf(labelBuffer, "%.2fpx", static_cast<double>(rangeNormal));
            if (makePurchasableButton("Cat range", 10.f, 1.5f, rangeNormal / 10.f))
                rangeNormal *= 1.05f;

            std::sprintf(labelBuffer, "%d unicats", nCatUnicorn);
            if (makePurchasableButton("Unicat", 250, 1.5f, static_cast<float>(nCatUnicorn)))
            {
                for (auto& cat : cats)
                    cat.beingDragged = 0.f;

                catDragPosition.emplace(mousePos);

                cats.emplace_back(
                    Cat{.type         = CatType::Unicorn,
                        .position     = mousePos,
                        .rangeOffset  = {0.f, -100.f},
                        .sprite       = {.position = {200.f, 200.f},
                                         .scale{0.2f, 0.2f},
                                         .origin      = txrUniCat.size / 2.f,
                                         .textureRect = txrUniCat},
                        .pawSprite    = {.position = {200.f, 200.f},
                                         .scale{0.1f, 0.1f},
                                         .origin      = txrUniCatPaw.size / 2.f,
                                         .textureRect = txrUniCatPaw},
                        .textName     = makeCatNameText(catNames[nextCatName++ % catNames.size()].c_str()),
                        .textStatus   = makeCatStatusText(),
                        .beingDragged = 3000.f});
            }

            if (nCatUnicorn > 0)
            {
                auto& maxCooldownUnicorn = catCooldownPerType[static_cast<int>(CatType::Unicorn)];
                auto& rangeUnicorn       = catRangePerType[static_cast<int>(CatType::Unicorn)];

                std::sprintf(labelBuffer, "%.2fs", static_cast<double>(maxCooldownUnicorn / 1000.f));
                if (makePurchasableButton("Unicat cooldown", 50.f, 1.25f, 10.f + (2000.f - maxCooldownUnicorn) / 10.f))
                    maxCooldownUnicorn *= 0.95f;

                std::sprintf(labelBuffer, "%.2fpx", static_cast<double>(rangeUnicorn));
                if (makePurchasableButton("Unicat range", 10.f, 1.5f, rangeUnicorn / 10.f))
                    rangeUnicorn *= 1.05f;
            }
        }

        if (nCatUnicorn > 0)
        {
            std::sprintf(labelBuffer, "%d devilcats", nCatDevil);
            if (makePurchasableButton("Devilcat", 1000.f, 1.5f, static_cast<float>(nCatDevil)))
            {
                for (auto& cat : cats)
                    cat.beingDragged = 0.f;

                catDragPosition.emplace(mousePos);

                cats.emplace_back(
                    Cat{.type         = CatType::Devil,
                        .position     = mousePos,
                        .sprite       = {.position = {200.f, 200.f},
                                         .scale{0.2f, 0.2f},
                                         .origin      = txrDevilCat.size / 2.f,
                                         .textureRect = txrDevilCat},
                        .pawSprite    = {.position = {200.f, 200.f},
                                         .scale{0.1f, 0.1f},
                                         .origin      = txrCatPaw.size / 2.f,
                                         .textureRect = txrCatPaw},
                        .textName     = makeCatNameText(catNames[nextCatName++ % catNames.size()].c_str()),
                        .textStatus   = makeCatStatusText(),
                        .beingDragged = 3000.f});
            }

            if (nCatDevil > 0)
            {
                auto& maxCooldownDevil = catCooldownPerType[static_cast<int>(CatType::Devil)];
                auto& rangeDevil       = catRangePerType[static_cast<int>(CatType::Devil)];

                std::sprintf(labelBuffer, "%.2fs", static_cast<double>(maxCooldownDevil / 1000.f));
                if (makePurchasableButton("Devilcat cooldown", 50.f, 1.15f, 10.f + (1000.f - maxCooldownDevil) / 10.f))
                    maxCooldownDevil *= 0.95f;

                std::sprintf(labelBuffer, "%.2fpx", static_cast<double>(rangeDevil));
                if (makePurchasableButton("Devilcat range", 10.f, 1.5f, rangeDevil / 10.f))
                    rangeDevil *= 1.05f;
            }
        }

        ImGui::End();
        ImGui::PopFont();

        window.clear(sf::Color{84, 72, 81});

        const float progress = remap(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f, 0.f, 100.f);
        window.setView(gameView);

        cpuDrawableBatch.clear();

        for (auto& cat : cats)
        {
            cpuDrawableBatch.add(cat.sprite);
            cpuDrawableBatch.add(cat.pawSprite);

            const auto maxCooldown = catCooldownPerType[static_cast<int>(cat.type)];
            const auto range       = catRangePerType[static_cast<int>(cat.type)];

            sf::CircleShape radiusCircle{{
                .position         = cat.position + cat.rangeOffset,
                .origin           = {range, range},
                .fillColor        = sf::Color::Transparent,
                .outlineThickness = 1.f,
                .radius           = range,
                .pointCount       = 32,
            }};

            const sf::Color colorsByType[3]{
                sf::Color::Pink,
                sf::Color::Blue,
                sf::Color::Red,
            };

            radiusCircle.setOutlineColor(
                {colorsByType[static_cast<int>(cat.type)].r,
                 colorsByType[static_cast<int>(cat.type)].g,
                 colorsByType[static_cast<int>(cat.type)].b,
                 cat.cooldown < 0.f ? static_cast<sf::base::U8>(0u)
                                    : static_cast<sf::base::U8>(cat.cooldown / maxCooldown * 128.f)});

            window.draw(radiusCircle, /* texture */ nullptr);
            cpuDrawableBatch.add(cat.textName);
            cpuDrawableBatch.add(cat.textStatus);
        };

        for (auto& bubble : bubbles)
            cpuDrawableBatch.add(bubble.sprite);

        for (auto& particle : particles)
            cpuDrawableBatch.add(particle.sprite);

        window.draw(cpuDrawableBatch, {.texture = &textureAtlas.getTexture()});

        for (auto& textParticle : textParticles)
            window.draw(textParticle.text);

        window.setView({.center = {resolution.x / 2.f, resolution.y / 2.f}, .size = resolution});

        moneyText.setString("$" + std::to_string(money));
        window.draw(moneyText);
        comboText.setString("x" + std::to_string(combo + 1));
        window.draw(comboText);

        window.draw(sf::RectangleShape{{.position  = {comboText.getCenterRight().x, 110.f},
                                        .fillColor = sf::Color{255, 255, 255, 75},
                                        .size      = {100.f * comboTimer / 700.f, 20.f}}},
                    /* texture */ nullptr);

        //
        // Minimap stuff
        const float minimapScale            = 20.f;
        const float minimapOffset           = 15.f;
        const float minimapOutlineThickness = 2.f;

        sf::RectangleShape minimapBorder{
            {.position         = {minimapOffset, minimapOffset},
             .fillColor        = sf::Color::DarkBabyPink,
             .outlineColor     = sf::Color::White,
             .outlineThickness = minimapOutlineThickness,
             .size             = boundaries / minimapScale + sf::Vector2f{6.f, 6.f}}};

        window.draw(minimapBorder, /* texture */ nullptr);

        sf::RectangleShape minimapIndicator{
            {.position = {minimapOffset + (progress / 100.f) * (boundaries.x - resolution.x) / minimapScale, minimapOffset},
             .fillColor        = sf::Color::Transparent,
             .outlineColor     = sf::Color::Red,
             .outlineThickness = minimapOutlineThickness,
             .size             = resolution / minimapScale + sf::Vector2f{6.f, 6.f}}};

        window.draw(minimapIndicator, /* texture */ nullptr);

        sf::View minimapView{.center = {((resolution.x / 2.f) - (minimapOffset + minimapOutlineThickness)) * minimapScale,
                                        ((resolution.y / 2.f) - (minimapOffset + minimapOutlineThickness)) * minimapScale},
                             .size = resolution * minimapScale};
        window.setView(minimapView);

        window.draw(cpuDrawableBatch, {.texture = &textureAtlas.getTexture()});

        imGuiContext.render(window);
        window.display();

        window.setTitle("FPS: " + std::to_string(1.f / fpsClock.getElapsedTime().asSeconds()));
    }
}
