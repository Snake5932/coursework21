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

double perlin::get_noise(double x, double y, int period, int vecNum, int *perm, double *grad_x, double *grad_y) {
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

void perlin::create_height_map(uint64_t seed, double** height_map, int size, int startPeriod, int octNum, double attn, bool useSin) {
    auto *arr = new int[1024];
    for (int i = 0; i < 512; i++) {
        arr[i] = i;
    }
    std::mt19937_64 rng;
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
    rng.seed(ss);
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
                if (useSin) {
                    res += std::abs(get_noise(i, j, periods[oct], 16, arr, grx, gry) * octAttn[oct]);
                } else {
                    res += get_noise(i, j, periods[oct], 16, arr, grx, gry) * octAttn[oct];
                }
            }
            res /= octAcc;
            if (useSin) {
                res = sin(res);
                res = (res + 1) / 2;
                res = fade(res);
            } else {
                res = (res + 1) / 2;
            }
            height_map[i][j] = float(res) * 255;
        }
    }
    delete[] periods;
    delete[] octAttn;
    delete[] grx;
    delete[] gry;
    delete[] arr;
}

//grad from here: http://riven8192.blogspot.com/2010/08/calculate-perlinnoise-twice-as-fast.html
double perlin::grad(int hash, double x, double y, double z)
{
    switch(hash & 0xF)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x;
        case 0xD: return -y + z;
        case 0xE: return  y - x;
        case 0xF: return -y - z;
        default: return 0;
    }
}

double perlin::get_noise3d(double x, double y, double z, int period, int *perm) {
    int cell_x = floor(x / period);
    int cell_y = floor(y / period);
    int cell_z = floor(z / period);
    double rel_x = (x - cell_x * period) / period;
    double rel_y = (y - cell_y * period) / period;
    double rel_z = (z - cell_z * period) / period;
    double u = fade(rel_x);
    double v = fade(rel_y);
    double k = fade(rel_z);
    int aaa = perm[perm[perm[cell_x] + cell_y] + cell_z];
    int aab = perm[perm[perm[cell_x] + cell_y] + cell_z + 1];
    int aba = perm[perm[perm[cell_x] + cell_y + 1] + cell_z];
    int abb = perm[perm[perm[cell_x] + cell_y + 1] + cell_z + 1];
    int baa = perm[perm[perm[cell_x + 1] + cell_y] + cell_z];
    int bab = perm[perm[perm[cell_x + 1] + cell_y] + cell_z + 1];
    int bba = perm[perm[perm[cell_x + 1] + cell_y + 1] + cell_z];
    int bbb = perm[perm[perm[cell_x + 1] + cell_y + 1] + cell_z + 1];
    double dot1 = lerp(grad (aaa, rel_x, rel_y, rel_z), grad (baa, rel_x - 1, rel_y, rel_z), u);
    double dot2 = lerp(grad (aba, rel_x, rel_y - 1, rel_z), grad (bba, rel_x - 1, rel_y - 1, rel_z), u);
    double dot3 = lerp(grad (aab, rel_x  , rel_y, rel_z - 1), grad (bab, rel_x - 1, rel_y, rel_z - 1), u);
    double dot4 = lerp(grad (abb, rel_x, rel_y - 1, rel_z - 1), grad (bbb, rel_x - 1, rel_y - 1, rel_z - 1), u);
    double res = lerp (lerp(dot1, dot2, v), lerp (dot3, dot4, v), k);
    return res / (sqrt(3) / 2);
}

void perlin::create_noise3d(uint64_t seed, double*** map, int size, int startPeriod, int octNum, double attn) {
    auto *arr = new int[1024];
    for (int i = 0; i < 512; i++) {
        arr[i] = i;
    }
    std::mt19937_64 rng;
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
    rng.seed(ss);
    shuffle(arr, 512, &rng);
    duplicate(arr, 512);
    auto *octAttn = new double[octNum];
    auto *periods = new int[octNum];
    double octAcc = 0;
    for (int oct = 0; oct < octNum; oct++) {
        octAttn[oct] = pow(attn, oct);
        octAcc += octAttn[oct];
        periods[oct] = startPeriod * pow(0.5, oct);
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                double res = 0;
                for (int oct = 0; oct < octNum; oct++) {
                    res += get_noise3d(i, j, k, periods[oct], arr) * octAttn[oct];
                }
                res /= octAcc;
                res = (res + 1) / 2;
                map[i][j][k] = float(res) * 255;
            }
        }
    }
    delete[] periods;
    delete[] octAttn;
    delete[] arr;
}