#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Generic implementation of the PassKey idiom
///
/// `PassKey<T>` is a tag type that only `T` can construct (because it
/// befriends `T`). Functions that take a `PassKey<T>` as a parameter
/// are therefore callable only by `T`, even when they are publicly
/// declared. This lets a class expose a function to a single specific
/// caller without forcing it to be a friend.
///
/// `PassKey` is non-copyable and non-movable to prevent third parties
/// from acquiring an instance through indirect means.
///
////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] PassKey
{
    friend T;

private:
    // NOLINTBEGIN(modernize-use-equals-delete)
    // NOLINTBEGIN(modernize-use-equals-default)

    ////////////////////////////////////////////////////////////
    // Intentionally not using `= default` here as it would make `PassKey` an aggregate
    // and thus constructible from anyone
    [[nodiscard]] explicit PassKey() noexcept
    {
    }

    // NOLINTEND(modernize-use-equals-default)
    // NOLINTEND(modernize-use-equals-delete)

public:
    ////////////////////////////////////////////////////////////
    PassKey(const PassKey&) = delete;
    PassKey(PassKey&&)      = delete;

    ////////////////////////////////////////////////////////////
    PassKey& operator=(const PassKey&) = delete;
    PassKey& operator=(PassKey&&)      = delete;
};

} // namespace sf::base
