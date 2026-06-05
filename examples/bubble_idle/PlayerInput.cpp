#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "InputHelper.hpp"
#include "PlayerInput.hpp"
#include "Playthrough.hpp"
#include "UIState.hpp"
#include "Version.hpp"

#include "Zancle/ImGui/ImGuiContext.hpp"
#include "Zancle/ImGui/IncludeImGui.hpp"

#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/View.hpp"

#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/Keyboard.hpp"
#include "Zancle/Window/Mouse.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Algorithm/Erase.hpp"
#include "Zancle/Math/Lround.hpp"
#include "Zancle/Math/MinMax.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Container/Vector.hpp"

namespace
{
void clampDemoPlaythrough(Main& main)
{
    if constexpr (!isDemoVersion)
        return;

    const auto clampNPurchases = [](auto& psv) { psv.nPurchases = za::min(psv.nPurchases, psv.data->nMaxPurchases); };

    clampNPurchases(main.pt->psvMapExtension);
    clampNPurchases(main.pt->psvShrineActivation);
    clampNPurchases(main.pt->psvBubbleValue);

    za::vectorEraseIf(main.pt->cats,
                      [](const Cat& cat) { return cat.type >= CatType::Mouse && cat.type <= CatType::Duck; });
}

[[nodiscard]] za::Vector<za::Vec2f> collectDownFingers(const Main& main)
{
    za::Vector<za::Vec2f> downFingers;

    for (const auto maybeFinger : main.playerInputState.fingerPositions)
        if (maybeFinger.hasValue())
            downFingers.pushBack(*maybeFinger);

    return downFingers;
}

void handleGameLoopScrollInput(Main& main, const float deltaTimeMs, const za::Vector<za::Vec2f>& downFingers)
{
    if (!main.pt->mapPurchased)
        return;

    if (main.inputHelper.wasKeyJustPressed(za::Keyboard::Key::Home))
        main.playerInputState.scroll = 0.f;
    else if (main.inputHelper.wasKeyJustPressed(za::Keyboard::Key::End))
        main.playerInputState.scroll = static_cast<float>(main.pt->getMapLimitIncreases()) * gameScreenSize.x * 0.5f;

    const auto currentScrollScreenIndex = static_cast<za::SizeT>(
        za::lround(main.playerInputState.scroll / (gameScreenSize.x * 0.5f)));

    if (main.inputHelper.wasKeyJustPressed(za::Keyboard::Key::PageDown) ||
        main.inputHelper.wasMouseButtonJustPressed(za::Mouse::Button::Extra2))
    {
        const auto nextScrollScreenIndex = za::min(currentScrollScreenIndex + 1u, main.pt->getMapLimitIncreases());
        main.playerInputState.scroll     = static_cast<float>(nextScrollScreenIndex) * gameScreenSize.x * 0.5f;
    }
    else if ((main.inputHelper.wasKeyJustPressed(za::Keyboard::Key::PageUp) ||
              main.inputHelper.wasMouseButtonJustPressed(za::Mouse::Button::Extra1)) &&
             currentScrollScreenIndex > 0u)
    {
        const auto nextScrollScreenIndex = za::max(static_cast<za::SizeT>(0u), currentScrollScreenIndex - 1u);

        main.playerInputState.scroll = static_cast<float>(nextScrollScreenIndex) * gameScreenSize.x * 0.5f;
    }

    const float scrollMult = main.keyDown(za::Keyboard::Key::LShift) ? 4.f : 2.f;

    if (main.keyDown(za::Keyboard::Key::Left) || main.keyDown(za::Keyboard::Key::A))
    {
        main.playerInputState.dragPosition.reset();
        main.playerInputState.scroll -= scrollMult * deltaTimeMs;
    }
    else if (main.keyDown(za::Keyboard::Key::Right) || main.keyDown(za::Keyboard::Key::D))
    {
        main.playerInputState.dragPosition.reset();
        main.playerInputState.scroll += scrollMult * deltaTimeMs;
    }
    else if (downFingers.size() == 2)
    {
        const auto avg = (downFingers[0] + downFingers[1]) / 2.f;

        if (main.playerInputState.dragPosition.hasValue())
        {
            main.playerInputState.scroll = main.playerInputState.dragPosition->x - avg.x;
        }
        else
        {
            main.playerInputState.dragPosition.emplace(avg);
            main.playerInputState.dragPosition->x += main.playerInputState.scroll;
        }
    }
}
} // namespace

////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::gameLoopHandleEvents(FrameInput& frameInput, const bool shouldDrawUI)
{
    while (const za::Optional event = window.pollEvent())
    {
        inputHelper.applyEvent(*event);
        imGuiContext.processEvent(window, *event);

        if (shouldDrawUI && event->is<za::Event::KeyPressed>() &&
            event->getIf<za::Event::KeyPressed>()->code == za::Keyboard::Key::Escape)
        {
            if (!playerInputState.escWasPressed)
            {
                playSound(sounds.btnswitch);
                playerInputState.escWasPressed = true;
            }
        }

        if (isDebugModeEnabled() && event->is<za::Event::KeyPressed>() &&
            event->getIf<za::Event::KeyPressed>()->code == za::Keyboard::Key::F8)
        {
            uiState.debugWindowVisible = !uiState.debugWindowVisible;
            playSound(sounds.uitab);
        }

        if (event->is<za::Event::Closed>())
            return false;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
        if (const auto* e0 = event->getIf<za::Event::TouchBegan>())
        {
            playerInputState.fingerPositions[e0->finger].emplace(e0->position.toVec2f());

            if (!frameInput.clickPosition.hasValue())
                frameInput.clickPosition.emplace(e0->position.toVec2f());
        }
        else if (const auto* e1 = event->getIf<za::Event::TouchEnded>())
        {
            playerInputState.fingerPositions[e1->finger].reset();
        }
        else if (const auto* e2 = event->getIf<za::Event::TouchMoved>())
        {
            playerInputState.fingerPositions[e2->finger].emplace(e2->position.toVec2f());

            if (pt->laserPopEnabled && !frameInput.clickPosition.hasValue())
                frameInput.clickPosition.emplace(e2->position.toVec2f());
        }
        else if (const auto* e3 = event->getIf<za::Event::MouseButtonPressed>())
        {
            if (e3->button == getLMB())
                frameInput.clickPosition.emplace(e3->position.toVec2f());

            if (e3->button == getRMB() && !playerInputState.dragPosition.hasValue())
            {
                frameInput.clickPosition.reset();

                playerInputState.dragPosition.emplace(e3->position.toVec2f());
                playerInputState.dragPosition->x += playerInputState.scroll;
            }
        }
        else if (const auto* e4 = event->getIf<za::Event::MouseButtonReleased>())
        {
            if (e4->button == getRMB())
                playerInputState.dragPosition.reset();
        }
        else if (const auto* e5 = event->getIf<za::Event::MouseMoved>())
        {
            if (pt->mapPurchased && playerInputState.dragPosition.hasValue())
                playerInputState.scroll = playerInputState.dragPosition->x - static_cast<float>(e5->position.x);
        }
        else if (const auto* e6 = event->getIf<za::Event::Resized>())
        {
            recreateBackgroundRenderTexture(getExpandedGameViewSize(gameScreenSize, e6->size.toVec2f()).toVec2u());
            recreateImGuiRenderTexture(e6->size);
            recreateGameRenderTexture(e6->size);

            hudTopParticles.clear();
            hudBottomParticles.clear();
        }
        else if (const auto* e7 = event->getIf<za::Event::KeyPressed>())
        {
            if (e7->code == za::Keyboard::Key::Z || e7->code == za::Keyboard::Key::X || e7->code == za::Keyboard::Key::Y)
                frameInput.clickPosition.emplace(za::Mouse::getPosition(window).toVec2f());
        }
        else if (const auto* e8 = event->getIf<za::Event::MouseWheelScrolled>())
        {
            const float scrollMult = keyDown(za::Keyboard::Key::LShift) ? 200.f : 100.f;

            if (!ImGui::GetIO().WantCaptureMouse)
                playerInputState.scroll += e8->delta * scrollMult;
        }
#pragma GCC diagnostic pop
    }

    return true;
}

////////////////////////////////////////////////////////////
void Main::gameLoopPrepareInput(FrameInput& frameInput, const float deltaTimeMs)
{
    if (ImGui::GetIO().WantCaptureMouse)
        frameInput.clickPosition.reset();

    gameLoopCheats();
    clampDemoPlaythrough(*this);

    if (pt->laserPopEnabled)
        if (keyDown(za::Keyboard::Key::Z) || keyDown(za::Keyboard::Key::X) || keyDown(za::Keyboard::Key::Y) ||
            mBtnDown(getLMB(), /* penetrateUI */ false))
        {
            if (!frameInput.clickPosition.hasValue())
                frameInput.clickPosition.emplace(za::Mouse::getPosition(window).toVec2f());
        }

    frameInput.downFingers = collectDownFingers(*this);
    handleGameLoopScrollInput(*this, deltaTimeMs, frameInput.downFingers);
    gameLoopUpdateScrolling(deltaTimeMs, frameInput.downFingers);

    frameInput.windowSpaceMouseOrFingerPos = frameInput.downFingers.size() == 1u ? frameInput.downFingers[0].toVec2i()
                                                                                 : za::Mouse::getPosition(window);

    if (frameInput.clickPosition.hasValue() && pt->mapPurchased)
    {
        const auto p = scaledHUDView.screenToWorld(frameInput.windowSpaceMouseOrFingerPos.toVec2f(),
                                                   window.getSize().toVec2f());

        if (uiState.minimapRect.contains(p))
            frameInput.clickPosition.reset();
    }

    const za::Vec2f resolution = getResolution();
    hudCullingBoundaries       = {0.f, resolution.x, 0.f, resolution.y};
    particleCullingBoundaries  = getViewCullingBoundaries(/* offset */ 0.f);
    bubbleCullingBoundaries    = getViewCullingBoundaries(/* offset */ -64.f);

    frameInput.mousePos = gameView.screenToWorld(frameInput.windowSpaceMouseOrFingerPos.toVec2f(),
                                                 window.getSize().toVec2f());
}
