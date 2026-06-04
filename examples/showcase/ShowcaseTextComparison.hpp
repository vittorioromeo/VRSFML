#pragma once

#include "ShowcaseExample.hpp"

#include "Zancle/Graphics/FontFace.hpp"
#include "Zancle/Graphics/GlyphMappedText.hpp"
#include "Zancle/Graphics/GlyphMapping.hpp"
#include "Zancle/Graphics/Text.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/TextureAtlas.hpp"

#include "Zancle/System/Path.hpp"
#include "Zancle/System/Utf8String.hpp"


////////////////////////////////////////////////////////////
class ExampleTextComparison : public ShowcaseExample
{
private:
    ////////////////////////////////////////////////////////////
    GameDependencies m_deps;

    ////////////////////////////////////////////////////////////
    za::FontFace     m_fontFace = za::FontFace::openFromFile("resources/tuffy.ttf").value();
    za::TextureAtlas m_atlas{za::Texture::create({1024u, 1024u}, {.smooth = true}).value()};
    za::GlyphMapping m_mapping = m_fontFace
                                     .loadGlyphs(m_atlas,
                                                 za::GlyphLoadSettings::latin1(/* characterSize */ 30,
                                                                               /* bold */ false,
                                                                               /* outlineThickness */ 0.f))
                                     .value();

    ////////////////////////////////////////////////////////////
    char           m_inputBuffer[256] = "Hello, World!";
    za::Utf8String m_convertedStr     = "Hello, World!";

    ////////////////////////////////////////////////////////////
    za::Text            m_legacyText;
    za::GlyphMappedText m_mappedText;

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleTextComparison(const GameDependencies& deps);

    ////////////////////////////////////////////////////////////
    void update(float deltaTimeMs) override;

    ////////////////////////////////////////////////////////////
    void imgui() override;

    ////////////////////////////////////////////////////////////
    void draw() override;
};
