#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "perlin.h"
#include "loader.h"
#define PI 3.1415926535897932

GLint WIDTH = 950;
GLint HEIGHT = 950;

bool wasdsc[6] = {false, false, false, false, false, false};
int cage = 1;
float speed = 0;
float delta = 0;
float rotVCoef = 0.1;
float basicSpeed = 100;

double oldXC = double(WIDTH) / 2;
double curXC = double(WIDTH) / 2;
double oldYC = double(HEIGHT) / 2;
double curYC = double(HEIGHT) / 2;
bool cursorUpd = false;

float prevTime = 0;
float curTime = 0;

int size = 1024;
int terrainScale = 2;

float grassVertices[] = {
        -0.5f,  0.5f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.0f,  1.0f,  1.0f,

        -0.5f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.0f,  1.0f,  0.0f,

        -0.25f,  0.5f,  -0.43f,  0.0f,  0.0f,
        -0.25f, -0.5f,  -0.43f,  0.0f,  1.0f,
        0.25f, -0.5f,  0.43f,  1.0f,  1.0f,

        -0.25f,  0.5f,  -0.43f,  0.0f,  0.0f,
        0.25f, -0.5f,  0.43f,  1.0f,  1.0f,
        0.25f,  0.5f,  0.43f,  1.0f,  0.0f,

        0.25f,  0.5f,  -0.43f,  0.0f,  0.0f,
        0.25f, -0.5f,  -0.43f,  0.0f,  1.0f,
        -0.25f, -0.5f,  0.43f,  1.0f,  1.0f,

        0.25f,  0.5f,  -0.43f,  0.0f,  0.0f,
        -0.25f, -0.5f,  0.43f,  1.0f,  1.0f,
        -0.25f,  0.5f,  0.43f,  1.0f,  0.0f
};

float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
};

load::shaderSource shaders[] = {load::shaderSource{R"(C:\CLionProjects\shaders\shaders\shader.vert)",
                                       R"(C:\CLionProjects\shaders\shaders\shader.frag)"},
                                load::shaderSource{R"(C:\CLionProjects\shaders\shaders\skyshader.vert)",
                                               R"(C:\CLionProjects\shaders\shaders\skyshader.frag)"},
                                load::shaderSource{R"(C:\CLionProjects\shaders\shaders\grassshader.vert)",
                                                   R"(C:\CLionProjects\shaders\shaders\grassshader.frag)"}};

glm::mat4 rModel = glm::mat4(1.0f);
glm::mat4 frustum = glm::perspective(glm::radians(60.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 4000.0f);
struct camera {
    GLfloat posX;
    GLfloat posY;
    GLfloat posZ;
    GLfloat stdDirX;
    GLfloat stdDirY;
    GLfloat stdDirZ;
    GLfloat stdRX;
    GLfloat stdRY;
    GLfloat stdRZ;
    GLfloat yaw;
    GLfloat pitch;
};

camera camera{110, 150, 110,
              0, 0, 1,
              -1, 0, 0,
              90, 0};


void multMatrVec(const glm::mat4 m1, const GLfloat v[4], GLfloat vr[4]) {
    for (int i = 0; i < 4; i++) {
        vr[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            vr[j] += m1[i][j] * v[i];
        }
    }
}

void cross(const GLfloat v1[3], const GLfloat v2[3], GLfloat vr[3]) {
    vr[0] = v1[1] * v2[2] - v1[2] * v2[1];
    vr[1] = -1 * (v1[0] * v2[2] - v1[2] * v2[0]);
    vr[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

int getCoord(int x) {
    return (x + size) % size;
}

glm::vec3 getNormal(int i, int j, double **height_map) {
    glm::vec3 v1 = glm::normalize(glm::cross(glm::vec3{terrainScale, height_map[getCoord(i + 1)][j], 0},
                                                glm::vec3{0, height_map[i][getCoord(j + 1)], terrainScale}));
    glm::vec3 v2 = glm::normalize(glm::cross(glm::vec3{0, height_map[i][getCoord(j + 1)], terrainScale},
                                                glm::vec3{-terrainScale, height_map[getCoord(i - 1)][j], 0}));
    glm::vec3 v3 = glm::normalize(glm::cross(glm::vec3{-terrainScale, height_map[getCoord(i - 1)][j], 0},
                                                glm::vec3{0, height_map[i][getCoord(j - 1)], -terrainScale}));
    glm::vec3 v4 = glm::normalize(glm::cross(glm::vec3{0, height_map[i][getCoord(j - 1)], -terrainScale},
                                                glm::vec3{terrainScale, height_map[getCoord(i + 1)][j], 0}));
    return glm::normalize(v1 + v2 + v3 + v4);
}

GLfloat vLength(const GLfloat v[3]) {
    GLfloat len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return len;
}

GLfloat dot(const GLfloat v1[3], const GLfloat v2[3]) {
    GLfloat angle = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
    angle = angle / (vLength(v1) * vLength(v2));
    return angle;
}

void normalise(GLfloat v[3]) {
    GLfloat len = vLength(v);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}

glm::mat4 lookAt(GLfloat Ax, GLfloat Ay, GLfloat Az, GLfloat Cx, GLfloat Cy, GLfloat Cz, GLfloat Ux, GLfloat Uy, GLfloat Uz) {
    GLfloat vb[3] = {Cx - Ax, Cy - Ay, Cz - Az};
    GLfloat vu[3] = {Ux, Uy, Uz};
    GLfloat vr[3] = {0};
    cross(vu, vb, vr);
    cross(vb, vr, vu);
    GLfloat vx[3] = {1, 0, 0};
    GLfloat vy[3] = {0, 1, 0};

    GLfloat k1 = 0;
    GLfloat tx[3] = {0};
    cross(vy, vu, tx);
    if (dot(tx, vr) < 0) {
        k1 = 1;
    } else {
        k1 = -1;
    }
    GLfloat k2 = 0;
    GLfloat ty[3] = {0};
    cross(vx, vr, ty);
    if (dot(ty, vu) < 0) {
        k2 = 1;
    } else {
        k2 = -1;
    }
    glm::mat4 lookAtM = glm::mat4(1.0f);
    lookAtM = glm::rotate(lookAtM, glm::radians(float(k1 * 180 * acosf(dot(vy, vu)) / PI)), glm::vec3(1.0, 0.0, 0.0));
    lookAtM = glm::rotate(lookAtM, glm::radians(float(k2 * 180 * acosf(dot(vx, vr)) / PI)), glm::vec3(0.0, 1.0, 0.0));
    lookAtM = glm::translate(lookAtM, glm::vec3(-Cx, -Cy, -Cz));
    return lookAtM;
}

void updCamera() {
    if (cursorUpd) {
        camera.yaw += (curXC - oldXC) * rotVCoef;
        camera.pitch -= (curYC - oldYC) * rotVCoef;
        if (camera.yaw < -180) {
            camera.yaw = 180;
        }
        if (camera.yaw > 180) {
            camera.yaw = -180;
        }
        if (camera.pitch < -80) {
            camera.pitch = -80;
        }
        if (camera.pitch > 80) {
            camera.pitch = 80;
        }
    }
    GLfloat v1[3] = {0.0, 1.0, 0.0};
    GLfloat v2[3] = {camera.stdRX, camera.stdRY, camera.stdRZ};
    GLfloat vm[3];
    cross(v1, v2, vm);
    normalise(vm);
    if (cursorUpd) {
        camera.stdDirX = cosf(PI * camera.yaw / 180) * cosf(PI * camera.pitch / 180);
        camera.stdDirY = sinf(PI * camera.pitch / 180);
        camera.stdDirZ = sinf(PI * camera.yaw / 180) * cosf(PI * camera.pitch / 180);
        GLfloat vd[3] = {camera.stdDirX, camera.stdDirY, camera.stdDirZ};
        normalise(vd);
        camera.stdDirX = vd[0];
        camera.stdDirY = vd[1];
        camera.stdDirZ = vd[2];
        GLfloat vu[3] = {0, 1, 0};
        GLfloat vr[3] = {0};
        cross(vd, vu, vr);
        normalise(vr);
        camera.stdRX = vr[0];
        camera.stdRY = vr[1];
        camera.stdRZ = vr[2];
        cursorUpd = false;
    }
    if (wasdsc[0]) {
        camera.posX += speed * vm[0];
        camera.posZ += speed * vm[2];
    }
    if (wasdsc[1]) {
        camera.posX -= speed * camera.stdRX;
        camera.posZ -= speed * camera.stdRZ;
    }
    if (wasdsc[2]) {
        camera.posX -= speed * vm[0];
        camera.posZ -= speed * vm[2];
    }
    if (wasdsc[3]) {
        camera.posX += speed * camera.stdRX;
        camera.posZ += speed * camera.stdRZ;
    }
    if (wasdsc[4]) {
        camera.posY += speed;
    }
    if (wasdsc[5]) {
        camera.posY -= speed;
    }
}

enum {
    sandColor = 0,
    clayColor,
    earthColor,
    grassColor,
    dirtColor,
    sandClayColor,
    clayEarthColor,
    earthGrassColor,
    earthDirtColor,
};

int getTerrainType(double height, float angle) {
    if (height >= 146 && angle >= 0.998) {
        return sandColor;
    }
    if (height >= 144 && height < 146 && angle >= 0.998) {
        return sandClayColor;
    }
    if (height >= 146 && angle >= 0.978 && angle < 0.998) {
        return sandClayColor;
    }
    if (height >= 144 && height < 146 && angle >= 0.978 && angle < 0.998) {
        return sandClayColor;
    }
    if (height >= 141) {
        return clayColor;
    }
    if (height >= 134 && height < 141) {
        return clayEarthColor;
    }
    if (height < 82) {
        return dirtColor;
    }
    if (height >= 82 && height < 87) {
        return earthDirtColor;
    }
    if (height >= 87 && height < 94) {
        return earthColor;
    }
    if (height >= 94 && height < 100 && angle >= 0.997) {
        return earthGrassColor;
    }
    if (height >= 100 && height < 120 && angle >= 0.977 && angle < 0.997) {
        return earthGrassColor;
    }
    if (height >= 94 && height < 100 && angle >= 0.977 && angle < 0.997) {
        return earthGrassColor;
    }
    if (height >= 100 && height < 120 && angle >= 0.997) {
        return grassColor;
    }
    if (height >= 120 && height < 126 && angle >= 0.997) {
        return earthGrassColor;
    }
    if (height >= 120 && height < 126 && angle >= 0.977 && angle < 0.997) {
        return earthGrassColor;
    }
    return earthColor;
}

float getCoef(double height, int type) {
    float coef = 0;
    if (type == clayEarthColor) {
        coef = float(height - 134) / 7.0f;
    }
    if (type == earthDirtColor) {
        coef = float(height - 82) / 5.0f;
    }
    coef = float(perlin::fade(coef));
    return coef;
}

double getDistance(double x1, double x2, double y1, double y2, double z1, double z2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

int getGrassInstances(GLfloat *vertices, GLfloat *grass_instances, double **height_map, uint64_t seed) {
    std::mt19937_64 rng;
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
    rng.seed(ss);
    std::uniform_real_distribution<float> unif(-1, 1);
    glm::vec3 normal;
    float offset_coef = 2.5;
    int grass_num = 0;
    float rand_offset = 0;
    float rand_type = 0;
    float grass_type = 0;
    float rand_grass = 0;
    float rand_cos = 0;
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {
            normal.x = vertices[i * (size - 1) * 27 + j * 27 + 3 + 0];
            normal.y = vertices[i * (size - 1) * 27 + j * 27 + 3 + 1];
            normal.z = vertices[i * (size - 1) * 27 + j * 27 + 3 + 2];
            int type = getTerrainType(height_map[i][j], std::abs(normal.y));
            if (type != sandColor && type != dirtColor && type != clayColor && type != sandClayColor) {
                rand_type = std::abs(unif(rng));
                if (type == grassColor) {
                    if (rand_type <= 0.95) {
                        rand_offset = unif(rng) / offset_coef;
                        grass_instances[grass_num * 6 + 0] = float(i * terrainScale) + rand_offset;
                        grass_instances[grass_num * 6 + 1] = height_map[i][j];
                        rand_offset = unif(rng) / offset_coef;
                        grass_instances[grass_num * 6 + 2] = float(j * terrainScale) + rand_offset;
                        grass_instances[grass_num * 6 + 3] = earthColor;
                        rand_cos = unif(rng);
                        grass_instances[grass_num * 6 + 4] = rand_cos;
                        grass_instances[grass_num * 6 + 5] = sqrtf(1 - rand_cos * rand_cos);
                        grass_num += 1;
                    }
                }
                if (type == earthColor) {
                    if (rand_type <= 0.95) {
                        rand_offset = unif(rng) / offset_coef;
                        grass_instances[grass_num * 6 + 0] = float(i * terrainScale) + rand_offset;
                        grass_instances[grass_num * 6 + 1] = height_map[i][j];
                        rand_offset = unif(rng) / offset_coef;
                        grass_instances[grass_num * 6 + 2] = float(j * terrainScale) + rand_offset;
                        grass_type = std::abs(unif(rng));
                        if (grass_type < 0.6) {
                            grass_instances[grass_num * 6 + 3] = earthColor;
                        } else {
                            grass_instances[grass_num * 6 + 3] = grassColor;
                        }
                        rand_cos = unif(rng);
                        grass_instances[grass_num * 6 + 4] = rand_cos;
                        grass_instances[grass_num * 6 + 5] = sqrtf(1 - rand_cos * rand_cos);
                        grass_num += 1;
                    }
                }
                if (type == earthGrassColor) {
                    if (rand_type <= 0.95) {
                        rand_offset = unif(rng) / offset_coef;
                        grass_instances[grass_num * 6 + 0] = float(i * terrainScale) + rand_offset;
                        grass_instances[grass_num * 6 + 1] = height_map[i][j];
                        rand_offset = unif(rng) / offset_coef;
                        grass_instances[grass_num * 6 + 2] = float(j * terrainScale) + rand_offset;
                        grass_type = std::abs(unif(rng));
                        if (grass_type > 0.3) {
                            grass_instances[grass_num * 6 + 3] = earthColor;
                        } else {
                            grass_instances[grass_num * 6 + 3] = grassColor;
                        }
                        rand_cos = unif(rng);
                        grass_instances[grass_num * 6 + 4] = rand_cos;
                        grass_instances[grass_num * 6 + 5] = sqrtf(1 - rand_cos * rand_cos);
                        grass_num += 1;
                    }
                }
                if (type == earthDirtColor) {
                    if (rand_type <= 0.95) {
                        rand_grass = std::abs(unif(rng));
                        float coef = getCoef(height_map[i][j], earthDirtColor);
                        if (rand_grass < coef) {
                            rand_offset = unif(rng) / offset_coef;
                            grass_instances[grass_num * 6 + 0] = float(i * terrainScale) + rand_offset;
                            grass_instances[grass_num * 6 + 1] = height_map[i][j];
                            rand_offset = unif(rng) / offset_coef;
                            grass_instances[grass_num * 6 + 2] = float(j * terrainScale) + rand_offset;
                            grass_num += 1;
                            if (rand_grass < coef - 0.2) {
                                grass_type = std::abs(unif(rng));
                                if (grass_type > 0.7) {
                                    grass_instances[grass_num * 6 + 3] = earthColor;
                                } else {
                                    grass_instances[grass_num * 6 + 3] = grassColor;
                                }
                            } else {
                                grass_instances[grass_num * 6 + 3] = earthDirtColor;
                            }
                            rand_cos = unif(rng);
                            grass_instances[grass_num * 6 + 4] = rand_cos;
                            grass_instances[grass_num * 6 + 5] = sqrtf(1 - rand_cos * rand_cos);
                        }
                    }
                }
                if (type == clayEarthColor) {
                    if (rand_type <= 0.95) {
                        rand_grass = std::abs(unif(rng));
                        float coef = getCoef(height_map[i][j], clayEarthColor);
                        if (rand_grass > coef) {
                            rand_offset = unif(rng) / offset_coef;
                            grass_instances[grass_num * 6 + 0] = float(i * terrainScale) + rand_offset;
                            grass_instances[grass_num * 6 + 1] = height_map[i][j];
                            rand_offset = unif(rng) / offset_coef;
                            grass_instances[grass_num * 6 + 2] = float(j * terrainScale) + rand_offset;
                            grass_num += 1;
                            if (rand_grass > coef + 0.2) {
                                grass_type = std::abs(unif(rng));
                                if (grass_type > 0.7) {
                                    grass_instances[grass_num * 6 + 3] = earthColor;
                                } else {
                                    grass_instances[grass_num * 6 + 3] = grassColor;
                                }
                            } else {
                                grass_instances[grass_num * 6 + 3] = clayEarthColor;
                            }
                            rand_cos = unif(rng);
                            grass_instances[grass_num * 6 + 4] = rand_cos;
                            grass_instances[grass_num * 6 + 5] = sqrtf(1 - rand_cos * rand_cos);
                        }
                    }
                }
            }
        }
    }
    return grass_num;
}

void calculateVertices(GLfloat *vertices, double **height_map, double **second_height_map) {
    glm::vec3 normal;
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {
            vertices[i * (size - 1) * 27 + j * 27 + 0 + 0] = i * terrainScale;
            vertices[i * (size - 1) * 27 + j * 27 + 0 + 1] = height_map[i][j];
            vertices[i * (size - 1) * 27 + j * 27 + 0 + 2] = j * terrainScale;
            normal = getNormal(i, j, height_map);
            vertices[i * (size - 1) * 27 + j * 27 + 3 + 0] = normal.x;
            vertices[i * (size - 1) * 27 + j * 27 + 3 + 1] = normal.y;
            vertices[i * (size - 1) * 27 + j * 27 + 3 + 2] = normal.z;
            vertices[i * (size - 1) * 27 + j * 27 + 6 + 0] = second_height_map[i][j];
            vertices[i * (size - 1) * 27 + j * 27 + 7 + 0] = i / 2.0;
            vertices[i * (size - 1) * 27 + j * 27 + 7 + 1] = j / 2.0;

            vertices[i * (size - 1) * 27 + j * 27 + 9 + 0] = i * terrainScale;
            vertices[i * (size - 1) * 27 + j * 27 + 9 + 1] = height_map[i][j + 1];
            vertices[i * (size - 1) * 27 + j * 27 + 9 + 2] = (j + 1) * terrainScale;
            normal = getNormal(i, j + 1, height_map);
            vertices[i * (size - 1) * 27 + j * 27 + 12 + 0] = normal.x;
            vertices[i * (size - 1) * 27 + j * 27 + 12 + 1] = normal.y;
            vertices[i * (size - 1) * 27 + j * 27 + 12 + 2] = normal.z;
            vertices[i * (size - 1) * 27 + j * 27 + 15 + 0] = second_height_map[i][j + 1];
            vertices[i * (size - 1) * 27 + j * 27 + 16 + 0] = i / 2.0;
            vertices[i * (size - 1) * 27 + j * 27 + 16 + 1] = (j + 1) / 2.0;

            vertices[i * (size - 1) * 27 + j * 27 + 18 + 0] = (i + 1) * terrainScale;
            vertices[i * (size - 1) * 27 + j * 27 + 18 + 1] = height_map[i + 1][j];
            vertices[i * (size - 1) * 27 + j * 27 + 18 + 2] = j * terrainScale;
            normal = getNormal(i + 1, j, height_map);
            vertices[i * (size - 1) * 27 + j * 27 + 21 + 0] = normal.x;
            vertices[i * (size - 1) * 27 + j * 27 + 21 + 1] = normal.y;
            vertices[i * (size - 1) * 27 + j * 27 + 21 + 2] = normal.z;
            vertices[i * (size - 1) * 27 + j * 27 + 24 + 0] = second_height_map[i + 1][j];
            vertices[i * (size - 1) * 27 + j * 27 + 25 + 0] = (i + 1) / 2.0;
            vertices[i * (size - 1) * 27 + j * 27 + 25 + 1] = j / 2.0;

            int step = (size - 1) * (size - 1) * 3 * 9;

            vertices[step + i * (size - 1) * 27 + j * 27 + 0 + 0] = (i + 1) * terrainScale;
            vertices[step + i * (size - 1) * 27 + j * 27 + 0 + 1] = height_map[i + 1][j + 1];
            vertices[step + i * (size - 1) * 27 + j * 27 + 0 + 2] = (j + 1) * terrainScale;
            normal = getNormal(i + 1, j + 1, height_map);
            vertices[step + i * (size - 1) * 27 + j * 27 + 3 + 0] = normal.x;
            vertices[step + i * (size - 1) * 27 + j * 27 + 3 + 1] = normal.y;
            vertices[step + i * (size - 1) * 27 + j * 27 + 3 + 2] = normal.z;
            vertices[step + i * (size - 1) * 27 + j * 27 + 6 + 0] = second_height_map[i + 1][j + 1];
            vertices[step + i * (size - 1) * 27 + j * 27 + 7 + 0] = (i + 1) / 2.0;
            vertices[step + i * (size - 1) * 27 + j * 27 + 7 + 1] = (j + 1) / 2.0;

            vertices[step + i * (size - 1) * 27 + j * 27 + 9 + 0] = (i + 1) * terrainScale;
            vertices[step + i * (size - 1) * 27 + j * 27 + 9 + 1] = height_map[i + 1][j];
            vertices[step + i * (size - 1) * 27 + j * 27 + 9 + 2] = j * terrainScale;
            normal = getNormal(i + 1, j, height_map);
            vertices[step + i * (size - 1) * 27 + j * 27 + 12 + 0] = normal.x;
            vertices[step + i * (size - 1) * 27 + j * 27 + 12 + 1] = normal.y;
            vertices[step + i * (size - 1) * 27 + j * 27 + 12 + 2] = normal.z;
            vertices[step + i * (size - 1) * 27 + j * 27 + 15 + 0] = second_height_map[i + 1][j];
            vertices[step + i * (size - 1) * 27 + j * 27 + 16 + 0] = (i + 1) / 2.0;
            vertices[step + i * (size - 1) * 27 + j * 27 + 16 + 1] = j / 2.0;

            vertices[step + i * (size - 1) * 27 + j * 27 + 18 + 0] = i * terrainScale;
            vertices[step + i * (size - 1) * 27 + j * 27 + 18 + 1] = height_map[i][j + 1];
            vertices[step + i * (size - 1) * 27 + j * 27 + 18 + 2] = (j + 1) * terrainScale;
            normal = getNormal(i, j + 1, height_map);
            vertices[step + i * (size - 1) * 27 + j * 27 + 21 + 0] = normal.x;
            vertices[step + i * (size - 1) * 27 + j * 27 + 21 + 1] = normal.y;
            vertices[step + i * (size - 1) * 27 + j * 27 + 21 + 2] = normal.z;
            vertices[step + i * (size - 1) * 27 + j * 27 + 24 + 0] = second_height_map[i][j + 1];
            vertices[step + i * (size - 1) * 27 + j * 27 + 25 + 0] = i / 2.0;
            vertices[step + i * (size - 1) * 27 + j * 27 + 25 + 1] = (j + 1) / 2.0;
        }
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    cursorUpd = true;
    oldXC = curXC;
    oldYC = curYC;
    curXC = xpos;
    curYC = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        cage *= -1;
        if (cage < 0) {
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glEnable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        wasdsc[0] = true;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        wasdsc[0] = false;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        wasdsc[1] = true;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        wasdsc[1] = false;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        wasdsc[2] = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        wasdsc[2] = false;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        wasdsc[3] = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        wasdsc[3] = false;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        wasdsc[4] = true;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
        wasdsc[4] = false;
    }
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
        wasdsc[5] = true;
    }
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
        wasdsc[5] = false;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

int main() {
    //uint64_t seed = 54678779080;
    uint64_t seed = 34564758975;
    auto height_map = new double*[size];
    auto second_height_map = new double*[size];
    for (int i = 0; i < size; i++) {
        height_map[i] = new double[size];
        second_height_map[i] = new double[size];
    }
    bool isSecondMap = false;
    perlin::create_height_map(seed, height_map, size, 512, 6, 0.5, isSecondMap);
    isSecondMap = true;
    perlin::create_height_map(seed, second_height_map, size, 512, 6, 0.5, isSecondMap);
    auto *vertices = new GLfloat[(size - 1) * (size - 1) * 2 * 3 * 9];
    auto *grass_instances = new GLfloat[(size - 1) * (size - 1) * 6];
    calculateVertices(vertices, height_map, second_height_map);
    int grassNum = getGrassInstances(vertices, grass_instances, height_map, seed);

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glClearColor(0, 0, 0, 1);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    int shaderNum = sizeof(shaders)/sizeof(shaders[0]);
    auto shaderPrograms = new int[shaderNum];
    load::load_shaders(shaders, shaderNum, shaderPrograms);


    unsigned int clay = load::load_texture(R"(C:\CLionProjects\shaders\textures\clay_texture.jpg)");
    unsigned int dirt = load::load_texture(R"(C:\CLionProjects\shaders\textures\dirt_texture.jpg)");
    unsigned int earth = load::load_texture(R"(C:\CLionProjects\shaders\textures\earth_texture.jpg)");
    unsigned int sand = load::load_texture(R"(C:\CLionProjects\shaders\textures\sand_texture.jpg)");
    unsigned int grass = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture.jpg)");
    unsigned int grass3 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture0.png)");
    unsigned int grass4 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture4.png)");
    unsigned int grass5 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture5.png)");
    unsigned int grass6 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture6.png)");

    unsigned int VBO_T, VBO_S, VAO_T, VAO_S;
    glGenVertexArrays(1, &VAO_T);
    glGenVertexArrays(1, &VAO_S);
    glGenBuffers(1, &VBO_T);
    glGenBuffers(1, &VBO_S);

    glBindVertexArray(VAO_S);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO_T);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_T);
    glBufferData(GL_ARRAY_BUFFER, (size - 1) * 2 * (size - 1) * 3 * 9 * sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int VBO_instanced, VBO_G, VAO_G;

    glGenBuffers(1, &VBO_instanced);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_instanced);
    glBufferData(GL_ARRAY_BUFFER, (size - 1) * (size - 1) * 6 * sizeof(float), grass_instances, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &VAO_G);
    glGenBuffers(1, &VBO_G);

    glBindVertexArray(VAO_G);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_G);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_instanced);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shaderPrograms[0]);
    glUniform1i(glGetUniformLocation(shaderPrograms[0], "grassTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderPrograms[0], "earthTexture"), 1);
    glUniform1i(glGetUniformLocation(shaderPrograms[0], "dirtTexture"), 2);
    glUniform1i(glGetUniformLocation(shaderPrograms[0], "clayTexture"), 3);
    glUniform1i(glGetUniformLocation(shaderPrograms[0], "sandTexture"), 4);

    glUseProgram(shaderPrograms[2]);
    glUniform1i(glGetUniformLocation(shaderPrograms[2], "grassTexture2"), 5);
    glUniform1i(glGetUniformLocation(shaderPrograms[2], "grassTexture3"), 6);
    glUniform1i(glGetUniformLocation(shaderPrograms[2], "grassTexture5"), 7);
    glUniform1i(glGetUniformLocation(shaderPrograms[2], "grassTexture6"), 8);


    float fps[1000];
    int count = 0;
    float theta = 0.0;
    float sun_y, sun_z;
    while (!glfwWindowShouldClose(window)) {
        curTime = float(glfwGetTime());
        delta = curTime - prevTime;
        prevTime = curTime;
        speed = basicSpeed * delta;
        theta += 0.0125f * delta;
        sun_z = cosf(2.0f * float(PI) * theta);
        sun_y = sinf(2.0f * float(PI) * theta);

        if (count < 1000) {
            fps[count] = 1 / delta;
            count += 1;
        } else {
            if (count >= 1000) {

                std::cout << 1 / delta << " over" << std::endl;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);
        glm::mat4 lookAtM = lookAt(camera.posX + camera.stdDirX, camera.posY + camera.stdDirY, camera.posZ + camera.stdDirZ,
                                       camera.posX, camera.posY, camera.posZ, 0, 1, 0);

        glUseProgram(shaderPrograms[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, earth);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, dirt);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, clay);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, sand);

        int rMod = glGetUniformLocation(shaderPrograms[0], "rModel");
        glUniformMatrix4fv(rMod, 1, GL_FALSE, glm::value_ptr(rModel));
        int proj = glGetUniformLocation(shaderPrograms[0], "projection");
        glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
        int la = glGetUniformLocation(shaderPrograms[0], "lookAt");
        glUniformMatrix4fv(la, 1, GL_FALSE, glm::value_ptr(lookAtM));
        int mo = glGetUniformLocation(shaderPrograms[0], "model");
        glUniformMatrix4fv(mo, 1, GL_FALSE, glm::value_ptr(glm::mat4{1.0f}));
        int light = glGetUniformLocation(shaderPrograms[0], "light");
        glUniform3f(light, 0.0, sun_y, sun_z);
        int camPos = glGetUniformLocation(shaderPrograms[0], "camPos");
        glUniform3f(camPos, camera.posX, camera.posY, camera.posZ);
        glBindVertexArray(VAO_T);
        glDrawArrays(GL_TRIANGLES, 0, (size - 1) * 2 * (size - 1) * 3);

        glUseProgram(shaderPrograms[2]);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, grass4);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, grass3);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, grass5);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, grass6);

        glDisable(GL_CULL_FACE);
        proj = glGetUniformLocation(shaderPrograms[2], "projection");
        glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
        la = glGetUniformLocation(shaderPrograms[2], "lookAt");
        glUniformMatrix4fv(la, 1, GL_FALSE, glm::value_ptr(lookAtM));
        light = glGetUniformLocation(shaderPrograms[2], "light");
        glUniform3f(light, 0.0, sun_y, sun_z);
        camPos = glGetUniformLocation(shaderPrograms[2], "camPos");
        glUniform3f(camPos, camera.posX, camera.posY, camera.posZ);
        int time = glGetUniformLocation(shaderPrograms[2], "deltaTime");
        glUniform1f(time, delta);
        glBindVertexArray(VAO_G);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 18, grassNum);
        glEnable(GL_CULL_FACE);

        glDepthFunc(GL_LEQUAL);
        glUseProgram(shaderPrograms[1]);
        proj = glGetUniformLocation(shaderPrograms[1], "projection");
        glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
        la = glGetUniformLocation(shaderPrograms[1], "lookAt");
        glUniformMatrix4fv(la, 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(lookAtM))));
        int sun = glGetUniformLocation(shaderPrograms[1], "uSunPos");
        glUniform3f(sun, 0.0, sun_y, sun_z);
        glBindVertexArray(VAO_S);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);
        glDisable(GL_CULL_FACE);
        if (cage > 0) {
            glEnable(GL_CULL_FACE);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
        updCamera();
    }

    float accum = 0;
    for (float fp : fps) {
        accum += fp;
    }
    std::cout << "\n" << accum / 1000;

    glDeleteVertexArrays(1, &VAO_T);
    glDeleteBuffers(1, &VBO_T);
    glDeleteVertexArrays(1, &VAO_S);
    glDeleteBuffers(1, &VBO_S);
    glDeleteVertexArrays(1, &VAO_G);
    glDeleteBuffers(1, &VBO_G);
    glDeleteBuffers(1, &VBO_instanced);
    glfwTerminate();
    delete[] vertices;
    delete[] grass_instances;
    for (int i = 0; i < size; i++) {
        delete[] height_map[i];
        delete[] second_height_map[i];
    }
    delete[] height_map;
    delete[] second_height_map;
    delete[] shaderPrograms;
    return 0;
}