#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Public Dear ImGui include shim
//
// Pulls in `<imgui.h>` after applying VRSFML's `IMGUI_USER_CONFIG`
// (`ImConfigSFML.hpp`), which injects implicit conversions between
// `ImVec2`/`ImVec4` and `sf::Vec2`/`sf::Color`. User code that
// needs Dear ImGui types or `ImGui::*` functions should include
// this header instead of `<imgui.h>` directly.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/ImGui/ImConfigSFML.hpp" // IWYU pragma: keep

#include <imgui.h> // IWYU pragma: export
