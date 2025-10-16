#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Output iterator that inserts elements using `push_back`
///
/// Acts like `std::back_insert_iterator`.
///
////////////////////////////////////////////////////////////
template <typename T>
class BackInserter
{
private:
    ////////////////////////////////////////////////////////////
    T* m_container;


public:
    ////////////////////////////////////////////////////////////
    using container_type = T;
    using value_type     = T::value_type;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] explicit BackInserter(T& container) noexcept : m_container(&container)
    {
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] BackInserter& operator=(const value_type& value)
    {
        if constexpr (requires { m_container->push_back(value); })
        {
            m_container->push_back(value);
        }
        else
        {
            m_container->pushBack(value);
        }

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] BackInserter& operator=(value_type&& value)
    {
        if constexpr (requires { m_container->push_back(static_cast<value_type&&>(value)); })
        {
            m_container->push_back(static_cast<value_type&&>(value));
        }
        else
        {
            m_container->pushBack(static_cast<value_type&&>(value));
        }

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] BackInserter& operator*() noexcept
    {
        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::pure]] BackInserter& operator++() noexcept
    {
        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] BackInserter operator++(int) noexcept
    {
        return *this;
    }
};

} // namespace sf::base
