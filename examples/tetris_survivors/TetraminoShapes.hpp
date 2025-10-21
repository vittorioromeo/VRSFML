#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ShapeMatrix.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
// NOLINTBEGIN(modernize-use-bool-literals)
// clang-format off
inline constexpr ShapeMatrix iTetraminoShape{0, 0, 0, 0,
                                             1, 1, 1, 1,
                                             0, 0, 0, 0,
                                             0, 0, 0, 0};

inline constexpr ShapeMatrix oTetraminoShape{0, 0, 0, 0,
                                             0, 1, 1, 0,
                                             0, 1, 1, 0,
                                             0, 0, 0, 0};

inline constexpr ShapeMatrix tTetraminoShape{0, 0, 0, 0,
                                             0, 1, 0, 0,
                                             1, 1, 1, 0,
                                             0, 0, 0, 0};

inline constexpr ShapeMatrix lTetraminoShape{0, 0, 0, 0,
                                             0, 0, 1, 0,
                                             1, 1, 1, 0,
                                             0, 0, 0, 0};

inline constexpr ShapeMatrix jTetraminoShape{0, 0, 0, 0,
                                             1, 0, 0, 0,
                                             1, 1, 1, 0,
                                             0, 0, 0, 0};

inline constexpr ShapeMatrix sTetraminoShape{0, 0, 0, 0,
                                             0, 1, 1, 0,
                                             1, 1, 0, 0,
                                             0, 0, 0, 0};

inline constexpr ShapeMatrix zTetraminoShape{0, 0, 0, 0,
                                             1, 1, 0, 0,
                                             0, 1, 1, 0,
                                             0, 0, 0, 0};
// clang-format on
// NOLINTEND(modernize-use-bool-literals)


////////////////////////////////////////////////////////////
inline constexpr sf::base::Array<ShapeMatrix, 7> tetraminoShapes = { //
    iTetraminoShape,
    oTetraminoShape,
    tTetraminoShape,
    lTetraminoShape,
    jTetraminoShape,
    sTetraminoShape,
    zTetraminoShape};

} // namespace tsurv
