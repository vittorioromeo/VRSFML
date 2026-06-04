#include "ShowcaseExample.hpp"
#include "ShowcaseTextComparison.hpp"

#include "Zancle/ImGui/IncludeImGui.hpp"

#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GlyphMappedText.hpp"
#include "Zancle/Graphics/RenderTarget.hpp"
#include "Zancle/Graphics/TextData.hpp"
#include "Zancle/Graphics/Texture.hpp"

#include "Zancle/System/Utf8String.hpp"

#include "ZancleBase/Builtin/Strlen.hpp"


////////////////////////////////////////////////////////////
ExampleTextComparison::ExampleTextComparison(const GameDependencies& deps) :
    ShowcaseExample{"TextComparison"},
    m_deps{deps},
    m_legacyText(*m_deps.font, {.position = {50.f, 70.f}, .string = m_inputBuffer, .characterSize = 30u}),
    m_mappedText(m_fontFace, m_atlas.getTexture(), m_mapping, {.position = {50.f, 270.f}, .string = m_inputBuffer})
{
}


////////////////////////////////////////////////////////////
void ExampleTextComparison::update(const float /* deltaTimeMs */)
{
}


////////////////////////////////////////////////////////////
void ExampleTextComparison::imgui()
{
    ImGui::Begin("Text Comparison", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::InputTextMultiline("Text", m_inputBuffer, sizeof(m_inputBuffer)))
    {
        m_convertedStr = za::Utf8String{m_inputBuffer, ZB_STRLEN(m_inputBuffer)};

        m_legacyText.setString(m_convertedStr);
        m_mappedText.setString(m_convertedStr);
    }

    const auto legacyBounds = m_legacyText.getLocalBounds();
    const auto mappedBounds = m_mappedText.getLocalBounds();

    ImGui::Text("Legacy Text bounds: (%.1f, %.1f) size (%.1f, %.1f)",
                static_cast<double>(legacyBounds.position.x),
                static_cast<double>(legacyBounds.position.y),
                static_cast<double>(legacyBounds.size.x),
                static_cast<double>(legacyBounds.size.y));

    ImGui::Text("GlyphMapped bounds: (%.1f, %.1f) size (%.1f, %.1f)",
                static_cast<double>(mappedBounds.position.x),
                static_cast<double>(mappedBounds.position.y),
                static_cast<double>(mappedBounds.size.x),
                static_cast<double>(mappedBounds.size.y));

    const bool boundsMatch = legacyBounds == mappedBounds;
    ImGui::TextColored(boundsMatch ? ImVec4{0.f, 1.f, 0.f, 1.f} : ImVec4{1.f, 0.f, 0.f, 1.f},
                       "%s",
                       boundsMatch ? "Bounds MATCH" : "Bounds DIFFER");

    ImGui::End();
}


////////////////////////////////////////////////////////////
void ExampleTextComparison::draw()
{
    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{.position = {50.f, 50.f}, .string = "Legacy za::Text:", .characterSize = 16},
                        {.view = *m_deps.view});

    m_legacyText.draw(*m_deps.rtGame, {.view = *m_deps.view});


    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{.position = {50.f, 250.f}, .string = "za::GlyphMappedText:", .characterSize = 16},
                        {.view = *m_deps.view});

    m_mappedText.draw(*m_deps.rtGame, {.view = *m_deps.view});


    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{.position = {450.f, 50.f}, .string = "Legacy za::TextData:", .characterSize = 16},
                        {.view = *m_deps.view});

    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{.position = {450.f, 70.f}, .string = m_convertedStr, .characterSize = 30},
                        {.view = *m_deps.view});


    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{.position = {450.f, 250.f}, .string = "za::GlyphMappedTextData:", .characterSize = 16},
                        {.view = *m_deps.view});

    m_deps.rtGame->draw(m_fontFace,
                        m_mapping,
                        za::GlyphMappedTextData{.position = {450.f, 270.f}, .string = m_convertedStr},
                        {
                            .view    = *m_deps.view,
                            .texture = &m_atlas.getTexture(),
                        });


    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{.position = {50.f, 560.f}, .string = "Legacy atlas:", .characterSize = 16},
                        {.view = *m_deps.view});

    m_deps.rtGame->draw(m_deps.font->getTexture(), {.position = {50.f, 580.f}, .scale = {0.75f, 0.75f}});


    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{.position = {50.f, 700.f}, .string = "Mapped atlas:", .characterSize = 16},
                        {.view = *m_deps.view});

    m_deps.rtGame->draw(m_atlas.getTexture(), {.position = {50.f, 720.f}, .scale = {0.75f, 0.75f}});
}
