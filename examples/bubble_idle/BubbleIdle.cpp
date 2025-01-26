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
#include "SFML/Audio/Music.hpp"
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

#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <string>
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
    static std::default_random_engine randomEngine(std::random_device{}());
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
struct TextShakeEffect
{
    float grow  = 0.f;
    float angle = 0.f;

    void bump(const float strength)
    {
        grow  = strength;
        angle = getRndFloat(-grow * 0.2f, grow * 0.2f);
    }

    void update(const float deltaTimeMs)
    {
        if (grow > 0.f)
            grow -= deltaTimeMs * 0.015f;

        if (angle != 0.f)
        {
            const float sign = angle > 0.f ? 1.f : -1.f;
            angle -= sign * deltaTimeMs * 0.005f;

            if (sign * angle < 0.f)
                angle = 0.f;
        }
    }

    void applyToText(sf::Text& text) const
    {
        text.scale    = {1.f + grow * 0.2f, 1.f + grow * 0.2f};
        text.rotation = sf::radians(angle);
    }
};

////////////////////////////////////////////////////////////
enum class BubbleType : sf::base::U8
{
    Normal = 0u,
    Star   = 1u,
    Bomb   = 2u
};

////////////////////////////////////////////////////////////
struct Bubble
{
    sf::Vector2f position;
    sf::Vector2f velocity;

    float scale;
    float radius;
    float rotation;

    BubbleType type;

    void update(const float deltaTime)
    {
        position += velocity * deltaTime;
    }

    void applyToSprite(sf::Sprite& sprite) const
    {
        sprite.position = position;
        sprite.scale    = {scale, scale};
        sprite.rotation = sf::radians(rotation);
    }
};

////////////////////////////////////////////////////////////
enum class ParticleType : sf::base::U8
{
    Bubble = 0u,
    Star   = 1u,
    Fire   = 2u,
    Hex    = 3u,
};

////////////////////////////////////////////////////////////
struct Particle
{
    sf::Vector2f position;
    sf::Vector2f velocity;

    float scale;
    float accelerationY;

    float opacity;
    float opacityDecay;

    float rotation;
    float torque;

    ParticleType type;

    void update(const float deltaTime)
    {
        velocity.y += accelerationY * deltaTime;
        position += velocity * deltaTime;

        rotation += torque * deltaTime;

        opacity = sf::base::clamp(opacity - opacityDecay * deltaTime, 0.f, 1.f);
    }

    void applyToSprite(sf::Sprite& sprite, const sf::FloatRect* txRects) const
    {
        sprite.position    = position;
        sprite.rotation    = sf::radians(rotation);
        sprite.scale       = {scale, scale};
        sprite.color.a     = static_cast<sf::base::U8>(opacity * 255.0f);
        sprite.textureRect = txRects[static_cast<sf::base::U8>(type)];
    }
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] Particle makeParticle(
    const float        x,
    const float        y,
    const ParticleType particleType,
    const float        scaleMult,
    const float        speedMult)
{
    return {.position      = {x, y},
            .velocity      = getRndVector2f({-0.75f, -0.75f}, {0.75f, 0.75f}) * speedMult,
            .scale         = getRndFloat(0.08f, 0.27f) * scaleMult,
            .accelerationY = 0.002f,
            .opacity       = 1.f,
            .opacityDecay  = getRndFloat(0.00025f, 0.0015f),
            .rotation      = getRndFloat(0.f, sf::base::tau),
            .torque        = getRndFloat(-0.002f, 0.002f),
            .type          = particleType};
}

////////////////////////////////////////////////////////////
struct TextParticle
{
    char buffer[16];

    sf::Vector2f position;
    sf::Vector2f velocity;

    float scale;
    float accelerationY;

    float opacity;
    float opacityDecay;

    float rotation;
    float torque;

    void update(const float deltaTime)
    {
        velocity.y += accelerationY * deltaTime;
        position += velocity * deltaTime;

        rotation += torque * deltaTime;

        opacity = sf::base::clamp(opacity - opacityDecay * deltaTime, 0.f, 1.f);
    }

    void applyToText(sf::Text& text) const
    {
        text.setString(buffer); // TODO P1: should find a way to assign directly to text buffer

        text.position = position;
        text.rotation = sf::radians(rotation);
        text.scale    = {scale, scale};
        text.origin   = text.getLocalBounds().size / 2.f;

        const auto alpha = static_cast<sf::base::U8>(opacity * 255.f);

        text.setFillColor(text.getFillColor().withAlpha(alpha));
        text.setOutlineColor(text.getOutlineColor().withAlpha(alpha));
    }
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] TextParticle makeTextParticle(const float x, const float y, const int combo)
{
    return {
        .buffer        = {},
        .position      = {x, y - 10.f},
        .velocity      = getRndVector2f({-0.1f, -1.65f}, {0.1f, -1.35f}) * 0.5f,
        .scale         = 1.f + 0.1f * static_cast<float>(combo + 1) / 2.f,
        .accelerationY = 0.0045f,
        .opacity       = 1.f,
        .opacityDecay  = 0.0020f,
        .rotation      = 0.f,
        .torque        = getRndFloat(-0.002f, 0.002f),
    };
}

////////////////////////////////////////////////////////////
enum class CatType : sf::base::U8
{
    Normal    = 0u,
    Unicorn   = 1u,
    Devil     = 2u,
    Witch     = 3u,
    Astromeow = 4u,
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

    sf::Text        textName;
    sf::Text        textStatus;
    TextShakeEffect textStatusShakeEffect{};

    float beingDragged = 0.f;
    int   hits         = 0;

    bool update(const float maxCooldown, const float deltaTime)
    {
        textStatusShakeEffect.update(deltaTime);
        textStatusShakeEffect.applyToText(textStatus);

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

////////////////////////////////////////////////////////////
bool handleBubbleCollision(Bubble& iBubble, Bubble& jBubble)
{
    const auto iRadius = iBubble.radius;
    const auto jRadius = jBubble.radius;

    const sf::Vector2f diff        = jBubble.position - iBubble.position;
    const float        squaredDiff = diff.lengthSquared();

    const sf::Vector2f radii{iRadius, jRadius};
    const float        squaredRadiiSum = radii.lengthSquared();

    if (squaredDiff >= squaredRadiiSum)
        return false;

    // Calculate the overlap between the bubbles
    const float distance = sf::base::sqrt(squaredDiff);    // Distance between centers
    const float overlap  = (iRadius + jRadius) - distance; // Amount of overlap

    // Define a "softness" factor to control how quickly the overlap is resolved
    const float softnessFactor = 0.025f; // Adjust this value to control the overlap solver (0.1 = 10% per frame)

    // Calculate the displacement needed to resolve the overlap
    const sf::Vector2f displacement = diff.normalized() * overlap * softnessFactor;

    // Move the bubbles apart based on their masses (heavier bubbles move less)
    const float m1        = iRadius * iRadius; // Mass of bubble i (quadratic scaling)
    const float m2        = jRadius * jRadius; // Mass of bubble j (quadratic scaling)
    const float totalMass = m1 + m2;

    iBubble.position -= displacement * (m2 / totalMass); // Move bubble i
    jBubble.position += displacement * (m1 / totalMass); // Move bubble j

    return true;
};

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
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
         .title           = "BubbleByte",
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

    /* --- Music */
    auto musicBGM = sf::Music::openFromFile("resources/hibiscus.mp3").value();

    /* --- Sound buffers */
    const auto soundBufferPop       = sf::SoundBuffer::loadFromFile("resources/pop.wav").value();
    const auto soundBufferShine     = sf::SoundBuffer::loadFromFile("resources/shine.ogg").value();
    const auto soundBufferClick     = sf::SoundBuffer::loadFromFile("resources/click2.wav").value();
    const auto soundBufferByteMeow  = sf::SoundBuffer::loadFromFile("resources/bytemeow.ogg").value();
    const auto soundBufferGrab      = sf::SoundBuffer::loadFromFile("resources/grab.ogg").value();
    const auto soundBufferDrop      = sf::SoundBuffer::loadFromFile("resources/drop.ogg").value();
    const auto soundBufferScratch   = sf::SoundBuffer::loadFromFile("resources/scratch.ogg").value();
    const auto soundBufferBuy       = sf::SoundBuffer::loadFromFile("resources/buy.ogg").value();
    const auto soundBufferExplosion = sf::SoundBuffer::loadFromFile("resources/explosion.ogg").value();
    const auto soundBufferMakeBomb  = sf::SoundBuffer::loadFromFile("resources/makebomb.ogg").value();
    const auto soundBufferHex       = sf::SoundBuffer::loadFromFile("resources/hex.ogg").value();

    /* --- Sounds */
    sf::Sound soundPop(soundBufferPop);
    sf::Sound soundShine(soundBufferShine);
    sf::Sound soundClick(soundBufferClick);
    sf::Sound soundByteMeow(soundBufferByteMeow);
    sf::Sound soundGrab(soundBufferGrab); // TODO
    sf::Sound soundDrop(soundBufferDrop); // TODO
    sf::Sound soundScratch(soundBufferScratch);
    soundScratch.setVolume(35.f);
    sf::Sound soundBuy(soundBufferBuy);
    sf::Sound soundExplosion(soundBufferExplosion);
    soundExplosion.setVolume(75.f);
    sf::Sound soundMakeBomb(soundBufferMakeBomb);
    sf::Sound soundHex(soundBufferHex);

    /* --- Images */
    const auto imgBubble128    = sf::Image::loadFromFile("resources/bubble2.png").value();
    const auto imgBubbleStar   = sf::Image::loadFromFile("resources/bubble3.png").value();
    const auto imgCat          = sf::Image::loadFromFile("resources/cat.png").value();
    const auto imgUniCat       = sf::Image::loadFromFile("resources/unicat.png").value();
    const auto imgDevilCat     = sf::Image::loadFromFile("resources/devilcat.png").value();
    const auto imgCatPaw       = sf::Image::loadFromFile("resources/catpaw.png").value();
    const auto imgUniCatPaw    = sf::Image::loadFromFile("resources/unicatpaw.png").value();
    const auto imgDevilCatPaw  = sf::Image::loadFromFile("resources/devilcatpaw.png").value();
    const auto imgParticle     = sf::Image::loadFromFile("resources/particle.png").value();
    const auto imgStarParticle = sf::Image::loadFromFile("resources/starparticle.png").value();
    const auto imgFireParticle = sf::Image::loadFromFile("resources/fireparticle.png").value();
    const auto imgHexParticle  = sf::Image::loadFromFile("resources/hexparticle.png").value();
    const auto imgWitchCat     = sf::Image::loadFromFile("resources/witchcat.png").value();
    const auto imgWitchCatPaw  = sf::Image::loadFromFile("resources/witchcatpaw.png").value();
    const auto imgAstromeowCat = sf::Image::loadFromFile("resources/astromeow.png").value();
    const auto imgBomb         = sf::Image::loadFromFile("resources/bomb.png").value();

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
    const auto txrDevilCatPaw  = textureAtlas.add(imgDevilCatPaw).value();
    const auto txrParticle     = textureAtlas.add(imgParticle).value();
    const auto txrStarParticle = textureAtlas.add(imgStarParticle).value();
    const auto txrFireParticle = textureAtlas.add(imgFireParticle).value();
    const auto txrHexParticle  = textureAtlas.add(imgHexParticle).value();
    const auto txrWitchCat     = textureAtlas.add(imgWitchCat).value();
    const auto txrWitchCatPaw  = textureAtlas.add(imgWitchCatPaw).value();
    const auto txrAstromeowCat = textureAtlas.add(imgAstromeowCat).value();
    const auto txrBomb         = textureAtlas.add(imgBomb).value();

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

    TextShakeEffect moneyTextShakeEffect;
    TextShakeEffect comboTextShakeEffect;

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
    const auto costFunction = [](const float baseCost, const float nOwned, const float growthFactor)
    { return baseCost * std::pow(growthFactor, nOwned); };

    bool comboPurchased             = false;
    bool longerComboPurchased       = false;
    bool bubbleTargetCountPurchased = false;

    //
    //
    // Scaling values
    SizeT rewardPerType[3]{
        1u,  // Normal
        25u, // Star
        1u,  // Bomb
    };

    float catCooldownPerType[5]{
        1000.f, // Normal
        2000.f, // Unicorn
        5000.f, // Devil
        1000.f, // Witch
        1000.f  // Astromeow
    };

    float catRangePerType[5]{
        96.f,  // Normal
        64.f,  // Unicorn
        48.f,  // Devil
        256.f, // Witch
        96.f   // Astromeow
    };

    //
    //
    // Cat names
    std::array catNames{"Gorgonzola", "Provolino",  "Pistacchietto", "Ricottina",  "Mozzarellina",  "Tiramisu",
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
    SizeT               bubbleTargetCount = 250;

    std::vector<Cat> cats;

    SizeT money = 0;

    int   combo           = 0;
    float comboTimer      = 0.f;
    float comboTimerStart = 600.f;

    const auto makeRandomBubble = [&]
    {
        const float scaleFactor = getRndFloat(0.07f, 0.17f) * 0.61f;
        const float radius      = txrBubble128.size.x / 2.f * scaleFactor;

        return Bubble{.position = getRndVector2f(boundaries),
                      .velocity = getRndVector2f({-0.1f, -0.1f}, {0.1f, 0.1f}),
                      .scale    = scaleFactor,
                      .radius   = radius,
                      .rotation = 0.f,
                      .type     = BubbleType::Normal};
    };


    sf::Clock            fpsClock;
    sf::Clock            deltaClock;
    sf::CPUDrawableBatch cpuDrawableBatch;

    sf::base::Optional<sf::Vector2f> dragPosition;
    float                            scroll = 0.f;

    sf::base::Optional<sf::Vector2f> catDragPosition;

    std::vector<sf::base::Optional<sf::Vector2f>> fingerPositions;
    fingerPositions.resize(10);

    soundByteMeow.play(playbackDevice);

    musicBGM.setLooping(true);
    musicBGM.play(playbackDevice);
    musicBGM.setVolume(75.f);

    const auto addReward = [&](const SizeT reward)
    {
        money += reward;
        moneyTextShakeEffect.bump(1.f + static_cast<float>(combo) * 0.1f);
    };

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
                fingerPositions[e->finger].emplace(e->position.toVector2f());
            }
            else if (const auto* e = event->getIf<sf::Event::TouchEnded>())
            {
                fingerPositions[e->finger].reset();
            }
            else if (const auto* e = event->getIf<sf::Event::TouchMoved>())
            {
                fingerPositions[e->finger].emplace(e->position.toVector2f());
            }
            else if (const auto* e = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (e->button == sf::Mouse::Button::Left)
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
                    clickPosition.reset();

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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F4))
        {
            comboPurchased             = true;
            longerComboPurchased       = true;
            bubbleTargetCountPurchased = true;

            money = 1'000'000'000u;
        }

        const auto firstTwoFingersPositions = [&]
        {
            std::pair<sf::base::Optional<sf::Vector2f>, sf::base::Optional<sf::Vector2f>> result;

            for (const auto& fingerPosition : fingerPositions)
            {
                if (fingerPosition.hasValue())
                {
                    if (!result.first.hasValue())
                        result.first.emplace(*fingerPosition);
                    else if (!result.second.hasValue())
                        result.second.emplace(*fingerPosition);
                }
            }

            return result;
        }();

        const bool twoFingersDown = firstTwoFingersPositions.first.hasValue() && firstTwoFingersPositions.second.hasValue();

        if (twoFingersDown && !dragPosition.hasValue())
        {
            dragPosition.emplace(*firstTwoFingersPositions.first);
            dragPosition->x += scroll;
        }

        if (twoFingersDown && dragPosition.hasValue())
        {
            const auto v0 = *firstTwoFingersPositions.first;
            const auto v1 = *firstTwoFingersPositions.second;

            scroll = dragPosition->x - static_cast<float>((v0.x + v1.x) / 2.f);
        }

        const bool allFingersNotDown = std::all_of(fingerPositions.begin(),
                                                   fingerPositions.end(),
                                                   [](const auto& fingerPosition) { return !fingerPosition.hasValue(); });

        if (dragPosition.hasValue() && allFingersNotDown && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
        {
            dragPosition.reset();
        }

        scroll = sf::base::clamp(scroll, 0.f, (boundaries.x - resolution.x) / 2.f);

        sf::View gameView{.center = {resolution.x / 2.f + scroll * 2.f, resolution.y / 2.f}, .size = resolution};
        gameView.center.x = sf::base::clamp(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f);

        const auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);

        for (SizeT i = bubbles.size(); i < bubbleTargetCount; ++i)
            bubbles.emplace_back(makeRandomBubble());

        //
        //
        // Update spatial partitioning
        cellStartIndices.clear();
        cellStartIndices.resize(nCellsX * nCellsY + 1, 0); // +1 for prefix sum

        const auto computeGridRange = [&](const auto& bubble)
        {
            const auto minX = bubble.position.x - bubble.radius;
            const auto minY = bubble.position.y - bubble.radius;
            const auto maxX = bubble.position.x + bubble.radius;
            const auto maxY = bubble.position.y + bubble.radius;

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

        auto popBubble = [&](auto self, BubbleType bubbleType, SizeT reward, int combo, float x, float y) -> void
        {
            auto& tp = textParticles.emplace_back(makeTextParticle(x, y, combo));
            std::snprintf(tp.buffer, sizeof(tp.buffer), "+$%zu", reward);

            soundPop.play(playbackDevice);
            soundPop.setPitch(remap(static_cast<float>(combo), 1, 10, 1.f, 2.f));

            for (int i = 0; i < 32; ++i)
                particles.emplace_back(makeParticle(x, y, ParticleType::Bubble, 0.5f, 0.5f));

            for (int i = 0; i < 8; ++i)
                particles.emplace_back(makeParticle(x, y, ParticleType::Bubble, 1.2f, 0.25f));

            if (bubbleType == BubbleType::Star)
                for (int i = 0; i < 16; ++i)
                    particles.emplace_back(makeParticle(x, y, ParticleType::Star, 0.25f, 0.35f));

            if (bubbleType == BubbleType::Bomb)
            {
                soundExplosion.play(playbackDevice);

                for (int i = 0; i < 32; ++i)
                    particles.emplace_back(makeParticle(x, y, ParticleType::Fire, 3.f, 1.f));

                const int cellX = static_cast<int>(x / gridSize);
                const int cellY = static_cast<int>(y / gridSize);

                for (int ox = -4; ox <= 4; ++ox)
                    for (int oy = -4; oy <= 4; ++oy)
                    {
                        const auto cellIdx = convert2DTo1D( //
                            static_cast<SizeT>(sf::base::clamp(cellX + ox, 0, static_cast<int>(nCellsX) - 1)),
                            static_cast<SizeT>(sf::base::clamp(cellY + oy, 0, static_cast<int>(nCellsY) - 1)),
                            nCellsX);

                        const SizeT start = cellStartIndices[cellIdx];
                        const SizeT end   = cellStartIndices[cellIdx + 1];

                        // Iterate over all bubbles in this cell
                        for (SizeT i = start; i < end; ++i)
                        {
                            const SizeT bubbleIdx = bubbleIndices[i];
                            auto&       bubble    = bubbles[bubbleIdx];

                            if (bubble.type != BubbleType::Bomb)
                            {
                                const SizeT newReward = rewardPerType[static_cast<int>(bubble.type)] * 10u;
                                self(self, bubble.type, newReward, 1, bubble.position.x, bubble.position.y);
                                addReward(newReward);
                                bubble = makeRandomBubble();
                            }
                        }
                    }
            }
        };

        for (auto& bubble : bubbles)
        {
            if (bubble.type == BubbleType::Bomb)
                bubble.rotation += deltaTimeMs * 0.01f;

            auto& pos = bubble.position;

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

                if (comboPurchased)
                {
                    if (combo == 0)
                    {
                        combo      = 1;
                        comboTimer = comboTimerStart;
                    }
                    else
                    {
                        ++combo;
                        comboTimer += 150.f - static_cast<float>(combo) * 5.f;

                        comboTextShakeEffect.bump(1.f + static_cast<float>(combo) * 0.2f);
                    }
                }
                else
                {
                    combo = 1;
                }

                const auto reward = rewardPerType[static_cast<int>(bubble.type)] * static_cast<SizeT>(combo);
                popBubble(popBubble, bubble.type, reward, combo, x, y);
                addReward(reward);
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

            const bool onlyOneFingerPressed = std::count_if(fingerPositions.begin(),
                                                            fingerPositions.end(),
                                                            [](const auto& pos) { return pos.hasValue(); }) == 1;

            if (onlyOneFingerPressed)
                for (const auto& fingerPos : fingerPositions)
                    if (fingerPos.hasValue() && handleClick(*fingerPos))
                        break;
        }

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
                        handleBubbleCollision(bubbles[bubbleA], bubbles[bubbleB]);
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

            if (fingerPositions[0].hasValue())
            {
                const auto fingerPos = window.mapPixelToCoords(fingerPositions[0]->toVector2i(), gameView);

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

                if (fingerPositions[0].hasValue())
                {
                    const auto fingerPos = window.mapPixelToCoords(fingerPositions[0]->toVector2i(), gameView);
                    cat.position         = fingerPos;
                }
                else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || catDragPosition.hasValue())
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
                cat.textStatus.setString(std::to_string(cat.hits) + " IEDs");
            else if (cat.type == CatType::Witch)
                cat.textStatus.setString(std::to_string(cat.hits) + " hexes");

            const auto maxCooldown = catCooldownPerType[static_cast<int>(cat.type)];
            const auto range       = catRangePerType[static_cast<int>(cat.type)];

            auto diff = cat.pawSprite.position - cat.sprite.position - sf::Vector2f{-20.f, 20.f};
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
                const auto [cx, cy] = cat.position + cat.rangeOffset;

                if (cat.type == CatType::Witch)
                {
                    int  witchHits = 0;
                    bool pawSet    = false;

                    for (auto& otherCat : cats)
                    {
                        if (otherCat.type == CatType::Witch)
                            continue;

                        if ((otherCat.position - cat.position).length() > range)
                            continue;

                        otherCat.cooldown = catCooldownPerType[static_cast<int>(otherCat.type)];
                        ++witchHits;

                        if (!pawSet && getRndFloat(0.f, 100.f) > 50.f)
                        {
                            pawSet = true;

                            cat.pawSprite.position = {otherCat.position.x, otherCat.position.y};
                            cat.pawSprite.color.a  = 255;
                            cat.pawSprite.rotation = (otherCat.position - cat.position).angle() + sf::degrees(45);
                        }

                        for (int i = 0; i < 8; ++i)
                            particles.emplace_back(
                                makeParticle(otherCat.position.x, otherCat.position.y, ParticleType::Hex, 0.5f, 0.35f));
                    }

                    if (witchHits > 0)
                    {
                        soundHex.play(playbackDevice);

                        cat.textStatusShakeEffect.bump(1.5f);
                        cat.hits += witchHits;
                    }

                    cat.cooldown = 0.f;
                    continue;
                }

                for (auto& bubble : bubbles)
                {
                    if (cat.type == CatType::Unicorn && bubble.type != BubbleType::Normal)
                        continue;

                    const auto [x, y] = bubble.position;

                    if ((x - cx) * (x - cx) + (y - cy) * (y - cy) < range * range)
                    {
                        cat.pawSprite.position = {x, y};
                        cat.pawSprite.color.a  = 255;
                        cat.pawSprite.rotation = (bubble.position - cat.position).angle() + sf::degrees(45);

                        if (cat.type == CatType::Unicorn)
                        {
                            bubble.type       = BubbleType::Star;
                            bubble.velocity.y = getRndFloat(-0.1f, -0.05f);
                            soundShine.play(playbackDevice);

                            for (int i = 0; i < 4; ++i)
                                particles.emplace_back(makeParticle(x, y, ParticleType::Star, 0.25f, 0.35f));

                            cat.textStatusShakeEffect.bump(1.5f);
                            ++cat.hits;
                        }
                        else if (cat.type == CatType::Normal)
                        {
                            const auto reward = rewardPerType[static_cast<int>(bubble.type)];

                            addReward(reward);
                            popBubble(popBubble, bubble.type, reward, 1, x, y);
                            bubble = makeRandomBubble();

                            cat.textStatusShakeEffect.bump(1.5f);
                            ++cat.hits;
                        }
                        else if (cat.type == CatType::Devil)
                        {
                            bubble.type = BubbleType::Bomb;
                            bubble.velocity.y += getRndFloat(0.1f, 0.2f);
                            soundMakeBomb.play(playbackDevice);

                            for (int i = 0; i < 8; ++i)
                                particles.emplace_back(makeParticle(x, y, ParticleType::Fire, 1.25f, 0.35f));

                            cat.textStatusShakeEffect.bump(1.5f);
                            ++cat.hits;
                        }

                        cat.cooldown = 0.f;
                        break;
                    }
                }
            }
        }

        for (auto& textParticle : textParticles)
            textParticle.update(deltaTimeMs);

        std::erase_if(textParticles, [](const auto& textParticle) { return textParticle.opacity <= 0; });

        for (auto& particle : particles)
            particle.update(deltaTimeMs);

        std::erase_if(particles, [](const auto& particle) { return particle.opacity <= 0.f; });

        if (comboTimer > 0.f)
        {
            comboTimer -= deltaTimeMs;

            if (comboTimer <= 0.f)
            {
                if (combo > 2)
                    soundScratch.play(playbackDevice);

                combo      = 0;
                comboTimer = 0.f;
            }
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

            const auto cost = static_cast<SizeT>(costFunction(baseCost, count, growthFactor));
            std::sprintf(buffer, "$%zu##%s", cost, label);

            ImGui::BeginDisabled(money < cost);
            ImGui::Text("%s", label);
            ImGui::SameLine();

            const float rightWidgetPosX = 250.f - ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + rightWidgetPosX);

            ImGui::Button(buffer);
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (!imGuiContext.wasLastInputTouch())
                {
                    catDragPosition.emplace(mousePos);
                }

                soundClick.play(playbackDevice);
                soundBuy.play(playbackDevice);

                result = true;
                money -= cost;
            }

            ImGui::SetWindowFontScale(0.5f);
            ImGui::SameLine();
            ImGui::Text("%s", labelBuffer);
            ImGui::SetWindowFontScale(1.f);

            ImGui::EndDisabled();

            return result;
        };

        if (!comboPurchased)
        {
            std::sprintf(labelBuffer, "");
            if (makePurchasableButton("Combo", 10.f, 1.0f, 1.f))
            {
                comboPurchased = true;
                combo          = 0;
            }
        }

        if (comboPurchased)
        {
            std::sprintf(labelBuffer, "%.2fs", static_cast<double>(comboTimerStart / 1000.f));
            if (makePurchasableButton("Longer combo", 2.f, 1.5f, static_cast<float>(comboTimerStart) / 100.f))
            {
                longerComboPurchased = true;
                comboTimerStart *= 1.15f;
            }
        }

        const auto countCatsByType = [&](CatType type)
        {
            return static_cast<int>(
                std::count_if(cats.begin(), cats.end(), [type](const auto& cat) { return cat.type == type; }));
        };

        const auto nCatNormal  = countCatsByType(CatType::Normal);
        const auto nCatUnicorn = countCatsByType(CatType::Unicorn);
        const auto nCatDevil   = countCatsByType(CatType::Devil);
        const auto nCatWitch   = countCatsByType(CatType::Witch);

        if (nCatNormal > 0 && longerComboPurchased)
        {
            std::sprintf(labelBuffer, "%zu bubbles", bubbleTargetCount);
            if (makePurchasableButton("More bubbles", 25.f, 1.70f, static_cast<float>(bubbleTargetCount) / 100.f))
            {
                bubbleTargetCountPurchased = true;
                bubbleTargetCount          = static_cast<SizeT>(static_cast<float>(bubbleTargetCount) * 1.25f);
            }
        }

        if (bubbleTargetCountPurchased && nCatUnicorn > 2)
        {
            auto& rewardNormal = rewardPerType[static_cast<int>(BubbleType::Normal)];
            auto& rewardStar   = rewardPerType[static_cast<int>(BubbleType::Star)];

            std::sprintf(labelBuffer, "$%zu | $%zu", rewardNormal, rewardStar);
            if (makePurchasableButton("Bubble value", 500.f, 1.75f, static_cast<float>(rewardNormal)))
            {
                rewardNormal += 1;
                rewardStar += 5;
            }
        }

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
                       .textName     = makeCatNameText(getNextCatName()),
                       .textStatus   = makeCatStatusText(),
                       .beingDragged = 1500.f};
        };

        const auto makeCatModifiers =
            [&](const char* labelCooldown, const char* labelRange, const CatType catType, const float initialCooldown)
        {
            auto& maxCooldown = catCooldownPerType[static_cast<int>(catType)];
            auto& range       = catRangePerType[static_cast<int>(catType)];

            std::sprintf(labelBuffer, "%.2fs", static_cast<double>(maxCooldown / 1000.f));
            if (makePurchasableButton(labelCooldown, 20.f, 1.25f, 13.f + (initialCooldown - maxCooldown) / 35.f))
                maxCooldown *= 0.95f;

            std::sprintf(labelBuffer, "%.2fpx", static_cast<double>(range));
            if (makePurchasableButton(labelRange, 18.f, 1.75f, range / 15.f))
                range *= 1.05f;
        };

        const auto resetCatDrag = [&]
        {
            for (auto& cat : cats)
                cat.beingDragged = 0.f;

             catDragPosition.reset();
        };

        if (allFingersNotDown && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            resetCatDrag();

        if (comboPurchased)
        {
            std::sprintf(labelBuffer, "%d cats", nCatNormal);
            if (makePurchasableButton("Cat", 35, 1.5f, static_cast<float>(nCatNormal)))
            {
                resetCatDrag();
                cats.emplace_back(makeCat(CatType::Normal, {0.f, 0.f}, txrCat, txrCatPaw));
            }
        }

        if (bubbleTargetCountPurchased && nCatNormal > 0 && nCatUnicorn > 0)
            makeCatModifiers("Cat cooldown", "Cat range", CatType::Normal, 1000.f);

        // UNICORN CAT
        if (bubbleTargetCountPurchased && nCatNormal > 2)
        {
            std::sprintf(labelBuffer, "%d unicats", nCatUnicorn);
            if (makePurchasableButton("Unicat", 250, 1.5f, static_cast<float>(nCatUnicorn)))
            {
                resetCatDrag();
                cats.emplace_back(makeCat(CatType::Unicorn, {0.f, -100.f}, txrUniCat, txrUniCatPaw));
            }

            if (nCatUnicorn > 1 && nCatDevil > 0)
                makeCatModifiers("Unicat cooldown", "Unicat range", CatType::Unicorn, 2000.f);
        }

        // DEVIL CAT
        if (nCatUnicorn > 2)
        {
            std::sprintf(labelBuffer, "%d devilcats", nCatDevil);
            if (makePurchasableButton("Devilcat", 1000.f, 1.5f, static_cast<float>(nCatDevil)))
            {
                resetCatDrag();
                cats.emplace_back(makeCat(CatType::Devil, {0.f, 100.f}, txrDevilCat, txrDevilCatPaw));
            }

            if (nCatDevil > 1 && nCatWitch > 0)
                makeCatModifiers("Devilcat cooldown", "Devilcat range", CatType::Devil, 5000.f);
        }

        // WITCH CAT
        if (nCatDevil > 2)
        {
            std::sprintf(labelBuffer, "%d witch cats", nCatWitch);
            if (makePurchasableButton("Witch cat", 10000.f, 1.5f, static_cast<float>(nCatWitch)))
            {
                resetCatDrag();
                cats.emplace_back(makeCat(CatType::Witch, {0.f, 0.f}, txrWitchCat, txrWitchCatPaw));
            }

            if (nCatWitch > 1)
                makeCatModifiers("Witch cat cooldown", "Witch cat range", CatType::Witch, 1000.f);
        }

        ImGui::End();
        ImGui::PopFont();

        window.clear(sf::Color{157, 171, 191});

        const float progress = remap(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f, 0.f, 100.f);
        window.setView(gameView);

        window.draw(txBackground);

        cpuDrawableBatch.clear();

        // ---
        sf::CircleShape catRadiusCircle{{
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::Transparent,
            .outlineThickness   = 1.f,
            .pointCount         = 32,
        }};

        for (auto& cat : cats)
        {
            cpuDrawableBatch.add(cat.sprite);
            cpuDrawableBatch.add(cat.pawSprite);

            const auto maxCooldown = catCooldownPerType[static_cast<int>(cat.type)];
            const auto range       = catRangePerType[static_cast<int>(cat.type)];

            constexpr sf::Color colorsByType[5]{
                sf::Color::Blue,   // Cat
                sf::Color::Purple, // Unicorn
                sf::Color::Red,    // Devil
                sf::Color::Green,  // Witch
                sf::Color::Red,    // Astromeow
            };

            catRadiusCircle.position = cat.position + cat.rangeOffset;
            catRadiusCircle.origin   = {range, range};
            catRadiusCircle.setRadius(range);
            catRadiusCircle.setOutlineColor(colorsByType[static_cast<int>(cat.type)].withAlpha(
                cat.cooldown < 0.f ? static_cast<sf::base::U8>(0u)
                                   : static_cast<sf::base::U8>(cat.cooldown / maxCooldown * 128.f)));

            cpuDrawableBatch.add(catRadiusCircle);
            cpuDrawableBatch.add(cat.textName);   // TODO: move text object outside
            cpuDrawableBatch.add(cat.textStatus); // TODO: move text object outside
        };
        // ---

        // ---
        const sf::FloatRect bubbleRects[3]{txrBubble128, txrBubbleStar, txrBomb};
        sf::Sprite          bubbleSprite;

        for (const auto& bubble : bubbles)
        {
            bubble.applyToSprite(bubbleSprite);
            bubbleSprite.textureRect = bubbleRects[static_cast<int>(bubble.type)];
            bubbleSprite.origin      = bubbleSprite.textureRect.size / 2.f;

            cpuDrawableBatch.add(bubbleSprite);
        }
        // ---

        // ---
        const sf::FloatRect particleRects[4]{txrParticle, txrStarParticle, txrFireParticle, txrHexParticle};
        sf::Sprite          particleSprite;

        for (const auto& particle : particles)
        {
            particle.applyToSprite(particleSprite, particleRects);
            cpuDrawableBatch.add(particleSprite);
        }
        // ---

        // ---
        sf::Text textParticleText{fontDailyBubble,
                                  {.characterSize    = 16u,
                                   .fillColor        = sf::Color::White,
                                   .outlineColor     = colorBlueOutline,
                                   .outlineThickness = 1.0f}};

        for (const auto& textParticle : textParticles)
        {
            textParticle.applyToText(textParticleText);
            cpuDrawableBatch.add(textParticleText);
        }
        // ---

        window.draw(cpuDrawableBatch, {.texture = &textureAtlas.getTexture()});

        window.setView({.center = {resolution.x / 2.f, resolution.y / 2.f}, .size = resolution});

        moneyText.setString("$" + std::to_string(money));
        moneyText.scale  = {1.f, 1.f};
        moneyText.origin = moneyText.getLocalBounds().size / 2.f;

        moneyText.setTopLeft({15.f, 70.f});
        moneyTextShakeEffect.update(deltaTimeMs);
        moneyTextShakeEffect.applyToText(moneyText);

        window.draw(moneyText);

        if (comboPurchased)
        {
            comboText.setString("x" + std::to_string(combo + 1));

            comboTextShakeEffect.update(deltaTimeMs);
            comboTextShakeEffect.applyToText(comboText);

            window.draw(comboText);
        }

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


        window.draw(logoSprite, txLogo);

        window.display();

        //
        // Debug stuff
        window.setTitle("FPS: " + std::to_string(1.f / fpsClock.getElapsedTime().asSeconds()));
    }
}
