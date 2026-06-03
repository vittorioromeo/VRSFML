// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Entry point. Hands off to the bespoke testing runner.
////////////////////////////////////////////////////////////
#include "Tst/Tst.hpp"

#include "SFML/Config.hpp" // IWYU pragma: keep


////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    return ::tst::run(argc, argv);
}
