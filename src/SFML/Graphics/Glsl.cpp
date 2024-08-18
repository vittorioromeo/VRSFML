#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Glsl.hpp"
#include "SFML/Graphics/Transform.hpp"

#include <cstring>


namespace sf::priv
{

////////////////////////////////////////////////////////////
void copyMatrix(const Transform& source, Matrix<3, 3>& dest)
{
    const float* from = source.getMatrix(); // 4x4
    float*       to   = dest.array;         // 3x3

    // Use only left-upper 3x3 block (for a 2D transform)
    to[0] = from[0];
    to[1] = from[1];
    to[2] = from[3];
    to[3] = from[4];
    to[4] = from[5];
    to[5] = from[7];
    to[6] = from[12];
    to[7] = from[13];
    to[8] = from[15];
}


////////////////////////////////////////////////////////////
void copyMatrix(const Transform& source, Matrix<4, 4>& dest)
{
    // Adopt 4x4 matrix as-is
    copyMatrix(source.getMatrix(), 4 * 4, dest.array);
}

} // namespace sf::priv
