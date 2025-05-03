#pragma once


#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
class Sampler
{
public:
    ////////////////////////////////////////////////////////////
    enum : sf::base::SizeT
    {
        ToIgnore = 0u
    };

    ////////////////////////////////////////////////////////////
    explicit Sampler(const sf::base::SizeT capacity) : m_capacity(capacity)
    {
        m_data.resize(capacity, 0.f);
    }

    ////////////////////////////////////////////////////////////
    void record(const float value)
    {
        if (m_toIgnore > 0u)
        {
            --m_toIgnore;
            return;
        }

        if (m_size < m_capacity)
        {
            // Still filling the buffer.
            m_data[m_index] = value;
            m_sum += value;
            ++m_size;
        }
        else
        {
            // Buffer is full: subtract the overwritten value and add the new one.
            m_sum           = m_sum - m_data[m_index] + value;
            m_data[m_index] = value;
        }

        // Advance index in circular fashion.
        m_index = (m_index + 1u) % m_capacity;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] double getAverage() const
    {
        if (m_size == 0u)
            return 0.0;

        return static_cast<double>(m_sum) / static_cast<double>(m_size);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] sf::base::SizeT size() const
    {
        return m_size;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const float* data() const
    {
        return m_data.data();
    }

    ////////////////////////////////////////////////////////////
    void clear()
    {
        for (float& x : m_data)
            x = 0.f;

        m_size  = 0u;
        m_index = 0u;
        m_sum   = 0.f;

        m_toIgnore = ToIgnore;
    }

    ////////////////////////////////////////////////////////////
    void writeSamplesInOrder(float* target) const
    {
        if (m_size < m_capacity)
        {
            // Buffer not full: copy the valid samples (indices 0 .. m_size - 1)
            for (sf::base::SizeT i = 0u; i < m_size; ++i)
                target[i] = m_data[i];

            // Fill the rest with zeros.
            for (sf::base::SizeT i = m_size; i < m_capacity; ++i)
                target[i] = 0.f;
        }
        else
        {
            // Buffer is full: samples are stored in circular order.
            // The oldest sample is at m_data[m_index].
            sf::base::SizeT pos = 0u;

            // Copy from m_index to the end.
            for (sf::base::SizeT i = m_index; i < m_capacity; ++i)
                target[pos++] = m_data[i];

            // Then copy from the beginning up to m_index - 1.
            for (sf::base::SizeT i = 0u; i < m_index; ++i)
                target[pos++] = m_data[i];
        }
    }

private:
    sf::base::Vector<float> m_data;
    const sf::base::SizeT          m_capacity;

    sf::base::SizeT m_size  = 0;   // Number of valid samples currently in the buffer.
    sf::base::SizeT m_index = 0;   // Next index for insertion.
    float           m_sum   = 0.f; // Running sum for fast averaging.

    sf::base::SizeT m_toIgnore = ToIgnore;
};
