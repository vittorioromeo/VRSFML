#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Internal Dear ImGui include shim
//
// Pulls in `<imgui_internal.h>` (which itself depends on
// `<imgui.h>`) after applying Zancle's `IMGUI_USER_CONFIG`. Use
// this header only when you genuinely need access to Dear ImGui's
// unstable internal API; prefer `IncludeImGui.hpp` for the
// public API.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/ImGui/IncludeImGui.hpp" // IWYU pragma: keep

#include <imgui_internal.h> // IWYU pragma: export
