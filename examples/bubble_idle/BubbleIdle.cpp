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
#include "SFML/System/Rect.hpp"
#include "SFML/System/RectUtils.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

#include <imgui.h>

#include <_mingw_stat64.h>
#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstdio>


namespace
{
////////////////////////////////////////////////////////////
using sf::base::SizeT;

////////////////////////////////////////////////////////////
constexpr sf::Vector2f resolution{1366.f, 768.f};
constexpr auto         resolutionUInt = resolution.toVector2u();

////////////////////////////////////////////////////////////
constexpr sf::Vector2f boundaries{1366.f * 10.f, 768.f};

////////////////////////////////////////////////////////////
constexpr sf::Color colorBlueOutline{50, 84, 135};

////////////////////////////////////////////////////////////
[[nodiscard]] float getRndFloat(const float min, const float max)
{
    static std::random_device         randomDevice;
    static std::default_random_engine randomEngine(randomDevice());
    return std::uniform_real_distribution<float>{min, max}(randomEngine);
}

////////////////////////////////////////////////////////////
[[nodiscard]] sf::Vector2f getRndVector2f(const sf::Vector2f mins, const sf::Vector2f maxs)
{
    return {getRndFloat(mins.x, maxs.x), getRndFloat(mins.y, maxs.y)};
}

////////////////////////////////////////////////////////////
[[nodiscard]] sf::Vector2f getRndVector2f(const sf::Vector2f maxs)
{
    return getRndVector2f({0.f, 0.f}, maxs);
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr float remap(const float x, const float oldMin, const float oldMax, const float newMin, const float newMax)
{
    SFML_BASE_ASSERT(oldMax != oldMin);
    return newMin + ((x - oldMin) / (oldMax - oldMin)) * (newMax - newMin);
}

////////////////////////////////////////////////////////////
enum class BubbleType
{
    Normal,
    Star
};

////////////////////////////////////////////////////////////
struct Bubble
{
    BubbleType   type;
    sf::Sprite   sprite;
    sf::Vector2f velocity;
    float        radius;
};

////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////
enum class CatType
{
    Normal,
    Unicorn,
    Devil,
    Witch,
    Astromeow,
};

////////////////////////////////////////////////////////////
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
    //
    // Create an audio context and get the default playback device
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    //
    //
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    //
    // Create the render window
    sf::RenderWindow window(
        {.size            = resolutionUInt,
         .title           = "Bubble Idle",
         .vsync           = true,
         .frametimeLimit  = 144,
         .contextSettings = {.antiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel()}});

    //
    //
    // Create and initialize the ImGui context
    sf::ImGui::ImGuiContext imGuiContext;
    if (!imGuiContext.init(window))
        return -1;

    //
    //
    // Set up texture atlas
    sf::TextureAtlas textureAtlas{sf::Texture::create({4096u, 4096u}, {.smooth = true}).value()};

    //
    //
    // Load and initialize resources
    /* --- Fonts */
    const auto fontDailyBubble = sf::Font::openFromFile("resources/dailybubble.ttf", &textureAtlas).value();

    /* --- ImGui fonts */
    ImFont* fontImGuiDailyBubble = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/dailybubble.ttf", 32.f);

    /* --- Sound buffers */
    const auto soundBufferPop   = sf::SoundBuffer::loadFromFile("resources/pop.wav").value();
    const auto soundBufferShine = sf::SoundBuffer::loadFromFile("resources/shine.ogg").value();
    const auto soundBufferClick = sf::SoundBuffer::loadFromFile("resources/click2.wav").value();

    /* --- Images */
    const auto imgBubble128    = sf::Image::loadFromFile("resources/bubble2.png").value();
    const auto imgBubbleStar   = sf::Image::loadFromFile("resources/bubble3.png").value();
    const auto imgCat          = sf::Image::loadFromFile("resources/cat.png").value();
    const auto imgUniCat       = sf::Image::loadFromFile("resources/unicat.png").value();
    const auto imgDevilCat     = sf::Image::loadFromFile("resources/devilcat.png").value();
    const auto imgCatPaw       = sf::Image::loadFromFile("resources/catpaw.png").value();
    const auto imgUniCatPaw    = sf::Image::loadFromFile("resources/unicatpaw.png").value();
    const auto imgParticle     = sf::Image::loadFromFile("resources/particle.png").value();
    const auto imgStarParticle = sf::Image::loadFromFile("resources/starparticle.png").value();
    const auto imgWitchCat     = sf::Image::loadFromFile("resources/witchcat.png").value();
    const auto imgAstromeowCat = sf::Image::loadFromFile("resources/astromeow.png").value();

    /* --- Textures */
    const auto txLogo       = sf::Texture::loadFromFile("resources/logo.png", {.smooth = true}).value();
    const auto txBackground = sf::Texture::loadFromFile("resources/background.png", {.smooth = true}).value();

    /* --- Texture atlas rects */
    const auto txrWhiteDot     = textureAtlas.add(graphicsContext.getBuiltInWhiteDotTexture()).value();
    const auto txrBubble128    = textureAtlas.add(imgBubble128).value();
    const auto txrBubbleStar   = textureAtlas.add(imgBubbleStar).value();
    const auto txrCat          = textureAtlas.add(imgCat).value();
    const auto txrUniCat       = textureAtlas.add(imgUniCat).value();
    const auto txrDevilCat     = textureAtlas.add(imgDevilCat).value();
    const auto txrCatPaw       = textureAtlas.add(imgCatPaw).value();
    const auto txrUniCatPaw    = textureAtlas.add(imgUniCatPaw).value();
    const auto txrParticle     = textureAtlas.add(imgParticle).value();
    const auto txrStarParticle = textureAtlas.add(imgStarParticle).value();
    const auto txrWitchCat     = textureAtlas.add(imgWitchCat).value();
    const auto txrAstromeowCat = textureAtlas.add(imgAstromeowCat).value();

    /* --- Sounds */
    sf::Sound soundPop(soundBufferPop);
    sf::Sound soundShine(soundBufferShine);
    sf::Sound soundClick(soundBufferClick);

    //
    //
    // TODO: organize
    sf::Text moneyText{fontDailyBubble,
                       {.position         = {15.f, 70.f},
                        .string           = "$0",
                        .characterSize    = 32u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = colorBlueOutline,
                        .outlineThickness = 2.f}};

    sf::Text comboText{fontDailyBubble,
                       {.position         = {15.f, 105.f},
                        .string           = "x1",
                        .characterSize    = 24u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = colorBlueOutline,
                        .outlineThickness = 1.5f}};

    const auto makeCatNameText = [&](const char* name)
    {
        return sf::Text{fontDailyBubble,
                        {.string           = name,
                         .characterSize    = 24u,
                         .fillColor        = sf::Color::White,
                         .outlineColor     = colorBlueOutline,
                         .outlineThickness = 1.5f}};
    };

    const auto makeCatStatusText = [&]()
    {
        return sf::Text{fontDailyBubble,
                        {.characterSize    = 16u,
                         .fillColor        = sf::Color::White,
                         .outlineColor     = colorBlueOutline,
                         .outlineThickness = 1.f}};
    };

    const auto makeParticle = [&](const float x, const float y, const sf::FloatRect& txr, const float scaleMult)
    {
        return Particle{sf::Sprite{.position    = {x, y},
                                   .scale       = getRndVector2f({0.1f, 0.1f}, {0.25f, 0.25f}) * scaleMult,
                                   .origin      = txr.size / 2.f,
                                   .textureRect = txr},
                        getRndVector2f({-0.5f, -2.f}, {0.5f, -0.5f}),
                        {0.f, 0.005f},
                        getRndFloat(0.0005f, 0.0015f),
                        getRndFloat(-0.002f, 0.002f)};
    };

    //
    //
    // Spatial partitioning
    const float gridSize = 64.f;
    const auto  nCellsX  = static_cast<SizeT>(sf::base::ceil(boundaries.x / gridSize)) + 1;
    const auto  nCellsY  = static_cast<SizeT>(sf::base::ceil(boundaries.y / gridSize)) + 1;

    const auto convert2DTo1D = [](const SizeT x, const SizeT y, const SizeT width) { return y * width + x; };

    std::vector<SizeT> bubbleIndices;          // Flat list of all bubble indices in all cells
    std::vector<SizeT> cellStartIndices;       // Tracks where each cell's data starts in `bubbleIndices`
    std::vector<SizeT> cellInsertionPositions; // Temporary copy of `cellStartIndices` to track insertion points

    std::vector<Particle>     particles;
    std::vector<TextParticle> textParticles;

    //
    //
    // Purchasables
    const auto costFunction = [](float baseCost, float nOwned, float growthFactor)
    { return baseCost * std::pow(growthFactor, nOwned); };

    int rewardPerType[2]{
        1,  // Normal
        25, // Star
    };

    float catCooldownPerType[5]{
        1000.f, // Normal
        2000.f, // Unicorn
        1000.f, // Devil
        1000.f, // Witch
        1000.f  // Astromeow
    };

    float catRangePerType[5]{
        96.f, // Normal
        64.f, // Unicorn
        96.f, // Devil
        96.f, // Witch
        96.f  // Astromeow
    };

    //
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
                 "Sir",        "Nocciolina", "Fluffy",        "Costanzo",   "Mozart",        "DB",
                 "Soniuccia",  "Pupi",       "Pupetta",       "Genitore 1", "Genitore 2",    "Stonks",
                 "Carotina",   "Waffle",     "Pancake",       "Muffin",     "Cupcake",       "Donut",
                 "Jinx",       "Miao",       "Arnold",        "Granita",    "Leone",         "Pangocciolo"};

    std::shuffle(catNames.begin(), catNames.end(), std::mt19937{std::random_device{}()});
    auto getNextCatName = [&, nextCatName = 0u]() mutable { return catNames[nextCatName++ % catNames.size()]; };

    //
    // Game state
    std::vector<Bubble> bubbles;
    int                 bubbleTargetCount = 10000;

    std::vector<Cat> cats;


    int   money      = 10000;
    int   combo      = 0;
    float comboTimer = 0.f;

    const auto makeRandomBubble = [&]
    {
        const float scaleFactor = getRndFloat(0.07f, 0.17f) * 0.61f;

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
    sf::base::Optional<sf::Vector2f> catDragPositionFinger;

    std::unordered_map<unsigned int, sf::Vector2f> fingerPositions;

    while (true)
    {
        fpsClock.restart();

        sf::base::Optional<sf::Vector2f> clickPosition;

        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
            if (const auto* e = event->getIf<sf::Event::TouchBegan>())
            {
                fingerPositions[e->finger] = e->position.toVector2f();
            }
            else if (const auto* e = event->getIf<sf::Event::TouchEnded>())
            {
                fingerPositions.erase(e->finger);
            }
            else if (const auto* e = event->getIf<sf::Event::TouchMoved>())
            {
                fingerPositions[e->finger] = e->position.toVector2f();
                catDragPositionFinger.emplace(e->position.toVector2f());
            }
            else if (const auto* e = event->getIf<sf::Event::MouseButtonPressed>())
            {
                clickPosition.emplace(e->position.toVector2f());

                if (e->button == sf::Mouse::Button::Left && !catDragPosition.hasValue())
                {
                    catDragPosition.emplace(e->position.toVector2f());
                }
                else if (e->button == sf::Mouse::Button::Left)
                {
                    catDragPosition.reset();

                    for (auto& cat : cats)
                        cat.beingDragged = 0.f;
                }

                if (e->button == sf::Mouse::Button::Right && !dragPosition.hasValue())
                {
                    dragPosition.emplace(e->position.toVector2f());
                    dragPosition->x += scroll;
                }
            }
            else if (const auto* e = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (e->button == sf::Mouse::Button::Right)
                    dragPosition.reset();
            }
            else if (const auto* e = event->getIf<sf::Event::MouseMoved>())
            {
                if (dragPosition.hasValue())
                {
                    scroll = dragPosition->x - static_cast<float>(e->position.x);
                }
            }
#pragma clang diagnostic pop
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

        if (fingerPositions.size() == 2 && !dragPosition.hasValue())
        {
            dragPosition.emplace(fingerPositions.begin()->second);
            dragPosition->x += scroll;
        }

        if (fingerPositions.size() == 2 && dragPosition.hasValue())
        {
            const auto v0 = fingerPositions.begin()->second;
            const auto v1 = (++fingerPositions.begin())->second;

            scroll = dragPosition->x - static_cast<float>((v0.x + v1.x) / 2.f);
        }

        if (dragPosition.hasValue() && fingerPositions.empty() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
        {
            dragPosition.reset();
        }

        if (fingerPositions.empty())
        {
            catDragPositionFinger.reset();
        }

        scroll = sf::base::clamp(scroll, 0.f, boundaries.x - resolution.x);

        sf::View gameView{.center = {resolution.x / 2.f + scroll * 2.f, resolution.y / 2.f}, .size = resolution};
        gameView.center.x = sf::base::clamp(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f);

        const auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);

        for (int i = static_cast<int>(bubbles.size()); i < bubbleTargetCount; ++i)
            bubbles.emplace_back(makeRandomBubble());

        //
        //
        // Update spatial partitioning
        cellStartIndices.clear();
        cellStartIndices.resize(nCellsX * nCellsY + 1, 0); // +1 for prefix sum

        const auto computeGridRange = [&](const auto& bubble)
        {
            const auto [minX, minY] = bubble.sprite.getTopLeft();
            const auto [maxX, maxY] = bubble.sprite.getBottomRight();

            struct Result
            {
                SizeT xCellStartIdx, yCellStartIdx, xCellEndIdx, yCellEndIdx;
            };

            return Result{sf::base::max(SizeT{0u}, static_cast<SizeT>(minX / gridSize)),
                          sf::base::max(SizeT{0u}, static_cast<SizeT>(minY / gridSize)),
                          sf::base::min(SizeT{nCellsX - 1}, static_cast<SizeT>(maxX / gridSize)),
                          sf::base::min(SizeT{nCellsY - 1}, static_cast<SizeT>(maxY / gridSize))};
        };

        //
        // First Pass (Counting):
        // - Calculate how many bubbles will be placed in each grid cell.
        for (auto& bubble : bubbles)
        {
            const auto [xCellStartIdx, yCellStartIdx, xCellEndIdx, yCellEndIdx] = computeGridRange(bubble);

            // For each cell the bubble covers, increment the count
            for (SizeT x = xCellStartIdx; x <= xCellEndIdx; ++x)
                for (SizeT y = yCellStartIdx; y <= yCellEndIdx; ++y)
                {
                    const SizeT cellIdx = convert2DTo1D(x, y, nCellsX);
                    ++cellStartIndices[cellIdx + 1]; // +1 offsets for prefix sum
                }
        }

        //
        // Second Pass (Prefix Sum):
        // - Calculate the starting index for each cell’s data in `bubbleIndices`.

        // Prefix sum to compute start indices
        for (SizeT i = 1; i < cellStartIndices.size(); ++i)
            cellStartIndices[i] += cellStartIndices[i - 1];

        bubbleIndices.resize(cellStartIndices.back()); // Total bubbles across all cells

        // Used to track where to insert the next bubble index into the `bubbleIndices` buffer for each cell
        cellInsertionPositions.assign(cellStartIndices.begin(), cellStartIndices.end());

        //
        // Third Pass (Populating):
        // - Place the bubble indices into the correct positions in the `bubbleIndices` buffer.
        for (SizeT i = 0; i < bubbles.size(); ++i)
        {
            const auto& bubble                                                  = bubbles[i];
            const auto [xCellStartIdx, yCellStartIdx, xCellEndIdx, yCellEndIdx] = computeGridRange(bubble);

            // Insert the bubble index into all overlapping cells
            for (SizeT x = xCellStartIdx; x <= xCellEndIdx; ++x)
            {
                for (SizeT y = yCellStartIdx; y <= yCellEndIdx; ++y)
                {
                    const SizeT cellIdx      = convert2DTo1D(x, y, nCellsX);
                    const SizeT insertPos    = cellInsertionPositions[cellIdx]++;
                    bubbleIndices[insertPos] = i;
                }
            }
        }

        const auto popBubble = [&](BubbleType bubbleType, int reward, int combo, float x, float y)
        {
            sf::Text t{fontDailyBubble,
                       {.position         = {x, y - 10.f},
                        .string           = "+ $" + std::to_string(reward),
                        .characterSize    = 16u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = colorBlueOutline,
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
                particles.emplace_back(makeParticle(x, y, txrParticle, 0.5f));

            if (bubbleType == BubbleType::Star)
                for (int i = 0; i < 16; ++i)
                    particles.emplace_back(makeParticle(x, y, txrStarParticle, 0.25f));
        };

        for (auto& bubble : bubbles)
        {
            bubble.sprite.textureRect = bubble.type == BubbleType::Normal ? txrBubble128 : txrBubbleStar;

            auto& pos = bubble.sprite.position;

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

            const auto handleClick = [&](const sf::Vector2f clickPos)
            {
                const auto [x, y] = window.mapPixelToCoords(clickPos.toVector2i(), gameView);

                if ((x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y) >= bubble.radius * bubble.radius)
                    return false;

                if (combo == 0)
                {
                    combo      = 1;
                    comboTimer = 775.f;
                }
                else
                {
                    ++combo;
                    comboTimer += 135.f - static_cast<float>(combo) * 5.f;
                }

                const auto reward = rewardPerType[static_cast<int>(bubble.type)] * combo;

                popBubble(bubble.type, reward, combo, x, y);

                money += reward;
                bubble = makeRandomBubble();

                return true;
            };

            if (clickPosition.hasValue())
            {
                if (handleClick(*clickPosition))
                {
                    clickPosition.reset();
                    continue;
                }
            }

            if (fingerPositions.size() == 1)
                for (const auto& [finger, fingerPos] : fingerPositions)
                {
                    if (handleClick(fingerPos))
                        break;
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

                // Define a "softness" factor to control how quickly the overlap is resolved
                float softnessFactor = 0.025f; // Adjust this value to control the overlap solver (0.1 = 10% per frame)

                // Calculate the displacement needed to resolve the overlap
                sf::Vector2f displacement = collisionVector.normalized() * overlap * softnessFactor;

                // Move the bubbles apart based on their masses (heavier bubbles move less)
                float m1        = iRadius * iRadius; // Mass of bubble i (quadratic scaling)
                float m2        = jRadius * jRadius; // Mass of bubble j (quadratic scaling)
                float totalMass = m1 + m2;

                iBubble.sprite.position -= displacement * (m2 / totalMass); // Move bubble i
                jBubble.sprite.position += displacement * (m1 / totalMass);
            }
        };

        for (SizeT ix = 0; ix < nCellsX; ++ix)
        {
            for (SizeT iy = 0; iy < nCellsY; ++iy)
            {
                const SizeT cellIdx = convert2DTo1D(ix, iy, nCellsX);
                const SizeT start   = cellStartIndices[cellIdx];
                const SizeT end     = cellStartIndices[cellIdx + 1];

                // Iterate over all bubbles in this cell
                for (SizeT i = start; i < end; ++i)
                {
                    const SizeT bubbleA = bubbleIndices[i];
                    for (SizeT j = i + 1; j < end; ++j)
                    {
                        const SizeT bubbleB = bubbleIndices[j];
                        handleCollision(bubbleA, bubbleB);
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

            if (catDragPositionFinger.hasValue())
            {
                const auto fingerPos = window.mapPixelToCoords(catDragPositionFinger->toVector2i(), gameView);

                if (cat.sprite.getGlobalBounds().contains(fingerPos))
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
                cat.cooldown = -1000.f;

                if (catDragPositionFinger.hasValue())
                {
                    const auto fingerPos = window.mapPixelToCoords(catDragPositionFinger->toVector2i(), gameView);
                    cat.position         = fingerPos;
                }
                else
                {
                    cat.position = mousePos;
                }

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

                            for (int i = 0; i < 4; ++i)
                                particles.emplace_back(makeParticle(x, y, txrStarParticle, 0.25f));

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
                soundClick.play(playbackDevice);

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
            if (makePurchasableButton("More bubbles", 65.f, 1.5f, static_cast<float>(bubbleTargetCount) / 100.f))
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

        const auto makeCat =
            [&](const CatType catType, const sf::Vector2f rangeOffset, const sf::FloatRect& txr, const sf::FloatRect& txrPaw)
        {
            return Cat{.type        = catType,
                       .position    = mousePos,
                       .rangeOffset = rangeOffset,
                       .sprite = {.position = {200.f, 200.f}, .scale{0.2f, 0.2f}, .origin = txr.size / 2.f, .textureRect = txr},
                       .pawSprite    = {.position = {200.f, 200.f},
                                        .scale{0.1f, 0.1f},
                                        .origin      = txrPaw.size / 2.f,
                                        .textureRect = txrPaw},
                       .textName     = makeCatNameText(getNextCatName().c_str()),
                       .textStatus   = makeCatStatusText(),
                       .beingDragged = 3000.f};
        };

        const auto makeCatModifiers =
            [&](const char* labelCooldown, const char* labelRange, const CatType catType, const float initialCooldown)
        {
            auto& maxCooldown = catCooldownPerType[static_cast<int>(catType)];
            auto& range       = catRangePerType[static_cast<int>(catType)];

            std::sprintf(labelBuffer, "%.2fs", static_cast<double>(maxCooldown / 1000.f));
            if (makePurchasableButton(labelCooldown, 50.f, 1.25f, 10.f + (initialCooldown - maxCooldown) / 10.f))
                maxCooldown *= 0.95f;

            std::sprintf(labelBuffer, "%.2fpx", static_cast<double>(range));
            if (makePurchasableButton(labelRange, 10.f, 1.5f, range / 10.f))
                range *= 1.05f;
        };

        std::sprintf(labelBuffer, "%d cats", nCatNormal);
        if (makePurchasableButton("Cat", 35, 1.5f, static_cast<float>(nCatNormal)))
        {
            for (auto& cat : cats)
                cat.beingDragged = 0.f;

            catDragPosition.emplace(mousePos);
            cats.emplace_back(makeCat(CatType::Normal, {0.f, 0.f}, txrCat, txrCatPaw));
        }

        if (nCatNormal > 0)
        {
            makeCatModifiers("Cat cooldown", "Cat range", CatType::Normal, 1000.f);

            std::sprintf(labelBuffer, "%d unicats", nCatUnicorn);
            if (makePurchasableButton("Unicat", 250, 1.5f, static_cast<float>(nCatUnicorn)))
            {
                for (auto& cat : cats)
                    cat.beingDragged = 0.f;

                catDragPosition.emplace(mousePos);
                cats.emplace_back(makeCat(CatType::Unicorn, {0.f, -100.f}, txrUniCat, txrUniCatPaw));
            }

            if (nCatUnicorn > 0)
                makeCatModifiers("Unicat cooldown", "Unicat range", CatType::Unicorn, 2000.f);
        }

        if (nCatUnicorn > 0)
        {
            std::sprintf(labelBuffer, "%d devilcats", nCatDevil);
            if (makePurchasableButton("Devilcat", 1000.f, 1.5f, static_cast<float>(nCatDevil)))
            {
                for (auto& cat : cats)
                    cat.beingDragged = 0.f;

                catDragPosition.emplace(mousePos);
                cats.emplace_back(makeCat(CatType::Devil, {0.f, 0.f}, txrDevilCat, txrCatPaw));
            }

            if (nCatDevil > 0)
                makeCatModifiers("Devilcat cooldown", "Devilcat range", CatType::Devil, 1000.f);
        }

        ImGui::End();
        ImGui::PopFont();

        window.clear(sf::Color{157, 171, 191});

        const float progress = remap(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f, 0.f, 100.f);
        window.setView(gameView);

        window.draw(txBackground);

        cpuDrawableBatch.clear();

        for (auto& cat : cats)
        {
            cpuDrawableBatch.add(cat.sprite);
            cpuDrawableBatch.add(cat.pawSprite);

            const auto maxCooldown = catCooldownPerType[static_cast<int>(cat.type)];
            const auto range       = catRangePerType[static_cast<int>(cat.type)];

            sf::CircleShape radiusCircle{{
                .position           = cat.position + cat.rangeOffset,
                .origin             = {range, range},
                .outlineTextureRect = txrWhiteDot,
                .fillColor          = sf::Color::Transparent,
                .outlineThickness   = 1.f,
                .radius             = range,
                .pointCount         = 32,
            }};

            constexpr sf::Color colorsByType[3]{
                sf::Color::Blue,
                sf::Color::Purple,
                sf::Color::Red,
            };

            radiusCircle.setOutlineColor(
                {colorsByType[static_cast<int>(cat.type)].r,
                 colorsByType[static_cast<int>(cat.type)].g,
                 colorsByType[static_cast<int>(cat.type)].b,
                 cat.cooldown < 0.f ? static_cast<sf::base::U8>(0u)
                                    : static_cast<sf::base::U8>(cat.cooldown / maxCooldown * 128.f)});

            cpuDrawableBatch.add(radiusCircle);
            cpuDrawableBatch.add(cat.textName);
            cpuDrawableBatch.add(cat.textStatus);
        };

        for (const auto& bubble : bubbles)
            cpuDrawableBatch.add(bubble.sprite);

        for (const auto& particle : particles)
            cpuDrawableBatch.add(particle.sprite);

        for (const auto& textParticle : textParticles)
            cpuDrawableBatch.add(textParticle.text);

        window.draw(cpuDrawableBatch, {.texture = &textureAtlas.getTexture()});

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
             .fillColor        = sf::Color{102, 131, 173},
             .outlineColor     = sf::Color::White,
             .outlineThickness = minimapOutlineThickness,
             .size             = boundaries / minimapScale + sf::Vector2f{6.f, 6.f}}};

        window.draw(minimapBorder, /* texture */ nullptr);

        sf::RectangleShape minimapIndicator{
            {.position = {minimapOffset + (progress / 100.f) * (boundaries.x - resolution.x) / minimapScale, minimapOffset},
             .fillColor        = sf::Color::Transparent,
             .outlineColor     = sf::Color::Blue,
             .outlineThickness = minimapOutlineThickness,
             .size             = resolution / minimapScale + sf::Vector2f{6.f, 6.f}}};


        sf::View minimapView{.center = {((resolution.x / 2.f) - (minimapOffset + minimapOutlineThickness)) * minimapScale,
                                        ((resolution.y / 2.f) - (minimapOffset + minimapOutlineThickness)) * minimapScale},
                             .size = resolution * minimapScale};
        window.setView(minimapView);

        window.draw(txBackground);
        window.draw(cpuDrawableBatch, {.texture = &textureAtlas.getTexture()});

        window.setView({.center = {resolution.x / 2.f, resolution.y / 2.f}, .size = resolution});
        window.draw(minimapIndicator, /* texture */ nullptr);


        imGuiContext.render(window);

        //
        // Splash screen
        static float logoOpacity = 1000.f;
        logoOpacity -= deltaTimeMs * 0.5f;

        sf::Sprite logoSprite{.position    = resolution / 2.f,
                              .scale       = {0.70f, 0.70f},
                              .origin      = txLogo.getRect().size / 2.f,
                              .textureRect = txLogo.getRect(),
                              .color       = sf::Color{255,
                                                 255,
                                                 255,
                                                 static_cast<sf::base::U8>(sf::base::clamp(logoOpacity, 0.f, 255.f))}};

        // TODO: meow sound
        window.draw(logoSprite, txLogo);

        window.display();

        //
        // Debug stuff
        window.setTitle("FPS: " + std::to_string(1.f / fpsClock.getElapsedTime().asSeconds()));
    }
}
