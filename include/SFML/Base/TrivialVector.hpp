#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Memcpy.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"


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

    ItemUnion* m_data{nullptr};
    ItemUnion* m_endSize{nullptr};
    ItemUnion* m_endCapacity{nullptr};

public:
    ////////////////////////////////////////////////////////////
    explicit TrivialVector() = default;


    ////////////////////////////////////////////////////////////
    ~TrivialVector()
    {
        delete[] m_data;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TrivialVector(const TrivialVector& rhs) :
    m_data{rhs.m_data == nullptr ? nullptr : new ItemUnion[rhs.size()]},
    m_endSize{m_data + rhs.size()},
    m_endCapacity{m_data + rhs.size()}
    {
        SFML_BASE_MEMCPY(m_data, rhs.m_data, sizeof(TItem) * size());
    }


    ////////////////////////////////////////////////////////////
    TrivialVector& operator=(const TrivialVector& rhs)
    {
        if (this == &rhs)
            return *this;

        reserve(rhs.size());
        m_endSize = m_data + rhs.size();

        SFML_BASE_MEMCPY(m_data, rhs.m_data, sizeof(TItem) * size());

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TrivialVector(TrivialVector&& rhs) noexcept :
    m_data{rhs.m_data},
    m_endSize{rhs.m_endSize},
    m_endCapacity{rhs.m_endCapacity}
    {
        rhs.m_data = nullptr;
    }


    ////////////////////////////////////////////////////////////
    TrivialVector& operator=(TrivialVector&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        m_data        = rhs.m_data;
        m_endSize     = rhs.m_endSize;
        m_endCapacity = rhs.m_endCapacity;

        rhs.m_data = nullptr;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const SizeT n)
    {
        const auto oldSize = size();

        reserve(n);
        m_endSize = m_data + n;

        for (ItemUnion* p = m_data + oldSize; p < m_endSize; ++p)
            SFML_BASE_PLACEMENT_NEW(p) TItem();
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserveMore(const SizeT n)
    {
        const SizeT targetCapacity = size() + n;

        if (capacity() >= targetCapacity) [[likely]]
            return;

        reserve(static_cast<SizeT>(static_cast<float>(capacity()) * 1.5f) + n);
    }


    ////////////////////////////////////////////////////////////
    void reserve(const SizeT targetCapacity)
    {
        if (capacity() >= targetCapacity) [[likely]]
            return;

        auto*      newData = new ItemUnion[targetCapacity];
        const auto oldSize = size();

        if (m_data != nullptr)
        {
            SFML_BASE_MEMCPY(newData, m_data, sizeof(TItem) * oldSize);
            delete[] m_data;
        }
        else
        {
            SFML_BASE_ASSERT(size() == 0);
            SFML_BASE_ASSERT(capacity() == 0);
        }

        m_data        = newData;
        m_endSize     = m_data + oldSize;
        m_endCapacity = m_data + targetCapacity;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void unsafeEmplaceRange(const TItem* ptr, SizeT count) noexcept
    {
        SFML_BASE_ASSERT(size() + count <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        SFML_BASE_MEMCPY(m_endSize, ptr, sizeof(TItem) * count);

        m_endSize += count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void unsafeEmplaceOther(const TrivialVector& rhs) noexcept
    {
        unsafeEmplaceRange(rhs.m_data.get(), rhs.m_size);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear() noexcept
    {
        m_endSize = m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT size() const noexcept
    {
        return static_cast<SizeT>(m_endSize - m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT capacity() const noexcept
    {
        return static_cast<SizeT>(m_endCapacity - m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] bool empty() const noexcept
    {
        return m_endSize == 0u;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] void unsafeEmplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(size() <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        SFML_BASE_PLACEMENT_NEW(m_endSize++) TItem(static_cast<TItem>(SFML_BASE_FORWARD(xs))...);
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] void unsafePushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(size() + sizeof...(items) <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        (SFML_BASE_PLACEMENT_NEW(m_endSize++) TItem(static_cast<TItem>(SFML_BASE_FORWARD(items))), ...);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] TItem* data() noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] const TItem* data() const noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] TItem& operator[](const SizeT i) noexcept
    {
        SFML_BASE_ASSERT(i < size());
        SFML_BASE_ASSERT(m_data != nullptr);

        return *SFML_BASE_LAUNDER_CAST(TItem*, m_data + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] const TItem& operator[](const SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(i < size());
        SFML_BASE_ASSERT(m_data != nullptr);

        return *SFML_BASE_LAUNDER_CAST(TItem*, m_data + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] TItem* begin() noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] const TItem* begin() const noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] TItem* end() noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_endSize);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] const TItem* end() const noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, m_endSize);
    }
};

} // namespace sf::base
