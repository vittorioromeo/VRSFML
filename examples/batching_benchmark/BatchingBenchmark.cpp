#include "ExampleUtils/RNGFast.hpp"

#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/DrawableBatch.hpp"
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/Image.hpp"
#include "Zancle/Graphics/RenderStates.hpp"
#include "Zancle/Graphics/RenderTexture.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/Sprite.hpp"
#include "Zancle/Graphics/Text.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/TextureAtlas.hpp"
#include "Zancle/Graphics/View.hpp" // IWYU pragma: keep

#include "Zancle/Window/Event.hpp" // IWYU pragma: keep

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/Clock.hpp"
#include "Zancle/System/IO.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Constants.hpp"
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    //
    //
    // Set up random generator
    RNGFast    rng(/* seed */ 1234);
    const auto getRndFloat = [&](float min, float max) { return rng.getF(min, max); };

    //
    //
    // Set up graphics context
    auto graphicsContext = za::GraphicsContext::create().value();

    //
    //
    // Set up window
    constexpr za::Vec2f windowSize{1680.f, 1050.f};

    auto window = za::RenderWindow::create(
                      {
                          .size      = windowSize.toVec2u(),
                          .title     = "Vittorio's SFML fork: batching benchmark",
                          .resizable = false,
                          .vsync     = false,
                      })
                      .value();

    //
    //
    // Set up texture atlas
    za::TextureAtlas textureAtlas{za::Texture::create({1024u, 1024u}, {.smooth = true}).value()};

    //
    //
    // Load fonts
    const auto fontTuffy        = za::Font::openFromFile("resources/tuffy.ttf", &textureAtlas).value();
    const auto fontMouldyCheese = za::Font::openFromFile("resources/mouldycheese.ttf", &textureAtlas).value();

    //
    //
    // Load images
    const auto imgElephant = za::Image::loadFromFile("resources/elephant.png").value();
    const auto imgGiraffe  = za::Image::loadFromFile("resources/giraffe.png").value();
    const auto imgMonkey   = za::Image::loadFromFile("resources/monkey.png").value();
    const auto imgPig      = za::Image::loadFromFile("resources/pig.png").value();
    const auto imgRabbit   = za::Image::loadFromFile("resources/rabbit.png").value();
    const auto imgSnake    = za::Image::loadFromFile("resources/snake.png").value();

    //
    //
    // Add images to texture atlas
    const za::Rect2f spriteTextureRects[]{textureAtlas.add(imgElephant).value(),
                                          textureAtlas.add(imgGiraffe).value(),
                                          textureAtlas.add(imgMonkey).value(),
                                          textureAtlas.add(imgPig).value(),
                                          textureAtlas.add(imgRabbit).value(),
                                          textureAtlas.add(imgSnake).value()};

    //
    //
    // Simulation stuff
    struct Entity
    {
        za::Text   text;
        za::Sprite sprite;
        float      torque;
    };

    zb::Vector<Entity> entities;

    const auto populateEntities = [&](const zb::SizeT n)
    {
        entities.clear();
        entities.reserve(n);

        for (zb::SizeT i = 0u; i < n; ++i)
        {
            const zb::SizeT type        = i % 6u;
            const za::Rect2f&     textureRect = spriteTextureRects[type];

            auto& [text, sprite, torque] = entities.emplaceBack(za::Text{i % 2u == 0u ? fontTuffy : fontMouldyCheese,
                                                                         {.string           = "abcdefABCDEF",
                                                                          .characterSize    = 30u,
                                                                          .fillColor        = za::Color::Black,
                                                                          .outlineColor     = za::Color::White,
                                                                          .outlineThickness = 5.f}},
                                                                za::Sprite{.textureRect = textureRect},
                                                                getRndFloat(-0.05f, 0.05f));

            sprite.origin   = textureRect.size / 2.f;
            sprite.rotation = za::radians(getRndFloat(0.f, zb::tau));

            const float scaleFactor = getRndFloat(0.08f, 0.17f);
            sprite.scale            = {scaleFactor, scaleFactor};
            text.scale              = sprite.scale * 3.5f;

            sprite.position = {getRndFloat(0.f, windowSize.x), getRndFloat(0.f, windowSize.y)};

            text.origin   = text.getLocalBounds().size / 2.f;
            text.position = sprite.position;
        }
    };

    //
    //
    // Settings
    constexpr bool useBatch    = true;
    constexpr int  numEntities = 50'000;
    int            numFrames   = 240;

//
//
// Set up benchmark
#ifndef ZA_OPENGL_ES
    zb::printLn("OpenGL ES not detected, using persistent GPU batching");
    za::PersistentGPUDrawableBatch drawableBatch;
#else
    zb::printLn("OpenGL ES detected, using CPU storage-backed batching");
    za::CPUDrawableBatch drawableBatch;
#endif
    populateEntities(static_cast<zb::SizeT>(numEntities));

    if (useBatch)
    {
        drawableBatch.position = drawableBatch.origin = windowSize / 2.f;
        drawableBatch.reserveQuads(static_cast<zb::SizeT>(numEntities) * 25u);
    }
    else
    {
        window.reserveAutoBatchQuads(static_cast<zb::SizeT>(numEntities) * 25u);
    }

    const za::Clock clock;
    const auto      startTime = clock.getElapsedTime();

    while (--numFrames > 0)
    {
        window.clear();

        if (useBatch)
        {
            drawableBatch.clear();
            drawableBatch.rotation += za::degrees(2.f);
        }

        while (window.pollEvent())
            ;

        for (const Entity& entity : entities)
        {
            if (useBatch)
            {
                drawableBatch.add(entity.sprite);
                drawableBatch.add(entity.text);
            }
            else
            {
                window.draw(entity.sprite, {.texture = &textureAtlas.getTexture()});
                window.draw(entity.text);
            }
        }

        if (useBatch)
            window.draw(drawableBatch, {.texture = &textureAtlas.getTexture()});

        window.display();
    }

    const auto finalTime = clock.getElapsedTime() - startTime;

    zb::printLn("FINAL TIME: {} ms", finalTime.asMilliseconds());
}
