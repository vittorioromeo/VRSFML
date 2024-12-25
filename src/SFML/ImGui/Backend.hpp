#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct ImDrawData;


namespace sf::ImGui::priv
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

} // namespace sf::ImGui::priv
