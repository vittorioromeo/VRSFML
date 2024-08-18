#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Generic implementation of the PassKey idiom
///
////////////////////////////////////////////////////////////
template <typename T>
class PassKey
{
    friend T;

private:
    // NOLINTBEGIN(modernize-use-equals-delete)
    // NOLINTBEGIN(modernize-use-equals-default)

    // Intentionally not using `= default` here as it would make `PassKey` an aggregate
    // and thus constructible from anyone
    [[nodiscard]] explicit PassKey() noexcept
    {
    }

    //NOLINTEND(modernize-use-equals-default)
    //NOLINTEND(modernize-use-equals-delete)

public:
    PassKey(const PassKey&) = delete;
    PassKey(PassKey&&)      = delete;

    PassKey& operator=(const PassKey&) = delete;
    PassKey& operator=(PassKey&&)      = delete;
};

} // namespace sf::base
