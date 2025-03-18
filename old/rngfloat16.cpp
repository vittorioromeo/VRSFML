
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline _Float16 getHf(const _Float16 min, const _Float16 max)
    {
        ASSERT_AND_ASSUME(min <= max);

        // Returns a _Float16 in the inclusive range [min, max].
        // We extract 11 random bits, which is enough to fill the 10-bit mantissa of _Float16,
        // and normalize by dividing by (2^11 - 1).

        const sf::base::U32 randomBits = next() >> (64 - 11);                                    // Extract 11 bits.
        const _Float16 normalized = static_cast<_Float16>(randomBits) / _Float16((1 << 11) - 1); // Normalize to [0, 1].

        return min + normalized * (max - min);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2<_Float16> getVec2Hf(const sf::Vector2<_Float16> mins,
                                                                                           const sf::Vector2<_Float16> maxs)
    {
        return {getHf(mins.x, maxs.x), getHf(mins.y, maxs.y)};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Vector2<_Float16> getVec2Hf(const sf::Vector2<_Float16> maxs)
    {
        return {getHf(0.f, maxs.x), getHf(0.f, maxs.y)};
    }
