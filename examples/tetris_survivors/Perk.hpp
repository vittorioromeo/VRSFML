#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "World.hpp"

#include "SFML/Base/FwdStdString.hpp" // used


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] Perk
{
    ////////////////////////////////////////////////////////////
    virtual ~Perk() = default;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual std::string getName() const                             = 0;
    [[nodiscard]] virtual std::string getDescription(const World& world) const    = 0;
    [[nodiscard]] virtual std::string getProgressionStr(const World& world) const = 0;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool meetsPrerequisites(const World& world) const = 0;
    virtual void               apply(World& world) const                    = 0;
};

} // namespace tsurv
