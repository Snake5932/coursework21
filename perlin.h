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

    double get_noise(double x, double y, int period, int vecNum, const int *perm, const double *grad_x, const double *grad_y);

    void create_height_map(uint64_t seed, double** height_map, int size, int startPeriod, int octNum, double attn);
}

#endif