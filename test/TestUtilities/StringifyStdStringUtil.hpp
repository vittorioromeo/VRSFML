#pragma once

#include "SFML/Base/FwdStdString.hpp"

// `std::basic_string<T>`, `char8_t`, and `char16_t` render as "<?>" via the
// catch-all in `Tst/Detail/StringifyValue.hpp`. (Previously `std::string`
// showed its contents; std-namespace types cannot carry an ADL hook, so
// they are intentionally left to the placeholder.)
