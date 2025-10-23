#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ShapeDimension.hpp"
#include "ShapeMatrix.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"


namespace tsurv::tetramino_shapes
{
////////////////////////////////////////////////////////////
// clang-format off
using enum ShapeBlockSequence;


////////////////////////////////////////////////////////////
// I-Piece Rotations
inline constexpr ShapeMatrix iR0{_, _, _, _,
                                 A, B, C, D,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix iR1{_, _, A, _,
                                 _, _, B, _,
                                 _, _, C, _,
                                 _, _, D, _};

inline constexpr ShapeMatrix iR2{_, _, _, _,
                                 _, _, _, _,
                                 D, C, B, A,
                                 _, _, _, _};

inline constexpr ShapeMatrix iR3{_, D, _, _,
                                 _, C, _, _,
                                 _, B, _, _,
                                 _, A, _, _};


////////////////////////////////////////////////////////////
// J-Piece Rotations
inline constexpr ShapeMatrix jR0{A, _, _, _,
                                 B, C, D, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix jR1{_, B, A, _,
                                 _, C, _, _,
                                 _, D, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix jR2{_, _, _, _,
                                 D, C, B, _,
                                 _, _, A, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix jR3{_, D, _, _,
                                 _, C, _, _,
                                 A, B, _, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// L-Piece Rotations
inline constexpr ShapeMatrix lR0{_, _, A, _,
                                 D, C, B, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix lR1{_, D, _, _,
                                 _, C, _, _,
                                 _, B, A, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix lR2{_, _, _, _,
                                 B, C, D, _,
                                 A, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix lR3{A, B, _, _,
                                 _, C, _, _,
                                 _, D, _, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// O-Piece Rotations
inline constexpr ShapeMatrix oR0{_, A, B, _,
                                 _, D, C, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix oR1{_, D, A, _,
                                 _, C, B, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix oR2{_, C, D, _,
                                 _, B, A, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix oR3{_, B, C, _,
                                 _, A, D, _,
                                 _, _, _, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// S-Piece Rotations
inline constexpr ShapeMatrix sR0{_, A, B, _,
                                 C, D, _, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix sR1{_, C, _, _,
                                 _, D, A, _,
                                 _, _, B, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix sR2{_, _, _, _,
                                 _, D, C, _,
                                 B, A, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix sR3{B, _, _, _,
                                 A, D, _, _,
                                 _, C, _, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// T-Piece Rotations
inline constexpr ShapeMatrix tR0{_, A, _, _,
                                 B, C, D, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix tR1{_, B, _, _,
                                 _, C, A, _,
                                 _, D, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix tR2{_, _, _, _,
                                 D, C, B, _,
                                 _, A, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix tR3{_, D, _, _,
                                 A, C, _, _,
                                 _, B, _, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// Z-Piece Rotations
inline constexpr ShapeMatrix zR0{A, B, _, _,
                                 _, C, D, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix zR1{_, _, A, _,
                                 _, C, B, _,
                                 _, D, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix zR2{_, _, _, _,
                                 D, C, _, _,
                                 _, B, A, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix zR3{_, D, _, _,
                                 B, C, _, _,
                                 A, _, _, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// clang-format on


////////////////////////////////////////////////////////////
enum class TetraminoType : sf::base::U8
{
    I = 0u,
    J = 1u,
    L = 2u,
    O = 3u,
    S = 4u,
    T = 5u,
    Z = 6u,
};


////////////////////////////////////////////////////////////
inline constexpr sf::base::SizeT tetraminoShapeCount = 7u;


////////////////////////////////////////////////////////////
// This master array stores all rotation data
// Indexing: tetraminoShapes[pieceType][rotationState]
inline constexpr sf::base::Array<sf::base::Array<ShapeMatrix, shapeDimension>, tetraminoShapeCount> srsTetraminoShapes = {{
    {iR0, iR1, iR2, iR3}, // 0: I-Piece
    {jR0, jR1, jR2, jR3}, // 1: J-Piece
    {lR0, lR1, lR2, lR3}, // 2: L-Piece
    {oR0, oR1, oR2, oR3}, // 3: O-Piece
    {sR0, sR1, sR2, sR3}, // 4: S-Piece
    {tR0, tR1, tR2, tR3}, // 5: T-Piece
    {zR0, zR1, zR2, zR3}, // 6: Z-Piece
}};


////////////////////////////////////////////////////////////
// These tables define the "kick" offsets to test for wall/floor collisions during rotation
// There are 5 tests for each rotation (the first valid one is used)
using KickTable = sf::base::Array<sf::Vec2i, 5>;


////////////////////////////////////////////////////////////
// Kick data for J, L, S, T, Z pieces
inline constexpr sf::base::Array<KickTable, 8> kickDataJLSTZ = {{
    /* 0 -> 1 */ {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},
    /* 1 -> 0 */ {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}},
    /* 1 -> 2 */ {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}},
    /* 2 -> 1 */ {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},
    /* 2 -> 3 */ {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, -2}}},
    /* 3 -> 2 */ {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, 2}}},
    /* 3 -> 0 */ {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, 2}}},
    /* 0 -> 3 */ {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, -2}}},
}};


////////////////////////////////////////////////////////////
// Kick data for the I piece
inline constexpr sf::base::Array<KickTable, 8> kickDataI = {{
    /* 0 -> 1 */ {{{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}}},
    /* 1 -> 0 */ {{{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}}},
    /* 1 -> 2 */ {{{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}}},
    /* 2 -> 1 */ {{{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}}},
    /* 2 -> 3 */ {{{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}}},
    /* 3 -> 2 */ {{{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}}},
    /* 3 -> 0 */ {{{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}}},
    /* 0 -> 3 */ {{{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}}},
}};

} // namespace tsurv::tetramino_shapes


namespace tsurv
{
////////////////////////////////////////////////////////////
using tetramino_shapes::kickDataI;
using tetramino_shapes::kickDataJLSTZ;
using tetramino_shapes::srsTetraminoShapes;
using tetramino_shapes::tetraminoShapeCount;
using tetramino_shapes::TetraminoType;

} // namespace tsurv
