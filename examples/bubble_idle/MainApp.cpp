

#include "BGMStorage.hpp"
#include "BubbleIdleApp.hpp"
#include "BubbleIdleMain.hpp"
#include "ComboState.hpp"
#include "Constants.hpp"
#include "GameConstants.hpp"
#include "IconsFontAwesome6.h"
#include "InputHelper.hpp"
#include "MainAtlasRects.hpp"
#include "MainBombStorage.hpp"
#include "MainGameStorage.hpp"
#include "MainShaders.hpp"
#include "NotificationState.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"
#include "RNGSeedType.hpp"
#include "Serialization.hpp"
#include "Steam.hpp"
#include "SweepAndPrune.hpp"
#include "UIState.hpp"

#include "ExampleUtils/SoundManager.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"
#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Priv/ShaderBase.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
constexpr sf::TextureLoadSettings bgSettings{.smooth = true, .wrapMode = sf::TextureWrapMode::Repeat};
} // namespace


////////////////////////////////////////////////////////////
struct MainDrawableBatches
{
    sf::CPUDrawableBatch bubbleDrawableBatch;
    sf::CPUDrawableBatch starBubbleDrawableBatch;
    sf::CPUDrawableBatch bombBubbleDrawableBatch;
    sf::CPUDrawableBatch cpuCloudDrawableBatch;
    sf::CPUDrawableBatch cpuTopCloudDrawableBatch;
    sf::CPUDrawableBatch cpuCloudHudDrawableBatch;
    sf::CPUDrawableBatch cpuCloudUiDrawableBatch;
    sf::CPUDrawableBatch cpuDrawableBatchBeforeCats;
    sf::CPUDrawableBatch cpuDrawableBatch;
    sf::CPUDrawableBatch cpuDrawableBatchAfterCats;
    sf::CPUDrawableBatch cpuDrawableBatchAdditive;
    sf::CPUDrawableBatch minimapDrawableBatch;
    sf::CPUDrawableBatch catTextDrawableBatch;
    sf::CPUDrawableBatch hudDrawableBatch;
    sf::CPUDrawableBatch hudTopDrawableBatch;
    sf::CPUDrawableBatch hudBottomDrawableBatch;
    sf::CPUDrawableBatch cpuTopDrawableBatch;
    sf::CPUDrawableBatch catTextTopDrawableBatch;
    sf::CPUDrawableBatch tempDrawableBatch;
};


////////////////////////////////////////////////////////////
struct MainTextureStorage
{
    sf::Texture txLogo;
    sf::Texture txFixedBg;
    sf::Texture txBackgroundChunk;
    sf::Texture txBackgroundChunkDesaturated;
    sf::Texture txClouds;
    sf::Texture txTintedClouds;
    sf::Texture txBgSwamp;
    sf::Texture txBgObservatory;
    sf::Texture txBgAimTraining;
    sf::Texture txBgFactory;
    sf::Texture txBgWindTunnel;
    sf::Texture txBgMagnetosphere;
    sf::Texture txBgAuditorium;
    sf::Texture txDrawings;
    sf::Texture txTipBg;
    sf::Texture txTipByte;
    sf::Texture txCursor;
    sf::Texture txCursorMultipop;
    sf::Texture txCursorLaser;
    sf::Texture txCursorGrab;
    sf::Texture txArrow;
    sf::Texture txUnlock;
    sf::Texture txPurchasable;
    sf::Texture txLetter;
    sf::Texture txLetterText;
    sf::Texture txFrame;
    sf::Texture txFrameTiny;
    sf::Texture txCloudBtn;
    sf::Texture txCloudBtnSmall;
    sf::Texture txCloudBtnSquare;
    sf::Texture txCloudBtnSquare2;

    MainTextureStorage() :
        txLogo(sf::Texture::loadFromFile("resources/logo.png", {.smooth = true}).value()),
        txFixedBg(sf::Texture::loadFromFile("resources/fixedbg.png",
                                            {.smooth = true, .wrapMode = sf::TextureWrapMode::MirroredRepeat})
                      .value()),
        txBackgroundChunk(sf::Texture::loadFromFile("resources/bgtest.png", bgSettings).value()),
        txBackgroundChunkDesaturated(sf::Texture::loadFromFile("resources/bgtestdesaturated.png", bgSettings).value()),
        txClouds(sf::Texture::loadFromFile("resources/clouds.png", bgSettings).value()),
        txTintedClouds(sf::Texture::loadFromFile("resources/tintedclouds.png", bgSettings).value()),
        txBgSwamp(sf::Texture::loadFromFile("resources/bgswamp.png", bgSettings).value()),
        txBgObservatory(sf::Texture::loadFromFile("resources/bgobservatory.png", bgSettings).value()),
        txBgAimTraining(sf::Texture::loadFromFile("resources/bgaimtraining.png", bgSettings).value()),
        txBgFactory(sf::Texture::loadFromFile("resources/bgfactory.png", bgSettings).value()),
        txBgWindTunnel(sf::Texture::loadFromFile("resources/bgwindtunnel.png", bgSettings).value()),
        txBgMagnetosphere(sf::Texture::loadFromFile("resources/bgmagnetosphere.png", bgSettings).value()),
        txBgAuditorium(sf::Texture::loadFromFile("resources/bgauditorium.png", bgSettings).value()),
        txDrawings(sf::Texture::loadFromFile("resources/drawings.png", {.smooth = true}).value()),
        txTipBg(sf::Texture::loadFromFile("resources/tipbg.png", {.smooth = true}).value()),
        txTipByte(sf::Texture::loadFromFile("resources/tipbyte.png", {.smooth = true}).value()),
        txCursor(sf::Texture::loadFromFile("resources/cursor.png", {.smooth = true}).value()),
        txCursorMultipop(sf::Texture::loadFromFile("resources/cursormultipop.png", {.smooth = true}).value()),
        txCursorLaser(sf::Texture::loadFromFile("resources/cursorlaser.png", {.smooth = true}).value()),
        txCursorGrab(sf::Texture::loadFromFile("resources/cursorgrab.png", {.smooth = true}).value()),
        txArrow(sf::Texture::loadFromFile("resources/arrow.png", {.smooth = true}).value()),
        txUnlock(sf::Texture::loadFromFile("resources/unlock.png", {.smooth = true}).value()),
        txPurchasable(sf::Texture::loadFromFile("resources/purchasable.png", {.smooth = true}).value()),
        txLetter(sf::Texture::loadFromFile("resources/letter.png", {.smooth = true}).value()),
        txLetterText(sf::Texture::loadFromFile("resources/lettertext.png", {.smooth = true}).value()),
        txFrame(sf::Texture::loadFromFile("resources/frame.png", {.smooth = true}).value()),
        txFrameTiny(sf::Texture::loadFromFile("resources/frametiny.png", {.smooth = true}).value()),
        txCloudBtn(sf::Texture::loadFromFile("resources/cloudbtn.png", {.smooth = true}).value()),
        txCloudBtnSmall(sf::Texture::loadFromFile("resources/cloudbtnsmall.png", {.smooth = true}).value()),
        txCloudBtnSquare(sf::Texture::loadFromFile("resources/cloudbtnsquare.png", {.smooth = true}).value()),
        txCloudBtnSquare2(sf::Texture::loadFromFile("resources/cloudbtnsquare2.png", {.smooth = true}).value())
    {
    }
};


////////////////////////////////////////////////////////////
struct MainTextStorage
{
    sf::Text moneyText;
    sf::Text demoText;
    sf::Text textNameBuffer;
    sf::Text textStatusBuffer;
    sf::Text textMoneyBuffer;

    explicit MainTextStorage(const sf::Font& fontSuperBakery) :
        moneyText(fontSuperBakery,
                  {.position         = Main::moneyTextInitialPosition,
                   .string           = "$0",
                   .characterSize    = 64u,
                   .fillColor        = sf::Color::White,
                   .outlineColor     = colorBlueOutline,
                   .outlineThickness = 4.f}),
        demoText(fontSuperBakery,
                 {.position         = {},
                  .string           = "DEMO VERSION",
                  .characterSize    = 48u,
                  .fillColor        = sf::Color::White,
                  .outlineColor     = colorBlueOutline,
                  .outlineThickness = 3.f}),
        textNameBuffer(fontSuperBakery,
                       {
                           .string           = "",
                           .characterSize    = 48u,
                           .fillColor        = sf::Color::White,
                           .outlineColor     = colorBlueOutline,
                           .outlineThickness = 3.f,
                       }),
        textStatusBuffer(fontSuperBakery,
                         {
                             .string           = "",
                             .characterSize    = 32u,
                             .fillColor        = sf::Color::White,
                             .outlineColor     = colorBlueOutline,
                             .outlineThickness = 2.f,
                         }),
        textMoneyBuffer(fontSuperBakery,
                        {
                            .string           = "",
                            .characterSize    = 24u,
                            .fillColor        = sf::Color::White,
                            .outlineColor     = colorBlueOutline,
                            .outlineThickness = 1.5f,
                        })
    {
    }
};


////////////////////////////////////////////////////////////
template <typename T>
void MainOwnedDeleter<T>::operator()(T* ptr) noexcept
{
    delete ptr;
}


////////////////////////////////////////////////////////////
template struct MainOwnedDeleter<sf::RenderTexture>;
template struct MainOwnedDeleter<sf::AudioContext>;
template struct MainOwnedDeleter<sf::TextureAtlas>;
template struct MainOwnedDeleter<sf::RenderWindow>;
template struct MainOwnedDeleter<sf::Font>;
template struct MainOwnedDeleter<sf::GraphicsContext>;
template struct MainOwnedDeleter<sf::ImGuiContext>;
template struct MainOwnedDeleter<sf::Listener>;
template struct MainOwnedDeleter<sf::OutFile>;
template struct MainOwnedDeleter<sf::PlaybackDevice>;
template struct MainOwnedDeleter<sf::View>;
template struct MainOwnedDeleter<ComboState>;
template struct MainOwnedDeleter<InputHelper>;
template struct MainOwnedDeleter<MainBombStorage>;
template struct MainOwnedDeleter<MainDrawableBatches>;
template struct MainOwnedDeleter<MainAtlasRects>;
template struct MainOwnedDeleter<MainBGMStorage>;
template struct MainOwnedDeleter<MainGameStorage>;
template struct MainOwnedDeleter<MainShaders>;
template struct MainOwnedDeleter<MainTextStorage>;
template struct MainOwnedDeleter<MainTextureStorage>;
template struct MainOwnedDeleter<MainRenderTextureVector>;
template struct MainOwnedDeleter<NotificationState>;
template struct MainOwnedDeleter<UIState>;


////////////////////////////////////////////////////////////
bool debugMode = false;

////////////////////////////////////////////////////////////
void runBubbleIdleApp()
{
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    hg::Steam::SteamManager steamMgr;
    steamMgr.requestStatsAndAchievements();
    steamMgr.runCallbacks();

    // Using a heap-allocation here because `Main` exceeds the stack size
    sf::base::makeUnique<Main>(steamMgr)->run();
#else
    sf::base::makeUnique<Main>()->run();
#endif
}


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
Main::Main(hg::Steam::SteamManager& xSteamMgr) :
#else
Main::Main() :
#endif
#ifndef BUBBLEBYTE_NO_AUDIO
    audioContextStorage(new sf::AudioContext{sf::AudioContext::create().value()}),
    audioContext(*audioContextStorage),
    playbackDeviceStorage(new sf::PlaybackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()}),
    playbackDevice(*playbackDeviceStorage),
#endif
    graphicsContextStorage(new sf::GraphicsContext{sf::GraphicsContext::create().value()}),
    graphicsContext(*graphicsContextStorage),
    shadersStorage(
        []
{
    const auto loadShader = [](const sf::Shader::LoadFromFileSettings& settings)
    {
        auto s = sf::Shader::loadFromFile(settings).value();
        s.setUniform(s.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return s;
    };

    const auto bind = [](const sf::Shader& s, const char* name) { return s.getUniformLocation(name).value(); };

    auto shader = loadShader({.vertexPath = "resources/shader.vert", .fragmentPath = "resources/shader.frag"});

    const auto suBackgroundTexture   = bind(shader, "u_backgroundTexture");
    const auto suTime                = bind(shader, "u_time");
    const auto suResolution          = bind(shader, "u_resolution");
    const auto suBackgroundOrigin    = bind(shader, "u_backgroundOrigin");
    const auto suBubbleEffect        = bind(shader, "u_bubbleEffect");
    const auto suIridescenceStrength = bind(shader, "u_iridescenceStrength");
    const auto suEdgeFactorMin       = bind(shader, "u_edgeFactorMin");
    const auto suEdgeFactorMax       = bind(shader, "u_edgeFactorMax");
    const auto suEdgeFactorStrength  = bind(shader, "u_edgeFactorStrength");
    const auto suDistorsionStrength  = bind(shader, "u_distorsionStrength");
    const auto suSubTexOrigin        = bind(shader, "u_subTexOrigin");
    const auto suSubTexSize          = bind(shader, "u_subTexSize");
    const auto suBubbleLightness     = bind(shader, "u_bubbleLightness");
    const auto suLensDistortion      = bind(shader, "u_lensDistortion");
    const auto suRimShineStrength    = bind(shader, "u_rimShineStrength");
    const auto suRimShineFallRate    = bind(shader, "u_rimShineFallRate");
    const auto suRimShineTimeRate    = bind(shader, "u_rimShineTimeRate");
    const auto suRimShineArc         = bind(shader, "u_rimShineArc");

    // TODO P2: (lib) add support for `#include` in shaders
    auto shaderPostProcess = loadShader({.fragmentPath = "resources/postprocess.frag"});

    const auto suPPVibrance   = bind(shaderPostProcess, "u_vibrance");
    const auto suPPSaturation = bind(shaderPostProcess, "u_saturation");
    const auto suPPLightness  = bind(shaderPostProcess, "u_lightness");
    const auto suPPSharpness  = bind(shaderPostProcess, "u_sharpness");
    const auto suPPBlur       = bind(shaderPostProcess, "u_blur");

    auto shaderClouds = loadShader({.fragmentPath = "resources/clouds.frag"});

    const auto suCloudTime       = bind(shaderClouds, "u_time");
    const auto suCloudResolution = bind(shaderClouds, "u_resolution");

    auto shaderHexed = loadShader({.fragmentPath = "resources/hexed_cat.frag"});

    const auto suHexedTime               = bind(shaderHexed, "u_time");
    const auto suHexedSeed               = bind(shaderHexed, "u_seed");
    const auto suHexedDistortionStrength = bind(shaderHexed, "u_distortionStrength");
    const auto suHexedShimmerStrength    = bind(shaderHexed, "u_shimmerStrength");

    auto shaderShrineBackground = loadShader(
        {.vertexPath = "resources/shrine_background.vert", .fragmentPath = "resources/shrine_background.frag"});

    const auto suShrineBgTime               = bind(shaderShrineBackground, "u_time");
    const auto suShrineBgViewOrigin         = bind(shaderShrineBackground, "u_viewOrigin");
    const auto suShrineBgCenter             = bind(shaderShrineBackground, "u_shrineCenter");
    const auto suShrineBgRange              = bind(shaderShrineBackground, "u_shrineRange");
    const auto suShrineBgTintR              = bind(shaderShrineBackground, "u_shrineTintR");
    const auto suShrineBgTintG              = bind(shaderShrineBackground, "u_shrineTintG");
    const auto suShrineBgTintB              = bind(shaderShrineBackground, "u_shrineTintB");
    const auto suShrineBgTintA              = bind(shaderShrineBackground, "u_shrineTintA");
    const auto suShrineBgDistortionStrength = bind(shaderShrineBackground, "u_distortionStrength");
    const auto suShrineBgTintStrength       = bind(shaderShrineBackground, "u_tintStrength");
    const auto suShrineBgEffectStrength     = bind(shaderShrineBackground, "u_effectStrength");

    return new MainShaders{
        .shader                       = SFML_BASE_MOVE(shader),
        .suBackgroundTexture          = suBackgroundTexture,
        .suTime                       = suTime,
        .suResolution                 = suResolution,
        .suBackgroundOrigin           = suBackgroundOrigin,
        .suBubbleEffect               = suBubbleEffect,
        .suIridescenceStrength        = suIridescenceStrength,
        .suEdgeFactorMin              = suEdgeFactorMin,
        .suEdgeFactorMax              = suEdgeFactorMax,
        .suEdgeFactorStrength         = suEdgeFactorStrength,
        .suDistorsionStrength         = suDistorsionStrength,
        .suSubTexOrigin               = suSubTexOrigin,
        .suSubTexSize                 = suSubTexSize,
        .suBubbleLightness            = suBubbleLightness,
        .suLensDistortion             = suLensDistortion,
        .suRimShineStrength           = suRimShineStrength,
        .suRimShineFallRate           = suRimShineFallRate,
        .suRimShineTimeRate           = suRimShineTimeRate,
        .suRimShineArc                = suRimShineArc,
        .shaderPostProcess            = SFML_BASE_MOVE(shaderPostProcess),
        .suPPVibrance                 = suPPVibrance,
        .suPPSaturation               = suPPSaturation,
        .suPPLightness                = suPPLightness,
        .suPPSharpness                = suPPSharpness,
        .suPPBlur                     = suPPBlur,
        .shaderClouds                 = SFML_BASE_MOVE(shaderClouds),
        .suCloudTime                  = suCloudTime,
        .suCloudResolution            = suCloudResolution,
        .shaderHexed                  = SFML_BASE_MOVE(shaderHexed),
        .suHexedTime                  = suHexedTime,
        .suHexedSeed                  = suHexedSeed,
        .suHexedDistortionStrength    = suHexedDistortionStrength,
        .suHexedShimmerStrength       = suHexedShimmerStrength,
        .shaderShrineBackground       = SFML_BASE_MOVE(shaderShrineBackground),
        .suShrineBgTime               = suShrineBgTime,
        .suShrineBgViewOrigin         = suShrineBgViewOrigin,
        .suShrineBgCenter             = suShrineBgCenter,
        .suShrineBgRange              = suShrineBgRange,
        .suShrineBgTintR              = suShrineBgTintR,
        .suShrineBgTintG              = suShrineBgTintG,
        .suShrineBgTintB              = suShrineBgTintB,
        .suShrineBgTintA              = suShrineBgTintA,
        .suShrineBgDistortionStrength = suShrineBgDistortionStrength,
        .suShrineBgTintStrength       = suShrineBgTintStrength,
        .suShrineBgEffectStrength     = suShrineBgEffectStrength,
    };
}()),
    shaders(*shadersStorage),
    aaLevel(sf::base::min(16u, sf::RenderTexture::getMaximumAntiAliasingLevel())),
    gameStorage(new MainGameStorage{
        .profile =
            [&]
{
    Profile out;

    if (sf::Path{"userdata/profile.json"}.exists())
    {
        loadProfileFromFile(out);
        sf::base::printLn("Loaded profile from file on startup");
    }

    return out;
}(),
        .gameConstants =
            [&]
{
    GameConstants out;

    if (sf::Path{"resources/game_constants.json"}.exists())
    {
        loadGameConstantsFromFile(out);
        sf::base::printLn("Loaded game constants from file on startup");
    }

    return out;
}(),
        .ptMain     = {},
        .ptSpeedrun = {},
    }),
    profile(gameStorage->profile),
    gameConstants(gameStorage->gameConstants),
    fontMouldyCheeseStorage(new sf::Font{sf::Font::openFromFile("resources/fredoka.ttf").value()}),
    fontMouldyCheese(*fontMouldyCheeseStorage),
    windowStorage(new sf::RenderWindow{makeWindow()}),
    window(*windowStorage),
    loadingGuard(
        [&]
{
    refreshWindowAutoBatchModeFromProfile();
    window.clear(sf::Color::Black);

    window.draw(fontMouldyCheese,
                sf::TextUtils::anchored(fontMouldyCheese,
                                        sf::TextData{.position         = window.getSize().toVec2f() / 2.f,
                                                     .string           = "Loading...",
                                                     .characterSize    = 48u,
                                                     .fillColor        = sf::Color::White,
                                                     .outlineColor     = colorBlueOutline,
                                                     .outlineThickness = 2.f},
                                        {0.5f, 0.5f}));

    window.display();
    return true;
}()),
    imGuiContextStorage(new sf::ImGuiContext{/* loadDefaultFont */ false}),
    imGuiContext(*imGuiContextStorage),
    textureAtlasStorage(new sf::TextureAtlas{sf::Texture::create({6000u, 4096u}, {.smooth = true}).value()}),
    textureAtlas(*textureAtlasStorage),
    fontSuperBakeryStorage(new sf::Font{sf::Font::openFromFile("resources/fredoka.ttf", &textureAtlas).value()}),
    fontSuperBakery(*fontSuperBakeryStorage),
    fontImGuiMouldyCheese(ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fredoka.ttf", 28.f)),
    fontImGuiSuperBakery(ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fredoka.ttf", 28.f)),
    fontImGuiFA(
        []
{
    static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};

    ImFontConfig iconConfig;
    iconConfig.MergeMode        = true;
    iconConfig.PixelSnapH       = true;
    iconConfig.GlyphMinAdvanceX = 18.0f; // Helps keep icons square
    iconConfig.GlyphOffset.y    = -1.0f; // Moves icons down 2 pixels

    // 3. Load FontAwesome into the SAME font object
    auto* res = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fa-solid-900.ttf", 16.0f, &iconConfig, iconRanges);
    SFML_BASE_ASSERT(res != nullptr);

    ImGui::GetIO().Fonts->Build();
    return res;
}()),
    bgmStorage(new MainBGMStorage{}),
    bgm(*bgmStorage),
    soundManagerStorage(new SoundManager{}),
    soundManager(*soundManagerStorage),
    listenerStorage(new sf::Listener{}),
    listener(*listenerStorage),
    rtBackgroundStorage(new sf::RenderTexture{
        sf::RenderTexture::create(gameScreenSize.toVec2u(),
                                  {.antiAliasingLevel = aaLevel, .smooth = true, .wrapMode = sf::TextureWrapMode::Repeat})
            .value()}),
    rtBackground(*rtBackgroundStorage),
    rtBackgroundProcessedStorage(new sf::RenderTexture{
        sf::RenderTexture::create(gameScreenSize.toVec2u(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()}),
    rtBackgroundProcessed(*rtBackgroundProcessedStorage),
    rtImGuiStorage(new sf::RenderTexture{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()}),
    rtImGui(*rtImGuiStorage),
    rtCloudMaskStorage(new sf::RenderTexture{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()}),
    rtCloudMask(*rtCloudMaskStorage),
    rtCloudProcessedStorage(new sf::RenderTexture{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()}),
    rtCloudProcessed(*rtCloudProcessedStorage),
    rtGameStorage(new sf::RenderTexture{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()}),
    rtGame(*rtGameStorage),
    hexedCatRenderTexturesStorage(new MainRenderTextureVector{[this]
{
    MainRenderTextureVector result;
    result.reserve(maxHexedCatRenderTextures);

    for (sf::base::SizeT i = 0u; i < maxHexedCatRenderTextures; ++i)
        result.emplaceBack(
            sf::RenderTexture::create(hexedCatRenderTextureSize, {.antiAliasingLevel = aaLevel, .smooth = true}).value());

    return result;
}()}),
    hexedCatRenderTextures(*hexedCatRenderTexturesStorage),
    textureStorage(new MainTextureStorage{}),
    txLogo(textureStorage->txLogo),
    txFixedBg(textureStorage->txFixedBg),
    txBackgroundChunk(textureStorage->txBackgroundChunk),
    txBackgroundChunkDesaturated(textureStorage->txBackgroundChunkDesaturated),
    txClouds(textureStorage->txClouds),
    txTintedClouds(textureStorage->txTintedClouds),
    txBgSwamp(textureStorage->txBgSwamp),
    txBgObservatory(textureStorage->txBgObservatory),
    txBgAimTraining(textureStorage->txBgAimTraining),
    txBgFactory(textureStorage->txBgFactory),
    txBgWindTunnel(textureStorage->txBgWindTunnel),
    txBgMagnetosphere(textureStorage->txBgMagnetosphere),
    txBgAuditorium(textureStorage->txBgAuditorium),
    txDrawings(textureStorage->txDrawings),
    txTipBg(textureStorage->txTipBg),
    txTipByte(textureStorage->txTipByte),
    txCursor(textureStorage->txCursor),
    txCursorMultipop(textureStorage->txCursorMultipop),
    txCursorLaser(textureStorage->txCursorLaser),
    txCursorGrab(textureStorage->txCursorGrab),
    txArrow(textureStorage->txArrow),
    txUnlock(textureStorage->txUnlock),
    txPurchasable(textureStorage->txPurchasable),
    txLetter(textureStorage->txLetter),
    txLetterText(textureStorage->txLetterText),
    txFrame(textureStorage->txFrame),
    txFrameTiny(textureStorage->txFrameTiny),
    txCloudBtn(textureStorage->txCloudBtn),
    txCloudBtnSmall(textureStorage->txCloudBtnSmall),
    txCloudBtnSquare(textureStorage->txCloudBtnSquare),
    txCloudBtnSquare2(textureStorage->txCloudBtnSquare2),
    uiTextureAtlasStorage(new sf::TextureAtlas{sf::Texture::create({2048u, 1024u}, {.smooth = true}).value()}),
    uiTextureAtlas(*uiTextureAtlasStorage),
    atlasRectsStorage(new MainAtlasRects{
        .txrIconVolume          = addImgResourceToUIAtlas("iconvolumeon.png"),
        .txrIconBGM             = addImgResourceToUIAtlas("iconmusicon.png"),
        .txrIconBg              = addImgResourceToUIAtlas("iconbg.png"),
        .txrIconCfg             = addImgResourceToUIAtlas("iconcfg.png"),
        .txrIconCopyCat         = addImgResourceToUIAtlas("iconcopycat.png"),
        .txrMenuSeparator0      = addImgResourceToUIAtlas("menuseparator0.png"),
        .txrMenuSeparator1      = addImgResourceToUIAtlas("menuseparator1.png"),
        .txrMenuSeparator2      = addImgResourceToUIAtlas("menuseparator2.png"),
        .txrMenuSeparator3      = addImgResourceToUIAtlas("menuseparator3.png"),
        .txrMenuSeparator4      = addImgResourceToUIAtlas("menuseparator4.png"),
        .txrMenuSeparator5      = addImgResourceToUIAtlas("menuseparator5.png"),
        .txrMenuSeparator6      = addImgResourceToUIAtlas("menuseparator6.png"),
        .txrMenuSeparator7      = addImgResourceToUIAtlas("menuseparator7.png"),
        .txrMenuSeparator8      = addImgResourceToUIAtlas("menuseparator8.png"),
        .txrPrestigeSeparator0  = addImgResourceToUIAtlas("prestigeseparator0.png"),
        .txrPrestigeSeparator1  = addImgResourceToUIAtlas("prestigeseparator1.png"),
        .txrPrestigeSeparator2  = addImgResourceToUIAtlas("prestigeseparator2.png"),
        .txrPrestigeSeparator3  = addImgResourceToUIAtlas("prestigeseparator3.png"),
        .txrPrestigeSeparator4  = addImgResourceToUIAtlas("prestigeseparator4.png"),
        .txrPrestigeSeparator5  = addImgResourceToUIAtlas("prestigeseparator5.png"),
        .txrPrestigeSeparator6  = addImgResourceToUIAtlas("prestigeseparator6.png"),
        .txrPrestigeSeparator7  = addImgResourceToUIAtlas("prestigeseparator7.png"),
        .txrPrestigeSeparator8  = addImgResourceToUIAtlas("prestigeseparator8.png"),
        .txrPrestigeSeparator9  = addImgResourceToUIAtlas("prestigeseparator9.png"),
        .txrPrestigeSeparator10 = addImgResourceToUIAtlas("prestigeseparator10.png"),
        .txrPrestigeSeparator11 = addImgResourceToUIAtlas("prestigeseparator11.png"),
        .txrPrestigeSeparator12 = addImgResourceToUIAtlas("prestigeseparator12.png"),
        .txrPrestigeSeparator13 = addImgResourceToUIAtlas("prestigeseparator13.png"),
        .txrPrestigeSeparator14 = addImgResourceToUIAtlas("prestigeseparator14.png"),
        .txrPrestigeSeparator15 = addImgResourceToUIAtlas("prestigeseparator15.png"),
        .txrMagicSeparator0     = addImgResourceToUIAtlas("magicseparator0.png"),
        .txrMagicSeparator1     = addImgResourceToUIAtlas("magicseparator1.png"),
        .txrMagicSeparator2     = addImgResourceToUIAtlas("magicseparator2.png"),
        .txrMagicSeparator3     = addImgResourceToUIAtlas("magicseparator3.png"),
        .txrWhiteDot            = textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value(),
        .txrBubble              = addImgResourceToAtlas("bubble2.png"),
        .txrBubbleStar          = addImgResourceToAtlas("bubble3.png"),
        .txrBubbleNova          = addImgResourceToAtlas("bubble4.png"),
        .txrBubbleGlass         = addImgResourceToAtlas("bubbleglass.png"),
        .txrCat                 = addImgResourceToAtlas("cat.png"),
        .txrGuardhouseBack      = addImgResourceToAtlas("guardhouse_back.png"),
        .txrWardenCat           = addImgResourceToAtlas("wardencat.png"),
        .txrGuardhouseFront     = addImgResourceToAtlas("guardhouse_front.png"),
        .txrWardencatPaw        = addImgResourceToAtlas("wardencatpaw.png"),
        .txrUniCat              = addImgResourceToAtlas("unicat3.png"),
        .txrUniCat2             = addImgResourceToAtlas("unicat2.png"),
        .txrUniCatWings         = addImgResourceToAtlas("unicatwings.png"),
        .txrDevilCat2           = addImgResourceToAtlas("devilcat2.png"),
        .txrDevilCat3           = addImgResourceToAtlas("devilcat3.png"),
        .txrDevilCat3Arm        = addImgResourceToAtlas("devilcat3arm.png"),
        .txrDevilCat3Book       = addImgResourceToAtlas("devilcat3book.png"),
        .txrDevilCat3Tail       = addImgResourceToAtlas("devilcat3tail.png"),
        .txrDevilCat2Book       = addImgResourceToAtlas("devilcat2book.png"),
        .txrCatPaw              = addImgResourceToAtlas("catpaw.png"),
        .txrCatTail             = addImgResourceToAtlas("cattail.png"),
        .txrSmartCatHat         = addImgResourceToAtlas("smartcathat.png"),
        .txrSmartCatDiploma     = addImgResourceToAtlas("smartcatdiploma.png"),
        .txrBrainBack           = addImgResourceToAtlas("brainback.png"),
        .txrBrainFront          = addImgResourceToAtlas("brainfront.png"),
        .txrUniCatTail          = addImgResourceToAtlas("unicattail.png"),
        .txrUniCat2Tail         = addImgResourceToAtlas("unicat2tail.png"),
        .txrDevilCatTail2       = addImgResourceToAtlas("devilcattail2.png"),
        .txrAstroCatTail        = addImgResourceToAtlas("astrocattail.png"),
        .txrAstroCatFlag        = addImgResourceToAtlas("astrocatflag.png"),
        .txrWitchCatTail        = addImgResourceToAtlas("witchcattail.png"),
        .txrWizardCatTail       = addImgResourceToAtlas("wizardcattail.png"),
        .txrMouseCatTail        = addImgResourceToAtlas("mousecattail.png"),
        .txrMouseCatMouse       = addImgResourceToAtlas("mousecatmouse.png"),
        .txrEngiCatTail         = addImgResourceToAtlas("engicattail.png"),
        .txrEngiCatWrench       = addImgResourceToAtlas("engicatwrench.png"),
        .txrRepulsoCatTail      = addImgResourceToAtlas("repulsocattail.png"),
        .txrAttractoCatTail     = addImgResourceToAtlas("attractocattail.png"),
        .txrCopyCatTail         = addImgResourceToAtlas("copycattail.png"),
        .txrAttractoCatMagnet   = addImgResourceToAtlas("attractocatmagnet.png"),
        .txrUniCatPaw           = addImgResourceToAtlas("unicatpaw.png"),
        .txrDevilCatPaw         = addImgResourceToAtlas("devilcatpaw.png"),
        .txrDevilCatPaw2        = addImgResourceToAtlas("devilcatpaw2.png"),
        .txrParticle            = addImgResourceToAtlas("particle.png"),
        .txrStarParticle        = addImgResourceToAtlas("starparticle.png"),
        .txrFireParticle        = addImgResourceToAtlas("fireparticle.png"),
        .txrFireParticle2       = addImgResourceToAtlas("fireparticle2.png"),
        .txrSmokeParticle       = addImgResourceToAtlas("smokeparticle.png"),
        .txrExplosionParticle   = addImgResourceToAtlas("explosionparticle.png"),
        .txrTrailParticle       = addImgResourceToAtlas("trailparticle.png"),
        .txrHexParticle         = addImgResourceToAtlas("hexparticle.png"),
        .txrShrineParticle      = addImgResourceToAtlas("shrineparticle.png"),
        .txrCogParticle         = addImgResourceToAtlas("cogparticle.png"),
        .txrGlassParticle       = addImgResourceToAtlas("glassparticle.png"),
        .txrWitchCat            = addImgResourceToAtlas("witchcat.png"),
        .txrWitchCatPaw         = addImgResourceToAtlas("witchcatpaw.png"),
        .txrAstroCat            = addImgResourceToAtlas("astromeow.png"),
        .txrBomb                = addImgResourceToAtlas("bomb.png"),
        .txrShrine              = addImgResourceToAtlas("shrine.png"),
        .txrWizardCat           = addImgResourceToAtlas("wizardcat.png"),
        .txrWizardCatPaw        = addImgResourceToAtlas("wizardcatpaw.png"),
        .txrMouseCat            = addImgResourceToAtlas("mousecat.png"),
        .txrMouseCatPaw         = addImgResourceToAtlas("mousecatpaw.png"),
        .txrEngiCat             = addImgResourceToAtlas("engicat.png"),
        .txrEngiCatPaw          = addImgResourceToAtlas("engicatpaw.png"),
        .txrRepulsoCat          = addImgResourceToAtlas("repulsocat.png"),
        .txrRepulsoCatPaw       = addImgResourceToAtlas("repulsocatpaw.png"),
        .txrAttractoCat         = addImgResourceToAtlas("attractocat.png"),
        .txrCopyCat             = addImgResourceToAtlas("copycat.png"),
        .txrDuckCat             = addImgResourceToAtlas("duck.png"),
        .txrDuckFlag            = addImgResourceToAtlas("duckflag.png"),
        .txrAttractoCatPaw      = addImgResourceToAtlas("attractocatpaw.png"),
        .txrCopyCatPaw          = addImgResourceToAtlas("copycatpaw.png"),
        .txrDollNormal          = addImgResourceToAtlas("dollnormal.png"),
        .txrDollUni             = addImgResourceToAtlas("dolluni.png"),
        .txrDollDevil           = addImgResourceToAtlas("dolldevil.png"),
        .txrDollAstro           = addImgResourceToAtlas("dollastro.png"),
        .txrDollWizard          = addImgResourceToAtlas("dollwizard.png"),
        .txrDollMouse           = addImgResourceToAtlas("dollmouse.png"),
        .txrDollEngi            = addImgResourceToAtlas("dollengi.png"),
        .txrDollRepulso         = addImgResourceToAtlas("dollrepulso.png"),
        .txrDollAttracto        = addImgResourceToAtlas("dollattracto.png"),
        .txrCoin                = addImgResourceToAtlas("bytecoin.png"),
        .txrCatSoul             = addImgResourceToAtlas("catsoul.png"),
        .txrHellPortal          = addImgResourceToAtlas("hellportal.png"),
        .txrCatEyeLid0          = addImgResourceToAtlas("cateyelid0.png"),
        .txrCatEyeLid1          = addImgResourceToAtlas("cateyelid1.png"),
        .txrCatEyeLid2          = addImgResourceToAtlas("cateyelid2.png"),
        .txrCatWhiteEyeLid0     = addImgResourceToAtlas("catwhiteeyelid0.png"),
        .txrCatWhiteEyeLid1     = addImgResourceToAtlas("catwhiteeyelid1.png"),
        .txrCatWhiteEyeLid2     = addImgResourceToAtlas("catwhiteeyelid2.png"),
        .txrCatDarkEyeLid0      = addImgResourceToAtlas("catdarkeyelid0.png"),
        .txrCatDarkEyeLid1      = addImgResourceToAtlas("catdarkeyelid1.png"),
        .txrCatDarkEyeLid2      = addImgResourceToAtlas("catdarkeyelid2.png"),
        .txrCatGrayEyeLid0      = addImgResourceToAtlas("catgrayeyelid0.png"),
        .txrCatGrayEyeLid1      = addImgResourceToAtlas("catgrayeyelid1.png"),
        .txrCatGrayEyeLid2      = addImgResourceToAtlas("catgrayeyelid2.png"),
        .txrCatEars0            = addImgResourceToAtlas("catears0.png"),
        .txrCatEars1            = addImgResourceToAtlas("catears1.png"),
        .txrCatEars2            = addImgResourceToAtlas("catears2.png"),
        .txrCatYawn0            = addImgResourceToAtlas("catyawn0.png"),
        .txrCatYawn1            = addImgResourceToAtlas("catyawn1.png"),
        .txrCatYawn2            = addImgResourceToAtlas("catyawn2.png"),
        .txrCatYawn3            = addImgResourceToAtlas("catyawn3.png"),
        .txrCatYawn4            = addImgResourceToAtlas("catyawn4.png"),
        .txrCCMaskWitch         = addImgResourceToAtlas("ccmaskwitch.png"),
        .txrCCMaskWizard        = addImgResourceToAtlas("ccmaskwizard.png"),
        .txrCCMaskMouse         = addImgResourceToAtlas("ccmaskmouse.png"),
        .txrCCMaskEngi          = addImgResourceToAtlas("ccmaskengi.png"),
        .txrCCMaskRepulso       = addImgResourceToAtlas("ccmaskrepulso.png"),
        .txrCCMaskAttracto      = addImgResourceToAtlas("ccmaskattracto.png"),
        .txrMMNormal            = addImgResourceToAtlas("mmcatnormal.png"),
        .txrMMUni               = addImgResourceToAtlas("mmcatuni.png"),
        .txrMMDevil             = addImgResourceToAtlas("mmcatdevil.png"),
        .txrMMAstro             = addImgResourceToAtlas("mmcatastro.png"),
        .txrMMWitch             = addImgResourceToAtlas("mmcatwitch.png"),
        .txrMMWizard            = addImgResourceToAtlas("mmcatwizard.png"),
        .txrMMMouse             = addImgResourceToAtlas("mmcatmouse.png"),
        .txrMMEngi              = addImgResourceToAtlas("mmcatengi.png"),
        .txrMMRepulso           = addImgResourceToAtlas("mmcatrepulso.png"),
        .txrMMAttracto          = addImgResourceToAtlas("mmcatattracto.png"),
        .txrMMCopy              = addImgResourceToAtlas("mmcatcopy.png"),
        .txrMMDuck              = addImgResourceToAtlas("mmduck.png"),
        .txrMMShrine            = addImgResourceToAtlas("mmshrine.png"),
        .txrCloud               = addImgResourceToAtlas("cloud.png"),
    }),
    atlasRects(*atlasRectsStorage),
    ptMain(gameStorage->ptMain),
    ptSpeedrun(gameStorage->ptSpeedrun),
    pt(&ptMain),
    textStorage(new MainTextStorage{fontSuperBakery}),
    moneyText(textStorage->moneyText),
    comboStateStorage(new ComboState{moneyTextInitialPosition}),
    comboState(*comboStateStorage),
    demoText(textStorage->demoText),
    sweepAndPrune(sf::base::makeUnique<SweepAndPrune>()),
    seed(static_cast<RNGSeedType>(sf::Clock::now().asMicroseconds())),
    shuffledCatNamesPerType(makeShuffledCatNames(rng)),
    drawableBatchesStorage(new MainDrawableBatches{}),
    bubbleDrawableBatch(drawableBatchesStorage->bubbleDrawableBatch),
    starBubbleDrawableBatch(drawableBatchesStorage->starBubbleDrawableBatch),
    bombBubbleDrawableBatch(drawableBatchesStorage->bombBubbleDrawableBatch),
    cpuCloudDrawableBatch(drawableBatchesStorage->cpuCloudDrawableBatch),
    cpuTopCloudDrawableBatch(drawableBatchesStorage->cpuTopCloudDrawableBatch),
    cpuCloudHudDrawableBatch(drawableBatchesStorage->cpuCloudHudDrawableBatch),
    cpuCloudUiDrawableBatch(drawableBatchesStorage->cpuCloudUiDrawableBatch),
    cpuDrawableBatchBeforeCats(drawableBatchesStorage->cpuDrawableBatchBeforeCats),
    cpuDrawableBatch(drawableBatchesStorage->cpuDrawableBatch),
    cpuDrawableBatchAfterCats(drawableBatchesStorage->cpuDrawableBatchAfterCats),
    cpuDrawableBatchAdditive(drawableBatchesStorage->cpuDrawableBatchAdditive),
    minimapDrawableBatch(drawableBatchesStorage->minimapDrawableBatch),
    catTextDrawableBatch(drawableBatchesStorage->catTextDrawableBatch),
    hudDrawableBatch(drawableBatchesStorage->hudDrawableBatch),
    hudTopDrawableBatch(drawableBatchesStorage->hudTopDrawableBatch),
    hudBottomDrawableBatch(drawableBatchesStorage->hudBottomDrawableBatch),
    cpuTopDrawableBatch(drawableBatchesStorage->cpuTopDrawableBatch),
    catTextTopDrawableBatch(drawableBatchesStorage->catTextTopDrawableBatch),
    tempDrawableBatch(drawableBatchesStorage->tempDrawableBatch),
    textNameBuffer(textStorage->textNameBuffer),
    textStatusBuffer(textStorage->textStatusBuffer),
    textMoneyBuffer(textStorage->textMoneyBuffer),
    threadPool(getTPWorkerCount()),
    gameViewStorage(new sf::View{.center = {1.f, 1.f}, .size = {1.f, 1.f}}),
    gameView(*gameViewStorage),
    nonScaledHUDViewStorage(new sf::View{.center = {1.f, 1.f}, .size = {1.f, 1.f}}),
    nonScaledHUDView(*nonScaledHUDViewStorage),
    scaledHUDViewStorage(new sf::View{.center = {1.f, 1.f}, .size = {1.f, 1.f}}),
    scaledHUDView(*scaledHUDViewStorage),
    bombStorage(new MainBombStorage{}),
    notificationStateStorage(new NotificationState{}),
    notificationState(*notificationStateStorage),
    uiStateStorage(new UIState{}),
    uiState(*uiStateStorage),
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    steamMgr(xSteamMgr),
    onSteamDeck(steamMgr.isOnSteamDeck()),
#else
    onSteamDeck(false),
#endif
    inputHelperStorage(new InputHelper{}),
    inputHelper(*inputHelperStorage),
    logFileStorage(
        []
{
    auto opt = sf::OutFile::open("bubblebyte.log", sf::FileOpenMode::out | sf::FileOpenMode::app);
    return opt.hasValue() ? MainOwnedPtr<sf::OutFile>(new sf::OutFile(SFML_BASE_MOVE(*opt))) : MainOwnedPtr<sf::OutFile>{};
}()),
    logFile(logFileStorage.get())
{
    sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

    if (onSteamDeck)
    {
        // borderless windowed
        profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
        profile.windowed = true;

        profile.uiScale = 1.25f;
    }

    //
    // Playthrough
    if (sf::Path{"userdata/playthrough.json"}.exists())
    {
        loadPlaythroughFromFileAndReseed();
        sf::base::printLn("Loaded playthrough from file on startup");
    }
    else
    {
        pt->seed = seed;
    }

    //
    // Reserve memory
    particles.reserve(512);
    spentCoinParticles.reserve(512);
    textParticles.reserve(256);
    pt->bubbles.reserve(32'768);
    pt->cats.reserve(512);

    //
    // Touch state
    playerInputState.fingerPositions.resize(10);
}

////////////////////////////////////////////////////////////
Main::~Main()
{
    sf::base::printLn("Saving playthrough to file on exit");
    saveMainPlaythroughToFile();

    sf::base::printLn("Saving profile to file on exit");
    saveProfileToFile(profile);
}

////////////////////////////////////////////////////////////
void Main::run()
{
    //
    // Startup (splash screen and meow)
    splashCountdown.restart();
    playSound(sounds.byteMeow);

    //
    //
    // Background music
    auto& [entries, selectedIndex] = getBGMSelectorData();
    selectBGM(entries, selectedIndex);
    switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ true);

    //
    // Game loop
    playedClock.start();

    while (true)
        if (!gameLoop())
            return;
}
