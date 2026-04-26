

#include "BubbleIdleApp.hpp"
#include "BubbleIdleMain.hpp"
#include "Constants.hpp"
#include "GameConstants.hpp"
#include "IconsFontAwesome6.h"
#include "Profile.hpp"
#include "RNGSeedType.hpp"
#include "Serialization.hpp"
#include "Steam.hpp"
#include "SweepAndPrune.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Priv/ShaderBase.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"

#include "SFML/Audio/AudioContext.hpp"

#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <imgui.h>

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
    audioContext(sf::AudioContext::create().value()),
    playbackDevice(sf::AudioContext::getDefaultPlaybackDeviceHandle().value()),
#endif
    graphicsContext(sf::GraphicsContext::create().value()),
    shader(
        []
{
    auto result = sf::Shader::loadFromFile({.vertexPath = "resources/shader.vert", .fragmentPath = "resources/shader.frag"})
                      .value();
    result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
    return result;
}()),
    suBackgroundTexture(shader.getUniformLocation("u_backgroundTexture").value()),
    suTime(shader.getUniformLocation("u_time").value()),
    suResolution(shader.getUniformLocation("u_resolution").value()),
    suBackgroundOrigin(shader.getUniformLocation("u_backgroundOrigin").value()),
    suBubbleEffect(shader.getUniformLocation("u_bubbleEffect").value()),
    suIridescenceStrength(shader.getUniformLocation("u_iridescenceStrength").value()),
    suEdgeFactorMin(shader.getUniformLocation("u_edgeFactorMin").value()),
    suEdgeFactorMax(shader.getUniformLocation("u_edgeFactorMax").value()),
    suEdgeFactorStrength(shader.getUniformLocation("u_edgeFactorStrength").value()),
    suDistorsionStrength(shader.getUniformLocation("u_distorsionStrength").value()),
    suSubTexOrigin(shader.getUniformLocation("u_subTexOrigin").value()),
    suSubTexSize(shader.getUniformLocation("u_subTexSize").value()),
    suBubbleLightness(shader.getUniformLocation("u_bubbleLightness").value()),
    suLensDistortion(shader.getUniformLocation("u_lensDistortion").value()),
    suRimShineStrength(shader.getUniformLocation("u_rimShineStrength").value()),
    suRimShineFallRate(shader.getUniformLocation("u_rimShineFallRate").value()),
    suRimShineTimeRate(shader.getUniformLocation("u_rimShineTimeRate").value()),
    suRimShineArc(shader.getUniformLocation("u_rimShineArc").value()),
    shaderPostProcess(
        []
{
    // TODO P2: (lib) add support for `#include` in shaders
    auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/postprocess.frag"}).value();
    result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
    return result;
}()),
    suPPVibrance(shaderPostProcess.getUniformLocation("u_vibrance").value()),
    suPPSaturation(shaderPostProcess.getUniformLocation("u_saturation").value()),
    suPPLightness(shaderPostProcess.getUniformLocation("u_lightness").value()),
    suPPSharpness(shaderPostProcess.getUniformLocation("u_sharpness").value()),
    suPPBlur(shaderPostProcess.getUniformLocation("u_blur").value()),
    shaderClouds(
        []
{
    auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/clouds.frag"}).value();
    result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
    return result;
}()),
    suCloudTime(shaderClouds.getUniformLocation("u_time").value()),
    suCloudResolution(shaderClouds.getUniformLocation("u_resolution").value()),
    shaderHexed(
        []
{
    auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/hexed_cat.frag"}).value();
    result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
    return result;
}()),
    suHexedTime(shaderHexed.getUniformLocation("u_time").value()),
    suHexedSeed(shaderHexed.getUniformLocation("u_seed").value()),
    suHexedDistortionStrength(shaderHexed.getUniformLocation("u_distortionStrength").value()),
    suHexedShimmerStrength(shaderHexed.getUniformLocation("u_shimmerStrength").value()),
    shaderShrineBackground(
        []
{
    auto result = sf::Shader::loadFromFile({.vertexPath   = "resources/shrine_background.vert",
                                            .fragmentPath = "resources/shrine_background.frag"})
                      .value();
    result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
    return result;
}()),
    suShrineBgTime(shaderShrineBackground.getUniformLocation("u_time").value()),
    suShrineBgViewOrigin(shaderShrineBackground.getUniformLocation("u_viewOrigin").value()),
    suShrineBgCenter(shaderShrineBackground.getUniformLocation("u_shrineCenter").value()),
    suShrineBgRange(shaderShrineBackground.getUniformLocation("u_shrineRange").value()),
    suShrineBgTintR(shaderShrineBackground.getUniformLocation("u_shrineTintR").value()),
    suShrineBgTintG(shaderShrineBackground.getUniformLocation("u_shrineTintG").value()),
    suShrineBgTintB(shaderShrineBackground.getUniformLocation("u_shrineTintB").value()),
    suShrineBgTintA(shaderShrineBackground.getUniformLocation("u_shrineTintA").value()),
    suShrineBgTintStrength(shaderShrineBackground.getUniformLocation("u_tintStrength").value()),
    suShrineBgEffectStrength(shaderShrineBackground.getUniformLocation("u_effectStrength").value()),
    aaLevel(sf::base::min(16u, sf::RenderTexture::getMaximumAntiAliasingLevel())),
    profile(
        [&]
{
    Profile out;

    if (sf::Path{"userdata/profile.json"}.exists())
    {
        loadProfileFromFile(out);
        sf::cOut() << "Loaded profile from file on startup\n";
    }

    return out;
}()),
    gameConstants(
        [&]
{
    GameConstants out;

    if (sf::Path{"resources/game_constants.json"}.exists())
    {
        loadGameConstantsFromFile(out);
        sf::cOut() << "Loaded game constants from file on startup\n";
    }

    return out;
}()),
    fontMouldyCheese(sf::Font::openFromFile("resources/fredoka.ttf").value()),
    window(makeWindow()),
    loadingGuard(
        [&]
{
    refreshWindowAutoBatchModeFromProfile();
    window.clear(sf::Color::Black);

    sf::TextData loadingTextData{.position         = window.getSize().toVec2f() / 2.f,
                                 .string           = "Loading...",
                                 .characterSize    = 48u,
                                 .fillColor        = sf::Color::White,
                                 .outlineColor     = colorBlueOutline,
                                 .outlineThickness = 2.f};

    loadingTextData.origin = sf::TextUtils::precomputeTextLocalBounds(fontMouldyCheese, loadingTextData).size / 2.f;
    window.draw(fontMouldyCheese, loadingTextData);

    window.display();
    return true;
}()),
    textureAtlas(sf::Texture::create({6000u, 4096u}, {.smooth = true}).value()),
    fontSuperBakery(sf::Font::openFromFile("resources/fredoka.ttf", &textureAtlas).value()),
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
    rtBackground(
        sf::RenderTexture::create(gameScreenSize.toVec2u(),
                                  {.antiAliasingLevel = aaLevel, .smooth = true, .wrapMode = sf::TextureWrapMode::Repeat})
            .value()),
    rtBackgroundProcessed(
        sf::RenderTexture::create(gameScreenSize.toVec2u(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()),
    rtImGui(sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()),
    rtCloudMask(sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()),
    rtCloudProcessed(sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()),
    rtGame(sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()),
    hexedCatRenderTextures(
        [this]
{
    sf::base::Vector<sf::RenderTexture> result;
    result.reserve(maxHexedCatRenderTextures);

    for (sf::base::SizeT i = 0u; i < maxHexedCatRenderTextures; ++i)
        result.emplaceBack(
            sf::RenderTexture::create(hexedCatRenderTextureSize, {.antiAliasingLevel = aaLevel, .smooth = true}).value());

    return result;
}()),
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
    txCloudBtnSquare2(sf::Texture::loadFromFile("resources/cloudbtnsquare2.png", {.smooth = true}).value()),
    uiTextureAtlas(sf::Texture::create({2048u, 1024u}, {.smooth = true}).value()),
    txrIconVolume(addImgResourceToUIAtlas("iconvolumeon.png")),
    txrIconBGM(addImgResourceToUIAtlas("iconmusicon.png")),
    txrIconBg(addImgResourceToUIAtlas("iconbg.png")),
    txrIconCfg(addImgResourceToUIAtlas("iconcfg.png")),
    txrIconCopyCat(addImgResourceToUIAtlas("iconcopycat.png")),
    txrMenuSeparator0(addImgResourceToUIAtlas("menuseparator0.png")),
    txrMenuSeparator1(addImgResourceToUIAtlas("menuseparator1.png")),
    txrMenuSeparator2(addImgResourceToUIAtlas("menuseparator2.png")),
    txrMenuSeparator3(addImgResourceToUIAtlas("menuseparator3.png")),
    txrMenuSeparator4(addImgResourceToUIAtlas("menuseparator4.png")),
    txrMenuSeparator5(addImgResourceToUIAtlas("menuseparator5.png")),
    txrMenuSeparator6(addImgResourceToUIAtlas("menuseparator6.png")),
    txrMenuSeparator7(addImgResourceToUIAtlas("menuseparator7.png")),
    txrMenuSeparator8(addImgResourceToUIAtlas("menuseparator8.png")),
    txrPrestigeSeparator0(addImgResourceToUIAtlas("prestigeseparator0.png")),
    txrPrestigeSeparator1(addImgResourceToUIAtlas("prestigeseparator1.png")),
    txrPrestigeSeparator2(addImgResourceToUIAtlas("prestigeseparator2.png")),
    txrPrestigeSeparator3(addImgResourceToUIAtlas("prestigeseparator3.png")),
    txrPrestigeSeparator4(addImgResourceToUIAtlas("prestigeseparator4.png")),
    txrPrestigeSeparator5(addImgResourceToUIAtlas("prestigeseparator5.png")),
    txrPrestigeSeparator6(addImgResourceToUIAtlas("prestigeseparator6.png")),
    txrPrestigeSeparator7(addImgResourceToUIAtlas("prestigeseparator7.png")),
    txrPrestigeSeparator8(addImgResourceToUIAtlas("prestigeseparator8.png")),
    txrPrestigeSeparator9(addImgResourceToUIAtlas("prestigeseparator9.png")),
    txrPrestigeSeparator10(addImgResourceToUIAtlas("prestigeseparator10.png")),
    txrPrestigeSeparator11(addImgResourceToUIAtlas("prestigeseparator11.png")),
    txrPrestigeSeparator12(addImgResourceToUIAtlas("prestigeseparator12.png")),
    txrPrestigeSeparator13(addImgResourceToUIAtlas("prestigeseparator13.png")),
    txrPrestigeSeparator14(addImgResourceToUIAtlas("prestigeseparator14.png")),
    txrPrestigeSeparator15(addImgResourceToUIAtlas("prestigeseparator15.png")),
    txrMagicSeparator0(addImgResourceToUIAtlas("magicseparator0.png")),
    txrMagicSeparator1(addImgResourceToUIAtlas("magicseparator1.png")),
    txrMagicSeparator2(addImgResourceToUIAtlas("magicseparator2.png")),
    txrMagicSeparator3(addImgResourceToUIAtlas("magicseparator3.png")),
    txrWhiteDot(textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value()),
    txrBubble(addImgResourceToAtlas("bubble2.png")),
    txrBubbleStar(addImgResourceToAtlas("bubble3.png")),
    txrBubbleNova(addImgResourceToAtlas("bubble4.png")),
    txrBubbleGlass(addImgResourceToAtlas("bubbleglass.png")),
    txrCat(addImgResourceToAtlas("cat.png")),
    txrGuardhouseBack(addImgResourceToAtlas("guardhouse_back.png")),
    txrWardenCat(addImgResourceToAtlas("wardencat.png")),
    txrGuardhouseFront(addImgResourceToAtlas("guardhouse_front.png")),
    txrWardencatPaw(addImgResourceToAtlas("wardencatpaw.png")),
    txrUniCat(addImgResourceToAtlas("unicat3.png")),
    txrUniCat2(addImgResourceToAtlas("unicat2.png")),
    txrUniCatWings(addImgResourceToAtlas("unicatwings.png")),
    txrDevilCat2(addImgResourceToAtlas("devilcat2.png")),
    txrDevilCat3(addImgResourceToAtlas("devilcat3.png")),
    txrDevilCat3Arm(addImgResourceToAtlas("devilcat3arm.png")),
    txrDevilCat3Book(addImgResourceToAtlas("devilcat3book.png")),
    txrDevilCat3Tail(addImgResourceToAtlas("devilcat3tail.png")),
    txrDevilCat2Book(addImgResourceToAtlas("devilcat2book.png")),
    txrCatPaw(addImgResourceToAtlas("catpaw.png")),
    txrCatTail(addImgResourceToAtlas("cattail.png")),
    txrSmartCatHat(addImgResourceToAtlas("smartcathat.png")),
    txrSmartCatDiploma(addImgResourceToAtlas("smartcatdiploma.png")),
    txrBrainBack(addImgResourceToAtlas("brainback.png")),
    txrBrainFront(addImgResourceToAtlas("brainfront.png")),
    txrUniCatTail(addImgResourceToAtlas("unicattail.png")),
    txrUniCat2Tail(addImgResourceToAtlas("unicat2tail.png")),
    txrDevilCatTail2(addImgResourceToAtlas("devilcattail2.png")),
    txrAstroCatTail(addImgResourceToAtlas("astrocattail.png")),
    txrAstroCatFlag(addImgResourceToAtlas("astrocatflag.png")),
    txrWitchCatTail(addImgResourceToAtlas("witchcattail.png")),
    txrWizardCatTail(addImgResourceToAtlas("wizardcattail.png")),
    txrMouseCatTail(addImgResourceToAtlas("mousecattail.png")),
    txrMouseCatMouse(addImgResourceToAtlas("mousecatmouse.png")),
    txrEngiCatTail(addImgResourceToAtlas("engicattail.png")),
    txrEngiCatWrench(addImgResourceToAtlas("engicatwrench.png")),
    txrRepulsoCatTail(addImgResourceToAtlas("repulsocattail.png")),
    txrAttractoCatTail(addImgResourceToAtlas("attractocattail.png")),
    txrCopyCatTail(addImgResourceToAtlas("copycattail.png")),
    txrAttractoCatMagnet(addImgResourceToAtlas("attractocatmagnet.png")),
    txrUniCatPaw(addImgResourceToAtlas("unicatpaw.png")),
    txrDevilCatPaw(addImgResourceToAtlas("devilcatpaw.png")),
    txrDevilCatPaw2(addImgResourceToAtlas("devilcatpaw2.png")),
    txrParticle(addImgResourceToAtlas("particle.png")),
    txrStarParticle(addImgResourceToAtlas("starparticle.png")),
    txrFireParticle(addImgResourceToAtlas("fireparticle.png")),
    txrFireParticle2(addImgResourceToAtlas("fireparticle2.png")),
    txrSmokeParticle(addImgResourceToAtlas("smokeparticle.png")),
    txrExplosionParticle(addImgResourceToAtlas("explosionparticle.png")),
    txrTrailParticle(addImgResourceToAtlas("trailparticle.png")),
    txrHexParticle(addImgResourceToAtlas("hexparticle.png")),
    txrShrineParticle(addImgResourceToAtlas("shrineparticle.png")),
    txrCogParticle(addImgResourceToAtlas("cogparticle.png")),
    txrGlassParticle(addImgResourceToAtlas("glassparticle.png")),
    txrWitchCat(addImgResourceToAtlas("witchcat.png")),
    txrWitchCatPaw(addImgResourceToAtlas("witchcatpaw.png")),
    txrAstroCat(addImgResourceToAtlas("astromeow.png")),
    txrBomb(addImgResourceToAtlas("bomb.png")),
    txrShrine(addImgResourceToAtlas("shrine.png")),
    txrWizardCat(addImgResourceToAtlas("wizardcat.png")),
    txrWizardCatPaw(addImgResourceToAtlas("wizardcatpaw.png")),
    txrMouseCat(addImgResourceToAtlas("mousecat.png")),
    txrMouseCatPaw(addImgResourceToAtlas("mousecatpaw.png")),
    txrEngiCat(addImgResourceToAtlas("engicat.png")),
    txrEngiCatPaw(addImgResourceToAtlas("engicatpaw.png")),
    txrRepulsoCat(addImgResourceToAtlas("repulsocat.png")),
    txrRepulsoCatPaw(addImgResourceToAtlas("repulsocatpaw.png")),
    txrAttractoCat(addImgResourceToAtlas("attractocat.png")),
    txrCopyCat(addImgResourceToAtlas("copycat.png")),
    txrDuckCat(addImgResourceToAtlas("duck.png")),
    txrDuckFlag(addImgResourceToAtlas("duckflag.png")),
    txrAttractoCatPaw(addImgResourceToAtlas("attractocatpaw.png")),
    txrCopyCatPaw(addImgResourceToAtlas("copycatpaw.png")),
    txrDollNormal(addImgResourceToAtlas("dollnormal.png")),
    txrDollUni(addImgResourceToAtlas("dolluni.png")),
    txrDollDevil(addImgResourceToAtlas("dolldevil.png")),
    txrDollAstro(addImgResourceToAtlas("dollastro.png")),
    txrDollWizard(addImgResourceToAtlas("dollwizard.png")),
    txrDollMouse(addImgResourceToAtlas("dollmouse.png")),
    txrDollEngi(addImgResourceToAtlas("dollengi.png")),
    txrDollRepulso(addImgResourceToAtlas("dollrepulso.png")),
    txrDollAttracto(addImgResourceToAtlas("dollattracto.png")),
    txrCoin(addImgResourceToAtlas("bytecoin.png")),
    txrCatSoul(addImgResourceToAtlas("catsoul.png")),
    txrHellPortal(addImgResourceToAtlas("hellportal.png")),
    txrCatEyeLid0(addImgResourceToAtlas("cateyelid0.png")),
    txrCatEyeLid1(addImgResourceToAtlas("cateyelid1.png")),
    txrCatEyeLid2(addImgResourceToAtlas("cateyelid2.png")),
    txrCatWhiteEyeLid0(addImgResourceToAtlas("catwhiteeyelid0.png")),
    txrCatWhiteEyeLid1(addImgResourceToAtlas("catwhiteeyelid1.png")),
    txrCatWhiteEyeLid2(addImgResourceToAtlas("catwhiteeyelid2.png")),
    txrCatDarkEyeLid0(addImgResourceToAtlas("catdarkeyelid0.png")),
    txrCatDarkEyeLid1(addImgResourceToAtlas("catdarkeyelid1.png")),
    txrCatDarkEyeLid2(addImgResourceToAtlas("catdarkeyelid2.png")),
    txrCatGrayEyeLid0(addImgResourceToAtlas("catgrayeyelid0.png")),
    txrCatGrayEyeLid1(addImgResourceToAtlas("catgrayeyelid1.png")),
    txrCatGrayEyeLid2(addImgResourceToAtlas("catgrayeyelid2.png")),
    txrCatEars0(addImgResourceToAtlas("catears0.png")),
    txrCatEars1(addImgResourceToAtlas("catears1.png")),
    txrCatEars2(addImgResourceToAtlas("catears2.png")),
    txrCatYawn0(addImgResourceToAtlas("catyawn0.png")),
    txrCatYawn1(addImgResourceToAtlas("catyawn1.png")),
    txrCatYawn2(addImgResourceToAtlas("catyawn2.png")),
    txrCatYawn3(addImgResourceToAtlas("catyawn3.png")),
    txrCatYawn4(addImgResourceToAtlas("catyawn4.png")),
    txrCCMaskWitch(addImgResourceToAtlas("ccmaskwitch.png")),
    txrCCMaskWizard(addImgResourceToAtlas("ccmaskwizard.png")),
    txrCCMaskMouse(addImgResourceToAtlas("ccmaskmouse.png")),
    txrCCMaskEngi(addImgResourceToAtlas("ccmaskengi.png")),
    txrCCMaskRepulso(addImgResourceToAtlas("ccmaskrepulso.png")),
    txrCCMaskAttracto(addImgResourceToAtlas("ccmaskattracto.png")),
    txrMMNormal(addImgResourceToAtlas("mmcatnormal.png")),
    txrMMUni(addImgResourceToAtlas("mmcatuni.png")),
    txrMMDevil(addImgResourceToAtlas("mmcatdevil.png")),
    txrMMAstro(addImgResourceToAtlas("mmcatastro.png")),
    txrMMWitch(addImgResourceToAtlas("mmcatwitch.png")),
    txrMMWizard(addImgResourceToAtlas("mmcatwizard.png")),
    txrMMMouse(addImgResourceToAtlas("mmcatmouse.png")),
    txrMMEngi(addImgResourceToAtlas("mmcatengi.png")),
    txrMMRepulso(addImgResourceToAtlas("mmcatrepulso.png")),
    txrMMAttracto(addImgResourceToAtlas("mmcatattracto.png")),
    txrMMCopy(addImgResourceToAtlas("mmcatcopy.png")),
    txrMMDuck(addImgResourceToAtlas("mmduck.png")),
    txrMMShrine(addImgResourceToAtlas("mmshrine.png")),
    txrCloud(addImgResourceToAtlas("cloud.png")),
    sweepAndPrune(sf::base::makeUnique<SweepAndPrune>()),
    seed(static_cast<RNGSeedType>(sf::Clock::now().asMicroseconds())),
    shuffledCatNamesPerType(makeShuffledCatNames(rng)),
    threadPool(getTPWorkerCount()),
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    steamMgr(xSteamMgr),
    onSteamDeck(steamMgr.isOnSteamDeck())
#else
    onSteamDeck(false)
#endif
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
        sf::cOut() << "Loaded playthrough from file on startup\n";
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
