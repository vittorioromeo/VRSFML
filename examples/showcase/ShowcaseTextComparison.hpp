#pragma once

#include "ShowcaseExample.hpp"

#include "SFML/Graphics/FontFace.hpp"
#include "SFML/Graphics/GlyphMappedText.hpp"
#include "SFML/Graphics/GlyphMapping.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/UnicodeString.hpp"


////////////////////////////////////////////////////////////
class ExampleTextComparison : public ShowcaseExample
{
private:
    ////////////////////////////////////////////////////////////
    GameDependencies m_deps;

    ////////////////////////////////////////////////////////////
    sf::FontFace     m_fontFace = sf::FontFace::openFromFile("resources/tuffy.ttf").value();
    sf::TextureAtlas m_atlas{sf::Texture::create({1024u, 1024u}, {.smooth = true}).value()};
    sf::GlyphMapping m_mapping = m_fontFace
                                     .loadGlyphs(m_atlas,
                                                 sf::GlyphLoadSettings::latin1(/* characterSize */ 30,
                                                                               /* bold */ false,
                                                                               /* outlineThickness */ 0.f))
                                     .value();

    ////////////////////////////////////////////////////////////
    char              m_inputBuffer[256] = "Hello, World!";
    sf::UnicodeString m_convertedStr     = "Hello, World!";

    ////////////////////////////////////////////////////////////
    sf::Text            m_legacyText;
    sf::GlyphMappedText m_mappedText;

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
