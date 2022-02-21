#include <cmath>
#include <random>
#include "perlin.h"

double perlin::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double perlin::lerp(double t, double a, double b) {
    return a + t * (b - a);
}

void perlin::swap(int i, int j, int *perm) {
    int t = perm[i];
    perm[i] = perm[j];
    perm[j] = t;
}

void perlin::duplicate(int *perm, int n) {
    for (int i = 0; i < n; i++) {
        perm[i + n] = perm[i];
    }
}

void perlin::shuffle(int *perm, int n, std::mt19937_64 *rng) {
    for (int i = n - 1; i > 0; i--) {
        std::uniform_int_distribution<int> unif(0, i);
        int j = unif(*rng);
        swap(i, j, perm);
    }
}

void perlin::make_vec(double *grad_x, double *grad_y, int n) {
    for (int i = 0; i < n; i++) {
        grad_x[i] = cos(2.0 * M_PI * i / n);
        grad_y[i] = sin(2.0 * M_PI * i / n);
    }
}

double perlin::dot(double x1, double y1, double x2, double y2) {
    return x1 * x2 + y1 * y2;
}

double perlin::get_noise(double x, double y, int period, int vecNum, const int *perm, const double *grad_x, const double *grad_y) {
    int cell_x = floor(x / period);
    int cell_y = floor(y / period);
    double rel_x = (x - cell_x * period) / period;
    double rel_y = (y - cell_y * period) / period;
    double u = fade(rel_x);
    double v = fade(rel_y);
    int aa = perm[perm[cell_x] + cell_y];
    int ab = perm[perm[cell_x + 1] + cell_y];
    int ba = perm[perm[cell_x] + cell_y + 1];
    int bb = perm[perm[cell_x + 1] + cell_y + 1];
    double dot_aa = dot(grad_x[aa % vecNum], grad_y[aa % vecNum], u, v);
    double dot_ab = dot(grad_x[ab % vecNum], grad_y[ab % vecNum], u - 1, v);
    double dot_ba = dot(grad_x[ba % vecNum], grad_y[ba % vecNum], u, v - 1);
    double dot_bb = dot(grad_x[bb % vecNum], grad_y[bb % vecNum], u - 1, v - 1);
    double res = lerp(v, lerp(u, dot_aa, dot_ab), lerp(u, dot_ba, dot_bb));
    return res / (sqrt(2) / 2);
}

void perlin::create_height_map(uint64_t seed, double** height_map, int size, int startPeriod, int octNum, double attn) {
    auto *arr = new int[1024];
    for (int i = 0; i < 512; i++) {
        arr[i] = i;
    }
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed >> 32u)};
    std::mt19937_64 rng(ss);
    shuffle(arr, 512, &rng);
    duplicate(arr, 512);
    auto *grx = new double[16];
    auto *gry = new double[16];
    auto *octAttn = new double[octNum];
    auto *periods = new int[octNum];
    double octAcc = 0;
    for (int oct = 0; oct < octNum; oct++) {
        octAttn[oct] = pow(attn, oct);
        octAcc += octAttn[oct];
        periods[oct] = startPeriod * pow(0.5, oct);
    }
    make_vec(grx, gry, 16);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double res = 0;
            for (int oct = 0; oct < octNum; oct++) {
                res += get_noise(i, j, periods[oct], 16, arr, grx, gry) * octAttn[oct];
            }
            res /= octAcc;
            res = (res + 1) / 2;
            height_map[i][j] = float(res) * 255;
        }
    }
    delete[] periods;
    delete[] octAttn;
    delete[] grx;
    delete[] gry;
    delete[] arr;
}