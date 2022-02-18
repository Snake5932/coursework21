#ifndef PERLIN_H
#define PERLIN_H

#include <random>

namespace perlin {
    double fade(double t);

    double lerp(double t, double a, double b);

    void swap(int i, int j, int *perm);

    void duplicate(int *perm, int n);

    void shuffle(int *perm, int n, std::mt19937_64 *rng);

    void make_vec(double *grad_x, double *grad_y, int n);

    double dot(double x1, double y1, double x2, double y2);

    double get_noise(double x, double y, int period, int vecNum, int *perm, double *grad_x, double *grad_y);

    double grad(int hash, double x, double y, double z);

    double get_noise3d(double x, double y, double z, int period, int *perm);

    void create_noise3d(uint64_t seed, double*** map, int size, int startPeriod, int octNum, double attn);

    void create_height_map(uint64_t seed, double** height_map, int size, int startPeriod, int octNum, double attn, bool useSin);
}

#endif