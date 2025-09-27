#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct ImDrawData;

namespace sf::priv
{
////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
bool ImGui_ImplOpenGL3_Init(const char* glsl_version);

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
void ImGui_ImplOpenGL3_NewFrame();

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
void ImGui_ImplOpenGL3_Shutdown();

} // namespace sf::priv
