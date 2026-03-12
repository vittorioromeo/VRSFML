#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "World.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::base
{
class String;
} // namespace sf::base


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] Perk
{
    ////////////////////////////////////////////////////////////
    virtual ~Perk() = default;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual sf::base::String getName() const                             = 0;
    [[nodiscard]] virtual sf::base::String getDescription(const World& world) const    = 0;
    [[nodiscard]] virtual sf::base::String getProgressionStr(const World& world) const = 0;
    [[nodiscard]] virtual sf::base::String getInventoryStr(const World& world) const   = 0;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool isActive(const World& world) const           = 0;
    [[nodiscard]] virtual bool meetsPrerequisites(const World& world) const = 0;

    ////////////////////////////////////////////////////////////
    virtual void apply(World& world) const = 0;
};

} // namespace tsurv
