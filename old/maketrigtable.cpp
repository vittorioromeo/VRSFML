#include <iostream>

inline constexpr float pi = 3.141592654f;
inline constexpr float tau = pi * 2.f;

constexpr unsigned int sinBits    = 16u;
constexpr unsigned int sinMask    = ~(-1u << sinBits);
constexpr unsigned int sinCount   = sinMask + 1u;
constexpr float        radToIndex = sinCount / tau;
constexpr float        degToIndex = sinCount / 360.f;

struct TrigLookup
{
    float sintable[sinCount]{};

    [[nodiscard, gnu::always_inline, gnu::flatten]] TrigLookup()
    {
        for (unsigned int i = 0u; i < sinCount; ++i)
            sintable[i] = __builtin_sinf((static_cast<float>(i) + 0.5f) / sinCount * tau);

        // The four right angles get extra-precise values, because they are
        // the most likely to need to be correct.

        sintable[0]                                                      = 0.f;
        sintable[static_cast<unsigned int>(90u * degToIndex) & sinMask]  = 1.f;
        sintable[static_cast<unsigned int>(180u * degToIndex) & sinMask] = 0.f;
        sintable[static_cast<unsigned int>(270u * degToIndex) & sinMask] = -1.f;
    }

    // sin(x) for x between -2*pi and 2*pi
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float ftsin(float radians) noexcept
    {
        return sintable[static_cast<unsigned int>(radians * radToIndex) & sinMask];
    }

    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float ftcos(float radians) noexcept
    {
        return sintable[static_cast<unsigned int>((radians + (pi / 2.f)) * radToIndex) & sinMask];
    }
};

int main()
{
    TrigLookup tl;

    std::cout << "SINCOUNT: " << sinCount;

    for (float x : tl.sintable)
        std::cout << x << ", ";
}
