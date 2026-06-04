////////////////////////////////////////////////////////////
// In-world ImGui surfaces driven by Box2D physics.
//
// - Each "screen" is a rigid body whose face is an interactive ImGui panel.
// - Left-click on a screen to interact with its UI; the mouse is transformed
//   into the body's local frame so widgets work even while the body rotates.
// - Right-click drag to apply a temporary spring force at the grab point.
////////////////////////////////////////////////////////////

#include "ExampleUtils/Scaling.hpp"

#include "Zancle/ImGui/ImGuiContext.hpp"
#include "Zancle/ImGui/IncludeImGui.hpp"

#include "Zancle/Graphics/CircleShapeData.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/RectangleShapeData.hpp"
#include "Zancle/Graphics/RenderTexture.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/Text.hpp"
#include "Zancle/Graphics/TextData.hpp"
#include "Zancle/Graphics/View.hpp"

#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/EventUtils.hpp"
#include "Zancle/Window/Mouse.hpp"

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/Clock.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Time.hpp"
#include "Zancle/System/Vec2.hpp"

#include "ZancleBase/Math/Atan2.hpp"
#include "ZancleBase/Math/Fabs.hpp"
#include "ZancleBase/Math/Sin.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"

#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>


namespace
{
////////////////////////////////////////////////////////////
constexpr float pxPerMeter    = 100.f; // world meters → window pixels
constexpr float texPxPerMeter = 200.f; // body-local meters → render-texture pixels (higher = crisper UI)


////////////////////////////////////////////////////////////
[[nodiscard]] za::Vec2f toSfVec(const b2Vec2 v) noexcept
{
    return {v.x, v.y};
}


////////////////////////////////////////////////////////////
[[nodiscard]] b2Vec2 toB2Vec(const za::Vec2f v) noexcept
{
    return {v.x, v.y};
}


////////////////////////////////////////////////////////////
// Map a window pixel to world-space meters using the active view.
[[nodiscard]] za::Vec2f pixelToWorld(const za::Vec2i pixel, const za::Vec2f windowSize, const za::View& view)
{
    const za::Vec2f normalized = pixel.toVec2f().componentWiseDiv(windowSize);
    return view.center - view.size / 2.f + normalized.componentWiseMul(view.size);
}


////////////////////////////////////////////////////////////
// Map body-local meters to texture pixels (origin at top-left of the texture).
[[nodiscard]] za::Vec2f localToTexture(const za::Vec2f localM, const za::Vec2f halfSizeM, const za::Vec2u texSize)
{
    return {(localM.x + halfSizeM.x) / (2.f * halfSizeM.x) * static_cast<float>(texSize.x),
            (localM.y + halfSizeM.y) / (2.f * halfSizeM.y) * static_cast<float>(texSize.y)};
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool isInsideHalfBox(const za::Vec2f localM, const za::Vec2f halfSizeM)
{
    return zb::fabs(localM.x) <= halfSizeM.x && zb::fabs(localM.y) <= halfSizeM.y;
}


////////////////////////////////////////////////////////////
struct Screen
{
    b2BodyId  bodyId{};
    b2ShapeId shapeId{};

    za::Vec2f baseHalfSizeM; // unscaled half-size used to derive the current shape
    za::Vec2f halfSizeM;     // current (post-scale) half-size in world meters
    za::Vec2u textureSize;   // pixels

    zb::Optional<za::ImGuiContext> imGuiContext;

    // Per-screen UI state
    float     sliderValue{1.f};    // slider drives the box scale
    float     currentScale{1.f};   // last applied scale; resync the shape when these diverge
    float     sliderVelocity{0.f}; // gravity-driven drift along the slider axis
    bool      sliderActive{false}; // true while the user is dragging the slider this frame
    int       counter{0};
    bool      checkBox{false};
    za::Vec2f cursorTexPos; // last known cursor in texture pixels (for the active screen)

    // Visual tint applied to the rendered screen sprite, also editable from inside the screen
    float tintRGB[3]{0.7f, 0.9f, 1.f};
};


////////////////////////////////////////////////////////////
b2BodyId makeStaticBox(b2WorldId world, za::Vec2f centerM, za::Vec2f halfSizeM)
{
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = toB2Vec(centerM);

    const b2BodyId body = b2CreateBody(world, &bd);

    b2ShapeDef sd           = b2DefaultShapeDef();
    sd.material             = b2DefaultSurfaceMaterial();
    sd.material.friction    = 0.6f;
    sd.material.restitution = 0.1f;

    const b2Polygon box = b2MakeBox(halfSizeM.x, halfSizeM.y);
    b2CreatePolygonShape(body, &sd, &box);
    return body;
}


////////////////////////////////////////////////////////////
void initScreen(Screen& s, b2WorldId world, za::Vec2f initialPosM, za::Vec2f halfSizeM, const float tint[3], ImFontAtlas& sharedFontAtlas)
{
    s.baseHalfSizeM = halfSizeM;
    s.halfSizeM     = halfSizeM;
    s.textureSize   = (halfSizeM * 2.f * texPxPerMeter).toVec2u();

    for (int i = 0; i < 3; ++i)
        s.tintRGB[i] = tint[i];

    // All screens share one font atlas (populated by the caller). The first screen drives the
    // atlas (`createOwningAtlas`); the rest are sharers (`createSharingAtlas`). The driver
    // also owns the GL font texture; the others adopt it (saving ~1 MB of GPU memory per
    // sharer).
    s.imGuiContext.emplace(sharedFontAtlas.OwnerContext == nullptr
                               ? za::ImGuiContext::createOwningAtlas(sharedFontAtlas)
                               : za::ImGuiContext::createSharingAtlas(sharedFontAtlas));

    // Scale text and widgets so the in-world UI is readable at the on-screen size
    // (each screen body is drawn at ~half the texture's pixel resolution).
    s.imGuiContext->setCurrent();
    ImGui::GetIO().FontGlobalScale = 2.f;
    ImGui::GetStyle().ScaleAllSizes(2.f);

    b2BodyDef bd      = b2DefaultBodyDef();
    bd.type           = b2_dynamicBody;
    bd.position       = toB2Vec(initialPosM);
    bd.linearDamping  = 0.4f;
    bd.angularDamping = 0.6f;

    s.bodyId = b2CreateBody(world, &bd);

    b2ShapeDef sd           = b2DefaultShapeDef();
    sd.density              = 1.f;
    sd.material             = b2DefaultSurfaceMaterial();
    sd.material.friction    = 0.5f;
    sd.material.restitution = 0.15f;

    const b2Polygon box = b2MakeBox(halfSizeM.x, halfSizeM.y);
    s.shapeId           = b2CreatePolygonShape(s.bodyId, &sd, &box);
}


////////////////////////////////////////////////////////////
// Rebuild a screen's collision shape at a new scale (Box2D polygons aren't resizable in place).
void rescaleScreenShape(Screen& s)
{
    s.halfSizeM = s.baseHalfSizeM * s.sliderValue;

    b2DestroyShape(s.shapeId, /* updateBodyMass */ true);

    b2ShapeDef sd           = b2DefaultShapeDef();
    sd.density              = 1.f;
    sd.material             = b2DefaultSurfaceMaterial();
    sd.material.friction    = 0.5f;
    sd.material.restitution = 0.15f;

    const b2Polygon box = b2MakeBox(s.halfSizeM.x, s.halfSizeM.y);
    s.shapeId           = b2CreatePolygonShape(s.bodyId, &sd, &box);

    s.currentScale = s.sliderValue;
    b2Body_SetAwake(s.bodyId, true);
}


////////////////////////////////////////////////////////////
[[nodiscard]] Screen* findScreenAt(zb::Vector<Screen>& screens, za::Vec2f worldM, za::Vec2f& outLocalM)
{
    // Iterate in reverse so the topmost (later-drawn) screen wins overlapping hits
    for (zb::SizeT i = screens.size(); i-- > 0u;)
    {
        Screen&         s     = screens[i];
        const za::Vec2f local = toSfVec(b2Body_GetLocalPoint(s.bodyId, toB2Vec(worldM)));
        if (isInsideHalfBox(local, s.halfSizeM))
        {
            outLocalM = local;
            return &s;
        }
    }
    return nullptr;
}

} // namespace


////////////////////////////////////////////////////////////
int main()
{
    auto graphicsContext = za::GraphicsContext::create().value();

    constexpr za::Vec2u windowPx{1280u, 720u};

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size      = windowPx,
                          .title     = "ImGui + Box2D: in-world UIs",
                          .resizable = false,
                          .vsync     = true,
                      })
                      .value();

    const za::Vec2f worldSizeM = windowPx.toVec2f() / pxPerMeter;
    za::View        worldView{.center = worldSizeM / 2.f, .size = worldSizeM};

    const auto font = za::Font::openFromFile("resources/tuffy.ttf").value();

    // ----- Box2D world -----

    b2WorldDef worldDef   = b2DefaultWorldDef();
    worldDef.gravity      = {0.f, 9.81f}; // y-down
    const b2WorldId world = b2CreateWorld(&worldDef);

    constexpr float wallHalfThickness = 0.2f;

    // Walls forming a box around the world view
    makeStaticBox(world, {worldSizeM.x / 2.f, -wallHalfThickness}, {worldSizeM.x / 2.f, wallHalfThickness});
    makeStaticBox(world, {worldSizeM.x / 2.f, worldSizeM.y + wallHalfThickness}, {worldSizeM.x / 2.f, wallHalfThickness});
    makeStaticBox(world, {-wallHalfThickness, worldSizeM.y / 2.f}, {wallHalfThickness, worldSizeM.y / 2.f});
    makeStaticBox(world, {worldSizeM.x + wallHalfThickness, worldSizeM.y / 2.f}, {wallHalfThickness, worldSizeM.y / 2.f});

    // ----- Shared ImGui font atlas + shared scratch render target -----
    //
    // All screens share one ImFontAtlas: the first context to render uploads the GL font
    // texture, the others adopt it. Saves ~1 MB of GPU memory per extra screen.
    //
    // All screens share one RenderTexture used as scratch space: each frame, every screen's
    // UI is rendered into it sequentially and then immediately composited onto the main
    // window. Saves the per-screen framebuffer storage (~halfSizeM * 2 * texPxPerMeter
    // squared * 4 bytes per screen).

    // The caller is responsible for populating the shared atlas before constructing any
    // ImGuiContext that references it -- the shared-atlas constructor never adds fonts.
    ImFontAtlas sharedFontAtlas;
    sharedFontAtlas.AddFontDefaultBitmap();

    constexpr za::Vec2f maxScreenHalfSizeM{1.4f, 1.f};
    const za::Vec2u     scratchRTSize = (maxScreenHalfSizeM * 2.f * texPxPerMeter).toVec2u();

    auto scratchRT = za::RenderTexture::create(scratchRTSize, {.smooth = true}).value();

    // ----- Screens -----

    zb::Vector<Screen> screens;
    screens.reserve(3u);

    constexpr float palette[3][3] = {
        {0.6f, 0.85f, 1.f},
        {1.f, 0.8f, 0.6f},
        {0.85f, 1.f, 0.7f},
    };

    const za::Vec2f startPositions[3]  = {{4.f, 2.f}, {7.f, 2.f}, {10.f, 2.f}};
    const za::Vec2f screenHalfSizes[3] = {{1.4f, 1.f}, {1.4f, 1.f}, {1.4f, 1.f}};

    for (int i = 0; i < 3; ++i)
        initScreen(screens.emplaceBack(), world, startPositions[i], screenHalfSizes[i], palette[i], sharedFontAtlas);

    // ----- Drag/interaction state -----

    zb::Optional<b2BodyId> draggedBody;
    za::Vec2f                    dragLocalAnchor{};

    Screen* activeScreen = nullptr;

    za::Clock clock;

    while (true)
    {
        const za::Time  dt          = clock.restart();
        const za::Vec2f windowSizeF = window.getSize().toVec2f();

        const auto pixelToWorldM = [&](const za::Vec2i pixel) { return pixelToWorld(pixel, windowSizeF, worldView); };

        // ----- Event handling -----

        while (const zb::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (const auto* moved = event->getIf<za::Event::MouseMoved>())
            {
                if (activeScreen != nullptr)
                {
                    const za::Vec2f worldM = pixelToWorldM(moved->position);
                    const za::Vec2f localM = toSfVec(b2Body_GetLocalPoint(activeScreen->bodyId, toB2Vec(worldM)));

                    activeScreen->cursorTexPos = localToTexture(localM, activeScreen->halfSizeM, activeScreen->textureSize);

                    activeScreen->imGuiContext->processEvent(window,
                                                             za::Event::MouseMoved{
                                                                 .position = activeScreen->cursorTexPos.toVec2i(),
                                                             });
                }
            }
            else if (const auto* pressed = event->getIf<za::Event::MouseButtonPressed>())
            {
                const za::Vec2f worldM = pixelToWorldM(pressed->position);

                za::Vec2f localM{};
                Screen*   hitScreen = findScreenAt(screens, worldM, localM);

                if (pressed->button == za::Mouse::Button::Right && hitScreen != nullptr)
                {
                    draggedBody     = zb::makeOptional(hitScreen->bodyId);
                    dragLocalAnchor = localM;
                    b2Body_SetAwake(hitScreen->bodyId, true);
                }
                else if (pressed->button == za::Mouse::Button::Left && hitScreen != nullptr)
                {
                    activeScreen               = hitScreen;
                    activeScreen->cursorTexPos = localToTexture(localM, hitScreen->halfSizeM, hitScreen->textureSize);

                    // Synthesize a move first so ImGui has the cursor position before the click
                    activeScreen->imGuiContext->processEvent(window,
                                                             za::Event::MouseMoved{
                                                                 .position = activeScreen->cursorTexPos.toVec2i(),
                                                             });

                    activeScreen->imGuiContext->processEvent(window,
                                                             za::Event::MouseButtonPressed{
                                                                 .button   = pressed->button,
                                                                 .position = activeScreen->cursorTexPos.toVec2i(),
                                                             });
                }
            }
            else if (const auto* released = event->getIf<za::Event::MouseButtonReleased>())
            {
                if (released->button == za::Mouse::Button::Right)
                {
                    draggedBody.reset();
                }
                else if (released->button == za::Mouse::Button::Left && activeScreen != nullptr)
                {
                    const za::Vec2f worldM = pixelToWorldM(released->position);
                    const za::Vec2f localM = toSfVec(b2Body_GetLocalPoint(activeScreen->bodyId, toB2Vec(worldM)));
                    const za::Vec2f tex    = localToTexture(localM, activeScreen->halfSizeM, activeScreen->textureSize);

                    activeScreen->imGuiContext->processEvent(window,
                                                             za::Event::MouseButtonReleased{
                                                                 .button   = released->button,
                                                                 .position = tex.toVec2i(),
                                                             });

                    activeScreen = nullptr;
                }
            }
        }

        // ----- Spring drag -----

        if (draggedBody.hasValue())
        {
            const za::Vec2i mousePixel  = za::Mouse::getPosition(window);
            const za::Vec2f targetWorld = pixelToWorldM(mousePixel);
            const b2Vec2    worldAnchor = b2Body_GetWorldPoint(*draggedBody, toB2Vec(dragLocalAnchor));

            constexpr float stiffness = 60.f;
            constexpr float damping   = 8.f;
            const float     mass      = b2Body_GetMass(*draggedBody);

            const b2Vec2 ptVel = b2Body_GetWorldPointVelocity(*draggedBody, worldAnchor);
            const b2Vec2 force{stiffness * mass * (targetWorld.x - worldAnchor.x) - damping * mass * ptVel.x,
                               stiffness * mass * (targetWorld.y - worldAnchor.y) - damping * mass * ptVel.y};

            b2Body_ApplyForce(*draggedBody, force, worldAnchor, true);
        }

        // ----- Physics step -----

        b2World_Step(world, dt.asSeconds(), 4);

        // ----- Main render: walls first, then per-screen UI rendered through the shared scratch RT -----

        window.clear({30u, 32u, 38u});

        const auto drawCtx = window.withRenderStates({.view = worldView});

        // For each screen: build UI -> render into the shared scratch RT -> composite onto the main
        // window with the body's transform. The scratch RT is reused across screens (overwritten each
        // iteration), so we only pay for one framebuffer regardless of the number of screens.
        for (Screen& s : screens)
        {
            s.imGuiContext->setCurrent();

            // For the actively-interacted screen, refresh cursor each frame from real-time mouse;
            // for inactive screens, freeze the cursor far away so ImGui doesn't think it's hovering.
            za::Vec2f cursorTexPos = s.cursorTexPos;
            if (&s == activeScreen)
            {
                const za::Vec2f worldM = pixelToWorldM(za::Mouse::getPosition(window));
                const za::Vec2f localM = toSfVec(b2Body_GetLocalPoint(s.bodyId, toB2Vec(worldM)));

                cursorTexPos   = localToTexture(localM, s.halfSizeM, s.textureSize);
                s.cursorTexPos = cursorTexPos;
            }
            else
            {
                cursorTexPos = {-1.f, -1.f};
            }

            s.imGuiContext->update(cursorTexPos.toVec2i(), s.textureSize.toVec2f(), dt);

            ImGui::SetNextWindowPos({0.f, 0.f});
            ImGui::SetNextWindowSize({static_cast<float>(s.textureSize.x), static_cast<float>(s.textureSize.y)});

            ImGui::Begin("Screen",
                         nullptr,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

            ImGui::TextUnformatted("In-world ImGui");
            ImGui::Separator();

            ImGui::SliderFloat("Scale", &s.sliderValue, 0.5f, 2.f, "%.2fx");
            s.sliderActive = ImGui::IsItemActive();

            if (ImGui::Button("Click me"))
                ++s.counter;

            ImGui::SameLine();
            ImGui::Text("count = %d", s.counter);

            ImGui::Checkbox("Checkbox", &s.checkBox);

            ImGui::ColorEdit3("Tint", s.tintRGB);

            ImGui::Separator();
            ImGui::TextWrapped("Right-drag this surface to fling it around.");

            ImGui::End();

            // Render this screen's UI into the shared scratch RT.
            scratchRT.clear(za::Color::White);
            s.imGuiContext->render(scratchRT);
            scratchRT.display();

            // Composite the scratch onto the main window with the body's transform. Subsequent
            // screens will overwrite scratchRT, but by then we've already drawn this one out.
            const za::Vec2f bodyPos   = toSfVec(b2Body_GetPosition(s.bodyId));
            const float     bodyAngle = b2Rot_GetAngle(b2Body_GetRotation(s.bodyId));
            const za::Vec2f sizeM     = s.halfSizeM * 2.f;

            drawCtx.draw(scratchRT.getTexture(),
                         {
                             .position    = bodyPos,
                             .scale       = sizeM.componentWiseDiv(s.textureSize.toVec2f()),
                             .origin      = s.textureSize.toVec2f() / 2.f,
                             .rotation    = za::radians(bodyAngle),
                             .textureRect = {{0.f, 0.f}, s.textureSize.toVec2f()},
                             .color       = za::Color::fromFloats(s.tintRGB[0], s.tintRGB[1], s.tintRGB[2]),
                         });

            // The next iteration calls `scratchRT.display()`, which bumps the texture's
            // destructive-generation counter. The auto-batch detects the mismatch on the
            // next draw and flushes. We still need an explicit flush here because the next
            // iteration's destructive ops on `scratchRT` happen BEFORE that auto-flush
            // would fire, and at flush time GL would sample the texture's current (wrong)
            // content. The generation counter is for diagnosis (loud assert under
            // `withLockedRenderStates`) -- correctness still requires this manual flush.
            window.flush();

            // Gravity-driven slider drift. The slider's local axis is the body's +x; in world
            // coords that direction is `(cos θ, sin θ)`. Gravity is `(0, +9.81)` (y-down), so
            // the gravity component along the slider axis is proportional to `sin θ`. Below a
            // small threshold we model static friction and don't move; above it we accelerate
            // the slider value toward the "downhill" end. While the user is actively dragging
            // the slider their input wins, so we zero the velocity instead.
            constexpr float sliderFrictionSinThresh = 0.12f; // ~6.9°
            constexpr float sliderGravityStrength   = 0.6f;  // slider units / s² when fully tilted
            constexpr float sliderDamping           = 1.5f;  // per-second velocity decay
            constexpr float sliderMin               = 0.5f;
            constexpr float sliderMax               = 2.0f;

            if (s.sliderActive)
            {
                s.sliderVelocity = 0.f;
            }
            else
            {
                const float bodyAngleRad = b2Rot_GetAngle(b2Body_GetRotation(s.bodyId));
                const float gravityAlong = zb::sin(bodyAngleRad);

                if (zb::fabs(gravityAlong) > sliderFrictionSinThresh)
                    s.sliderVelocity += gravityAlong * sliderGravityStrength * dt.asSeconds();

                s.sliderVelocity -= s.sliderVelocity * sliderDamping * dt.asSeconds();
                s.sliderValue += s.sliderVelocity * dt.asSeconds();

                if (s.sliderValue <= sliderMin)
                {
                    s.sliderValue    = sliderMin;
                    s.sliderVelocity = 0.f;
                }
                else if (s.sliderValue >= sliderMax)
                {
                    s.sliderValue    = sliderMax;
                    s.sliderVelocity = 0.f;
                }
            }

            // If the slider moved (by the user or by gravity), resync the collision shape.
            if (zb::fabs(s.sliderValue - s.currentScale) > 0.001f)
                rescaleScreenShape(s);
        }

        // Drag rubber-band
        if (draggedBody.hasValue())
        {
            const za::Vec2f anchorWorld = toSfVec(b2Body_GetWorldPoint(*draggedBody, toB2Vec(dragLocalAnchor)));
            const za::Vec2f mouseWorld  = pixelToWorldM(za::Mouse::getPosition(window));

            // anchor dot
            drawCtx.draw(za::CircleShapeData{
                .position  = anchorWorld,
                .origin    = {0.06f, 0.06f},
                .fillColor = za::Color::Red,
                .radius    = 0.06f,
            });

            // line from anchor to cursor (built as a thin rectangle)
            const za::Vec2f delta  = mouseWorld - anchorWorld;
            const float     length = delta.length();

            if (length > 0.f)
            {
                drawCtx.draw(za::RectangleShapeData{
                    .position  = anchorWorld,
                    .origin    = {0.f, 0.02f},
                    .rotation  = za::radians(zb::atan2(delta.y, delta.x)),
                    .fillColor = {255u, 80u, 80u, 220u},
                    .size      = {length, 0.04f},
                });
            }
        }

        // HUD instructions (drawn in window pixels)
        window.draw(font,
                    za::TextData{
                        .position      = {12.f, 8.f},
                        .string        = "Left-click a surface to interact with its UI - Right-drag to fling it",
                        .characterSize = 16u,
                        .fillColor     = {220u, 220u, 230u},
                    });

        window.display();
    }
}
