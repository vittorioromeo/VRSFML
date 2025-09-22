#include <cstddef>

struct Particle
{
    float px, py;
    float vx, vy;
    float ax, ay;
    float scale;
    float opacity;
    float rotation;
    float scaleChange;
    float opacityChange;
    float rotationChange;
};

void update_aos(Particle* __restrict p, std::size_t n, float dt)
{
    for (std::size_t i = 0; i < n; ++i)
    {
        p[i].px += p[i].vx * dt;
        p[i].py += p[i].vy * dt;
        p[i].vx += p[i].ax * dt;
        p[i].vy += p[i].ay * dt;
        p[i].scale += p[i].scaleChange * dt;
        p[i].opacity += p[i].opacityChange * dt;
        p[i].rotation += p[i].rotationChange * dt;
    }
}

void update_soa(float* __restrict px,
                float* __restrict py,
                float* __restrict vx,
                float* __restrict vy,
                const float* __restrict ax,
                const float* __restrict ay,
                float* __restrict scale,
                float* __restrict opacity,
                float* __restrict rotation,
                const float* __restrict scaleChange,
                const float* __restrict opacityChange,
                const float* __restrict rotationChange,
                std::size_t n,
                float       dt)
{
    for (std::size_t i = 0; i < n; ++i)
    {
        px[i] += vx[i] * dt;
        py[i] += vy[i] * dt;
        vx[i] += ax[i] * dt;
        vy[i] += ay[i] * dt;
        scale[i] += scaleChange[i] * dt;
        opacity[i] += opacityChange[i] * dt;
        rotation[i] += rotationChange[i] * dt;
    }
}
