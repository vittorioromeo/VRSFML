struct ParticleSoA
{
    std::vector<sf::Vec2f> position;
    std::vector<sf::Vec2f> velocity;
    std::vector<sf::Vec2f> acceleration;

    std::vector<float> scale;
    std::vector<float> opacity;
    std::vector<float> rotation;

    std::vector<float> scaleRate;
    std::vector<float> opacityChange;
    std::vector<float> angularVelocity;

    void pushBack(const sf::Vec2f pos, const sf::Vec2f vel, const sf::Vec2f acc, float s, float o, float r, float sr, float oc, float av)
    {
        position.push_back(pos);
        velocity.push_back(vel);
        acceleration.push_back(acc);

        scale.push_back(s);
        opacity.push_back(o);
        rotation.push_back(r);

        scaleRate.push_back(sr);
        opacityChange.push_back(oc);
        angularVelocity.push_back(av);
    }

    std::size_t getSize() const
    {
        return position.size();
    }

    void forAllVectors(auto&& f)
    {
        f(position);
        f(velocity);
        f(acceleration);

        f(scale);
        f(opacity);
        f(rotation);

        f(scaleRate);
        f(opacityChange);
        f(angularVelocity);
    }

    void withNth(std::size_t i, auto&& f)
    {
        f(position[i],
          velocity[i],
          acceleration[i],

          scale[i],
          opacity[i],
          rotation[i],

          scaleRate[i],
          opacityChange[i],
          angularVelocity[i]);
    }

    void clear()
    {
        forAllVectors([](auto& vec) { vec.clear(); });
    }

    void resize(std::size_t newSize)
    {
        forAllVectors([&](auto& vec) { vec.resize(newSize); });
    }

    void reserve(std::size_t newCapacity)
    {
        forAllVectors([&](auto& vec) { vec.reserve(newCapacity); });
    }

    void swapNth(std::size_t i, std::size_t j)
    {
        withNth(i, [&](auto&... valuesI) { withNth(j, [&](auto&... valuesJ) { (..., std::swap(valuesI, valuesJ)); }); });
    }

    void cleanup()
    {
        std::size_t n = getSize();
        std::size_t i = 0u;

        // Process elements, swapping out removed ones.
        while (i < n)
        {
            if (opacities[i] > 0.f)
            {
                ++i;
                continue;
            }

            // Swap the current element with the last one, then reduce the container size.
            --n;
            swapNth(i, n);

            // Do not increment `i`; check the new element at `i`.
        }

        // Resize all columns to the new size.
        resize(n);
    }
};
