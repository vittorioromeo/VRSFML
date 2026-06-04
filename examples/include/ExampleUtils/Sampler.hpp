#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
template <typename T>
class Sampler
{
public:
    ////////////////////////////////////////////////////////////
    explicit Sampler(const zb::SizeT capacity) : m_data(capacity, T(0)), m_capacity(capacity)
    {
    }


    ////////////////////////////////////////////////////////////
    void record(const T value)
    {
        if (m_size < m_capacity)
        {
            // Still filling the buffer
            m_data[m_index] = value;
            m_sum += value;
            ++m_size;
        }
        else
        {
            // Buffer is full: subtract the overwritten value and add the new one
            m_sum           = m_sum - m_data[m_index] + value;
            m_data[m_index] = value;
        }

        // Advance index in circular fashion
        m_index = (m_index + 1u) % m_capacity;
    }


    ////////////////////////////////////////////////////////////
    template <typename U>
    [[nodiscard, gnu::always_inline]] U getAverageAs() const
    {
        return m_size == 0u ? U(0) : static_cast<U>(m_sum) / static_cast<U>(m_size);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] zb::SizeT size() const
    {
        return m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const T* data() const
    {
        return m_data.data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] zb::SizeT capacity() const
    {
        return m_capacity;
    }


    ////////////////////////////////////////////////////////////
    /// Index of the oldest sample in the circular buffer.
    /// Pass this as `values_offset` to `ImGui::PlotLines` together with `data()` and `capacity()`
    /// to plot in chronological order without copying.
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] zb::SizeT insertionIndex() const
    {
        return m_index;
    }


    ////////////////////////////////////////////////////////////
    void clear()
    {
        m_size  = 0u;
        m_index = 0u;
        m_sum   = T(0);
    }


    ////////////////////////////////////////////////////////////
    void writeSamplesInOrder(T* target) const
    {
        if (m_size < m_capacity)
        {
            // Buffer not full: copy the valid samples (indices `0` .. `m_size - 1`)
            for (zb::SizeT i = 0u; i < m_size; ++i)
                target[i] = m_data[i];

            // Fill the rest with zeros
            for (zb::SizeT i = m_size; i < m_capacity; ++i)
                target[i] = T{};
        }
        else
        {
            // Buffer is full: samples are stored in circular order
            // The oldest sample is at `m_data[m_index]`
            zb::SizeT pos = 0u;

            // Copy from `m_index` to the end
            for (zb::SizeT i = m_index; i < m_capacity; ++i)
                target[pos++] = m_data[i];

            // Then copy from the beginning up to `m_index - 1`
            for (zb::SizeT i = 0u; i < m_index; ++i)
                target[pos++] = m_data[i];
        }
    }

private:
    zb::Vector<T>   m_data;
    const zb::SizeT m_capacity;

    zb::SizeT m_size  = 0; // Number of valid samples currently in the buffer
    zb::SizeT m_index = 0; // Next index for insertion
    T               m_sum   = 0; // Running sum for fast averaging
};
