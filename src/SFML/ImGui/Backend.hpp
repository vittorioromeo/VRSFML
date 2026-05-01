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
/// \brief Initialize the bundled Dear ImGui OpenGL3 renderer backend
///
/// Vendored, lightly modified version of `imgui_impl_opengl3`. The
/// caller is responsible for making the desired OpenGL context
/// current and for selecting the active Dear ImGui context before
/// calling this function.
///
/// \param glsl_version GLSL `#version` string used when compiling
///                     the backend's shaders, or `nullptr` to let
///                     the backend pick a sensible default for the
///                     current GL/GLES profile.
///
/// \return `true` on success, `false` if the loader failed
///
////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
bool ImGui_ImplOpenGL3_Init(const char* glsl_version);

////////////////////////////////////////////////////////////
/// \brief Submit ImGui draw data to the OpenGL3 backend
///
/// Issues the OpenGL draw calls described by `draw_data` for the
/// current frame. Typically invoked with the result of
/// `ImGui::GetDrawData()` after `ImGui::Render()`.
///
/// \param draw_data Draw data produced by Dear ImGui
///
////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

////////////////////////////////////////////////////////////
/// \brief Notify the OpenGL3 backend that a new frame is starting
///
/// Lazily creates GPU resources (shaders, font texture, etc.) on
/// the first call. Must be called once per frame, before
/// `ImGui::NewFrame`.
///
////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
void ImGui_ImplOpenGL3_NewFrame();

////////////////////////////////////////////////////////////
/// \brief Shut down the OpenGL3 backend and release its GPU resources
///
////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
void ImGui_ImplOpenGL3_Shutdown();

} // namespace sf::priv
