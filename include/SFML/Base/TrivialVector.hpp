#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Memcpy.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"
#include "SFML/Base/UniquePtr.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////

template <typename TItem>
class [[nodiscard]] TrivialVector
{
private:
    union [[nodiscard]] ItemUnion
    {
        TItem item;

        [[gnu::always_inline]] ItemUnion()
        {
        }
    };

    static_assert(SFML_BASE_IS_TRIVIALLY_COPYABLE(TItem));
    static_assert(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem));

    static_assert(sizeof(ItemUnion) == sizeof(TItem));
    static_assert(alignof(ItemUnion) == alignof(TItem));

    static_assert(SFML_BASE_IS_TRIVIALLY_COPYABLE(ItemUnion));
    static_assert(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(ItemUnion));

    UniquePtr<ItemUnion, UniquePtrArrayDeleter> m_data{nullptr};
    SizeT                                       m_size{};
    SizeT                                       m_capacity{};

public:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserveMore(const SizeT n)
    {
        const SizeT targetCapacity = m_size + n;

        if (m_capacity >= targetCapacity) [[likely]]
            return;

        reserve(static_cast<SizeT>(static_cast<float>(m_capacity) * 1.5f) + n);
    }


    ////////////////////////////////////////////////////////////
    void reserve(const SizeT targetCapacity)
    {
        if (m_capacity >= targetCapacity) [[likely]]
            return;

        auto newData = decltype(m_data){new ItemUnion[targetCapacity]};

        if (m_data != nullptr)
        {
            SFML_BASE_MEMCPY(newData.get(), m_data.get(), sizeof(TItem) * m_size);
        }
        else
        {
            SFML_BASE_ASSERT(m_size == 0);
            SFML_BASE_ASSERT(m_capacity == 0);
        }

        m_data     = SFML_BASE_MOVE(newData);
        m_capacity = targetCapacity;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void unsafeEmplaceRange(const TItem* ptr, SizeT count) noexcept
    {
        SFML_BASE_ASSERT(m_size + count <= m_capacity);
        SFML_BASE_ASSERT(m_data != nullptr);

        SFML_BASE_MEMCPY(m_data.get() + m_size, ptr, sizeof(TItem) * count);

        m_size += count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void unsafeEmplaceOther(const TrivialVector& rhs) noexcept
    {
        unsafeEmplaceRange(rhs.m_data.get(), rhs.m_size);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear() noexcept
    {
        m_size = 0;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] SizeT size() const noexcept
    {
        return m_size;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] void unsafeEmplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(m_size <= m_capacity);
        SFML_BASE_ASSERT(m_data != nullptr);

        new (&m_data.get()[m_size++].item) TItem(static_cast<TItem>(SFML_BASE_FORWARD(xs))...);
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] void unsafePushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(m_size + sizeof...(items) <= m_capacity);
        SFML_BASE_ASSERT(m_data != nullptr);

        (new (&m_data.get()[m_size++].item) TItem(static_cast<TItem>(SFML_BASE_FORWARD(items))), ...);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] TItem* data() noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_data.get());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const TItem* data() const noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_data.get());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] TItem& operator[](const SizeT i) noexcept
    {
        SFML_BASE_ASSERT(i < m_size);
        SFML_BASE_ASSERT(m_data != nullptr);

        return m_data.get()[i].item;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const TItem& operator[](const SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(i < m_size);
        SFML_BASE_ASSERT(m_data != nullptr);

        return m_data.get()[i].item;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] TItem* begin() noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        return &(m_data.get()[0].item);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const TItem* begin() const noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        return &(m_data.get()[0].item);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] TItem* end() noexcept
    {
        return begin() + m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const TItem* end() const noexcept
    {
        return begin() + m_size;
    }
};

} // namespace sf::base
