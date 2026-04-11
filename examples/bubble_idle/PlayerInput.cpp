#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "PlayerInput.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Math/Lround.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

namespace
{
void clampDemoPlaythrough(Main& main)
{
    if constexpr (!isDemoVersion)
        return;

    const auto clampNPurchases = [](auto& psv)
    { psv.nPurchases = sf::base::min(psv.nPurchases, psv.data->nMaxPurchases); };

    clampNPurchases(main.pt->psvMapExtension);
    clampNPurchases(main.pt->psvShrineActivation);
    clampNPurchases(main.pt->psvBubbleValue);

    sf::base::vectorEraseIf(main.pt->cats, [](const Cat& cat) {
        return cat.type >= CatType::Mouse && cat.type <= CatType::Duck;
    });
}

[[nodiscard]] sf::base::Vector<sf::Vec2f> collectDownFingers(const Main& main)
{
    sf::base::Vector<sf::Vec2f> downFingers;

    for (const auto maybeFinger : main.playerInputState.fingerPositions)
        if (maybeFinger.hasValue())
            downFingers.pushBack(*maybeFinger);

    return downFingers;
}

void handleGameLoopScrollInput(Main& main, const float deltaTimeMs, const sf::base::Vector<sf::Vec2f>& downFingers)
{
    if (!main.pt->mapPurchased)
        return;

    if (main.inputHelper.wasKeyJustPressed(sf::Keyboard::Key::Home))
        main.playerInputState.scroll = 0.f;
    else if (main.inputHelper.wasKeyJustPressed(sf::Keyboard::Key::End))
        main.playerInputState.scroll = static_cast<float>(main.pt->getMapLimitIncreases()) * gameScreenSize.x * 0.5f;

    const auto currentScrollScreenIndex = static_cast<sf::base::SizeT>(
        sf::base::lround(main.playerInputState.scroll / (gameScreenSize.x * 0.5f)));

    if (main.inputHelper.wasKeyJustPressed(sf::Keyboard::Key::PageDown) ||
        main.inputHelper.wasMouseButtonJustPressed(sf::Mouse::Button::Extra2))
    {
        const auto nextScrollScreenIndex = sf::base::min(currentScrollScreenIndex + 1u, main.pt->getMapLimitIncreases());
        main.playerInputState.scroll = static_cast<float>(nextScrollScreenIndex) * gameScreenSize.x * 0.5f;
    }
    else if ((main.inputHelper.wasKeyJustPressed(sf::Keyboard::Key::PageUp) ||
              main.inputHelper.wasMouseButtonJustPressed(sf::Mouse::Button::Extra1)) &&
             currentScrollScreenIndex > 0u)
    {
        const auto nextScrollScreenIndex = sf::base::max(static_cast<sf::base::SizeT>(0u), currentScrollScreenIndex - 1u);

        main.playerInputState.scroll = static_cast<float>(nextScrollScreenIndex) * gameScreenSize.x * 0.5f;
    }

    const float scrollMult = main.keyDown(sf::Keyboard::Key::LShift) ? 4.f : 2.f;

    if (main.keyDown(sf::Keyboard::Key::Left) || main.keyDown(sf::Keyboard::Key::A))
    {
        main.playerInputState.dragPosition.reset();
        main.playerInputState.scroll -= scrollMult * deltaTimeMs;
    }
    else if (main.keyDown(sf::Keyboard::Key::Right) || main.keyDown(sf::Keyboard::Key::D))
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
    while (const sf::base::Optional event = window.pollEvent())
    {
        inputHelper.applyEvent(*event);
        imGuiContext.processEvent(window, *event);

        if (shouldDrawUI && event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)
        {
            if (!playerInputState.escWasPressed)
            {
                playSound(sounds.btnswitch);
                playerInputState.escWasPressed = true;
            }
        }

        if (event->is<sf::Event::Closed>())
            return false;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
        if (const auto* e0 = event->getIf<sf::Event::TouchBegan>())
        {
            playerInputState.fingerPositions[e0->finger].emplace(e0->position.toVec2f());

            if (!frameInput.clickPosition.hasValue())
                frameInput.clickPosition.emplace(e0->position.toVec2f());
        }
        else if (const auto* e1 = event->getIf<sf::Event::TouchEnded>())
        {
            playerInputState.fingerPositions[e1->finger].reset();
        }
        else if (const auto* e2 = event->getIf<sf::Event::TouchMoved>())
        {
            playerInputState.fingerPositions[e2->finger].emplace(e2->position.toVec2f());

            if (pt->laserPopEnabled && !frameInput.clickPosition.hasValue())
                frameInput.clickPosition.emplace(e2->position.toVec2f());
        }
        else if (const auto* e3 = event->getIf<sf::Event::MouseButtonPressed>())
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
        else if (const auto* e4 = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if (e4->button == getRMB())
                playerInputState.dragPosition.reset();
        }
        else if (const auto* e5 = event->getIf<sf::Event::MouseMoved>())
        {
            if (pt->mapPurchased && playerInputState.dragPosition.hasValue())
                playerInputState.scroll = playerInputState.dragPosition->x - static_cast<float>(e5->position.x);
        }
        else if (const auto* e6 = event->getIf<sf::Event::Resized>())
        {
            recreateBackgroundRenderTexture(getExpandedGameViewSize(gameScreenSize, e6->size.toVec2f()).toVec2u());
            recreateImGuiRenderTexture(e6->size);
            recreateGameRenderTexture(e6->size);

            hudTopParticles.clear();
            hudBottomParticles.clear();
        }
        else if (const auto* e7 = event->getIf<sf::Event::KeyPressed>())
        {
            if (e7->code == sf::Keyboard::Key::Z || e7->code == sf::Keyboard::Key::X || e7->code == sf::Keyboard::Key::Y)
                frameInput.clickPosition.emplace(sf::Mouse::getPosition(window).toVec2f());
        }
        else if (const auto* e8 = event->getIf<sf::Event::MouseWheelScrolled>())
        {
            const float scrollMult = keyDown(sf::Keyboard::Key::LShift) ? 200.f : 100.f;

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
        if (keyDown(sf::Keyboard::Key::Z) || keyDown(sf::Keyboard::Key::X) || keyDown(sf::Keyboard::Key::Y) ||
            mBtnDown(getLMB(), /* penetrateUI */ false))
        {
            if (!frameInput.clickPosition.hasValue())
                frameInput.clickPosition.emplace(sf::Mouse::getPosition(window).toVec2f());
        }

    frameInput.downFingers = collectDownFingers(*this);
    handleGameLoopScrollInput(*this, deltaTimeMs, frameInput.downFingers);
    gameLoopUpdateScrolling(deltaTimeMs, frameInput.downFingers);

    frameInput.windowSpaceMouseOrFingerPos = frameInput.downFingers.size() == 1u ? frameInput.downFingers[0].toVec2i()
                                                                                 : sf::Mouse::getPosition(window);

    if (frameInput.clickPosition.hasValue() && pt->mapPurchased)
    {
        const auto p = scaledHUDView.screenToWorld(frameInput.windowSpaceMouseOrFingerPos.toVec2f(),
                                                   window.getSize().toVec2f());

        if (uiState.minimapRect.contains(p))
            frameInput.clickPosition.reset();
    }

    const sf::Vec2f resolution = getResolution();
    hudCullingBoundaries       = {0.f, resolution.x, 0.f, resolution.y};
    particleCullingBoundaries  = getViewCullingBoundaries(/* offset */ 0.f);
    bubbleCullingBoundaries    = getViewCullingBoundaries(/* offset */ -64.f);

    frameInput.mousePos = gameView.screenToWorld(frameInput.windowSpaceMouseOrFingerPos.toVec2f(),
                                                 window.getSize().toVec2f());
}
