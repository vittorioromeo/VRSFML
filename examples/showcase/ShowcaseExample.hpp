#pragma once

#include "SFML/System/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Font;
class RenderTarget;
class RenderWindow;
struct View;
} // namespace sf


////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1016.f, 1016.f};


////////////////////////////////////////////////////////////
struct GameDependencies
{
    sf::RenderWindow* window;
    sf::RenderTarget* rtGame;
    const sf::View*   view;
    const sf::Font*   font;
};


////////////////////////////////////////////////////////////
struct ShowcaseExample
{
    const char* name;

    explicit ShowcaseExample(const char* theName) : name{theName}
    {
    }

    virtual ~ShowcaseExample() = default;

    virtual void update(float deltaTimeMs) = 0;

    virtual void imgui()
    {
    }

    virtual void draw() = 0;
};
