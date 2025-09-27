// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Glsl.hpp"
#include "SFML/Graphics/Transform.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
void copyMatrix(const Transform& source, Matrix<3, 3>& dest)
{
    dest.array[0] = source.a00;
    dest.array[1] = source.a10;
    dest.array[3] = source.a01;
    dest.array[4] = source.a11;
    dest.array[6] = source.a02;
    dest.array[7] = source.a12;
    dest.array[8] = 1.f;
}


////////////////////////////////////////////////////////////
void copyMatrix(const Transform& source, Matrix<4, 4>& dest)
{
    source.getMatrix(dest.array);
    dest.array[10] = 1.f;
    dest.array[15] = 1.f;
}

} // namespace sf::priv
