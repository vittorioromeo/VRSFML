// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/PathStreamOp.hpp"

#include "SFML/System/Path.hpp"

#include <ios>
#include <ostream>
#include <string>


namespace sf
{
////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const Path& path)
{
    // Use UTF-8 rather than streaming `std::filesystem::path` directly: the latter uses
    // the C locale, which throws on MinGW/Clang64 for some non-ASCII paths.
    const auto u8 = path.to<std::string>();
    return os.write(u8.data(), static_cast<std::streamsize>(u8.size()));
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \file
///
/// Isolated translation unit for `operator<<(std::ostream&, sf::Path)`.
///
/// Lives apart from the main `Path.cpp` (Unity) on purpose: in
/// libstdc++16 `<ostream>` pulls in `<format>`, which then drags in
/// `<locale>` and the whole `std::__format` template machinery. Keeping
/// that inclusion inside this tiny stand-alone TU stops it from
/// poisoning `ErrIOUnity.cpp` (Err + IO + Path) -- previously one of the
/// slowest parses in the whole build (~2.2 s) and a source of ~3 s of
/// `std::vformat_to` / `std::formatter` template instantiations.
////////////////////////////////////////////////////////////
