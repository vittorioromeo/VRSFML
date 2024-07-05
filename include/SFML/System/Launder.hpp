// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#if __has_builtin(__builtin_launder)
#define SFML_PRIV_LAUNDER __builtin_launder
#else
#include <new>
#define SFML_PRIV_LAUNDER ::std::launder
#endif

// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SFML_PRIV_LAUNDER_CAST(type, buffer) SFML_PRIV_LAUNDER(reinterpret_cast<type>(buffer))
