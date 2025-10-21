#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ShapeDimension.hpp"
#include "ShapeMatrix.hpp"


namespace tsurv::tetramino_shapes
{
////////////////////////////////////////////////////////////
// clang-format off
using enum ShapeBlock;


////////////////////////////////////////////////////////////
// --- I-Piece Rotations ---
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
// --- J-Piece Rotations ---
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
// --- L-Piece Rotations ---
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
// --- O-Piece Rotations ---
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
// --- S-Piece Rotations ---
inline constexpr ShapeMatrix sR0{_, A, B, _,
                                 C, D, _, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix sR1{_, C, _, _,
                                 _, D, A, _,
                                 _, _, B, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix sR2{_, D, C, _,
                                 B, A, _, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix sR3{_, B, _, _,
                                 _, A, D, _,
                                 _, _, C, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// --- T-Piece Rotations ---
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
// --- Z-Piece Rotations ---
inline constexpr ShapeMatrix zR0{A, B, _, _,
                                 _, C, D, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix zR1{_, _, A, _,
                                 _, C, B, _,
                                 _, D, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix zR2{D, C, _, _,
                                 _, B, A, _,
                                 _, _, _, _,
                                 _, _, _, _};

inline constexpr ShapeMatrix zR3{_, _, D, _,
                                 _, B, C, _,
                                 _, A, _, _,
                                 _, _, _, _};


////////////////////////////////////////////////////////////
// clang-format on


////////////////////////////////////////////////////////////
enum class TetraminoType : unsigned char
{
    I = 0,
    J = 1,
    L = 2,
    O = 3,
    S = 4,
    T = 5,
    Z = 6,
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

} // namespace tsurv::tetramino_shapes


namespace tsurv
{
////////////////////////////////////////////////////////////
using tetramino_shapes::srsTetraminoShapes;
using tetramino_shapes::tetraminoShapeCount;
using tetramino_shapes::TetraminoType;

} // namespace tsurv
