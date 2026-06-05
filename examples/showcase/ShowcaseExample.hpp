#pragma once

#include "Zancle/Geometry/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Font;
class RenderTarget;
class RenderWindow;
struct View;
} // namespace za


////////////////////////////////////////////////////////////
constexpr za::Vec2f resolution{1016.f, 1016.f};


////////////////////////////////////////////////////////////
struct GameDependencies
{
    za::RenderWindow* window;
    za::RenderTarget* rtGame;
    const za::View*   view;
    const za::Font*   font;
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
