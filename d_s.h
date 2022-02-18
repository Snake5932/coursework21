#include <random>
#include <chrono>
#include "stb_image.h"
#include "stb_image_write.h"

const int width = 257;
const int height = 257;
const int r_width = width - 1;
const int r_height = height - 1;
const int CHANNEL_NUM = 3;
double MAX = 0;
double MIN = 0;
double H = 3;
const double O = 0.7;
const char *const image_name = "D:\\height_map5.png";
//const uint64_t seed = 7890238182113896; //tenting seed
//const uint64_t seed = 2345654245676675634; //tenting seed h = 3; o = 0.7
//const uint64_t seed = 8791820826932084057; //crease h=3, o=0.7
const uint64_t seed = 567667867442890;

void create_pixels(uint8_t *pixels, double **height_map) {
    double amplitude = -1.0 * MIN + MAX;
    for (int i = 0; i < width; i += 1) {
        for (int j = 0; j < height; j += 1) {
            double t_color = (height_map[i][j] + -1 * MIN) / amplitude;
            if (t_color > 1) {
                t_color = 1;
            }
            if (t_color < 0) {
                t_color = 0;
            }
            int color = (int)(t_color * 255);
            pixels[i * height * 3 + j * 3 + 0] = color;
            pixels[i * height * 3 + j * 3 + 1] = color;
            pixels[i * height * 3 + j * 3 + 2] = color;
        }
    }
}

void diamond_square(double **height_map, int n, int level, std::mt19937_64 *rng,
                                                           std::normal_distribution<double> unif) {
    if (level <= 1) {
        return;
    }
    int half_level = level / 2;
    for (int i = 0; i < r_width; i += level) {
        for (int j = 0; j < r_height; j += level) {
            double a = height_map[i][j];
            double b = height_map[i][j + level];
            double c = height_map[i + level][j];
            double d = height_map[i + level][j + level];
            double e = (a + b + c + d) / 4.0;
            double rnd = (1 / pow(sqrt(2), n * H)) * O * unif(*rng);
            height_map[i + half_level][j + half_level] = e + rnd;
            if (height_map[i + half_level][j + half_level] > MAX) {
                MAX = height_map[i + half_level][j + half_level];
            }
        }
    }

    for (int i = 0; i < height; i += half_level) {
        for (int j = (i  + half_level) % level; j < height; j += level) {
            double a = 0, b = 0, c = 0, d = 0, e = 0;
            if (i == 0) {
                a = height_map[i + half_level][j];
                c = height_map[i][j + half_level];
                d = height_map[i][j -  half_level];
                e = (a + c + d) / 3.0;
            } else if (i == r_width) {
                b = height_map[i -  half_level][j];
                c = height_map[i][j + half_level];
                d = height_map[i][j -  half_level];
                e = (b + c + d) / 3.0;
            } else if (j == 0) {
                a = height_map[i + half_level][j];
                b = height_map[i -  half_level][j];
                c = height_map[i][j + half_level];
                e = (a + b + c) / 3.0;
            } else if (j == r_height) {
                a = height_map[i + half_level][j];
                b = height_map[i -  half_level][j];
                d = height_map[i][j -  half_level];
                e = (a + b + d) / 3.0;
            } else {
                a = height_map[i + half_level][j];
                b = height_map[i -  half_level][j];
                c = height_map[i][j + half_level];
                d = height_map[i][j -  half_level];
                e = (a + b + c + d) / 4.0;
            }
            double rnd = (1 / pow(sqrt(2), n * H)) * O * unif(*rng);
            height_map[i][j] = e + rnd;
            if (height_map[i][j] > MAX) {
                MAX = height_map[i][j];
            }
        }
    }
    diamond_square(height_map, n + 1, half_level, rng, unif);
}

void gen_d_s() {
    std::mt19937_64 rng;
    //uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
    rng.seed(ss);
    std::normal_distribution<double> unif(0, 1);
    auto **height_map = new double*[width];
    for (int i = 0; i < width; i++) {
        height_map[i] =  new double[height];
    }

    height_map[0][0] = unif(rng);
    height_map[0][height - 1] = unif(rng);
    height_map[width - 1][0] = unif(rng);
    height_map[width - 1][height - 1] = unif(rng);
    double t_MAX = std::max(std::max(height_map[0][0], height_map[0][height - 1]),
                   std::max(height_map[width - 1][0], height_map[width - 1][height - 1]));
    if (t_MAX > MAX) {
        MAX = t_MAX;
    }
    double t_MIN = std::min(std::min(height_map[0][0], height_map[0][height - 1]),
                            std::min(height_map[width - 1][0], height_map[width - 1][height - 1]));
    if (t_MIN < MIN) {
        MIN = t_MIN;
    }
    diamond_square(height_map, 1, r_width, &rng, unif);

    auto* pixels = new uint8_t[width * height * CHANNEL_NUM];
    create_pixels(pixels, height_map);
    stbi_write_png(image_name, width, height, CHANNEL_NUM, pixels, width * CHANNEL_NUM);
    delete[] pixels;
    for (int i = 0; i < width; i++) {
        delete[] height_map[i];
    }
    delete[] height_map;
}