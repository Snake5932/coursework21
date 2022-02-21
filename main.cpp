#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "perlin.h"
#include "loader.h"

int screenWidth = 1920;
int screenHeight = 1080;
int texResolutionWidth = screenHeight / 1;
int texResolutionHeight = screenHeight / 1;

bool wasdsc[6] = {false, false, false, false, false, false};
float speed = 0;
float delta = 0;
float rotVCoef = 0.1;
float basicSpeed = 100;
float waterHeight = 85;
float basicWaveSpeed = 0.02;
float waveSpeed = 0;
float sun_y, sun_z;
int grassNum;
float glitchOffset = 0.1;

double oldXC = double(screenWidth) / 2;
double curXC = double(screenWidth) / 2;
double oldYC = double(screenHeight) / 2;
double curYC = double(screenHeight) / 2;
bool cursorUpd = false;
glm::mat4 lookAtM;
glm::mat4 lookAtRefl;

unsigned int VBO_S, VAO_S, VBO_instanced, VBO_G, VAO_G, VBO_T, VAO_T, VBO_W, VAO_W,
             refractionFBO, refractionTexture, refractionDepthTexture,
             reflectionFBO, reflectionTexture, reflectionDepthFBO;
int proj, la_ter, light_ter, camPos_ter, camDir_ter, la_gr, light_gr, camPos_gr, camDir_gr, time_gr,
    sun_s, la_s, camDir_s, la_w, camPos_w, clip_ter, clip_gr, mapSize_w, stride_w, light_w;

float prevTime = 0;
float curTime = 0;

int size = 1024;
int terrainScale = 2;

float grassVertices[] = {
        -0.5,  0.5,  0.0,  0.0,  0.0,
        -0.5, -0.5,  0.0,  0.0,  1.0,
        0.5, -0.5,  0.0,  1.0,  1.0,

        -0.5,  0.5,  0.0,  0.0,  0.0,
        0.5, -0.5,  0.0,  1.0,  1.0,
        0.5,  0.5,  0.0,  1.0,  0.0,

        -0.25,  0.5,  -0.43,  0.0,  0.0,
        -0.25, -0.5,  -0.43,  0.0,  1.0,
        0.25, -0.5,  0.43,  1.0,  1.0,

        -0.25,  0.5,  -0.43,  0.0,  0.0,
        0.25, -0.5,  0.43,  1.0,  1.0,
        0.25,  0.5,  0.43,  1.0,  0.0,

        0.25,  0.5,  -0.43,  0.0,  0.0,
        0.25, -0.5,  -0.43,  0.0,  1.0,
        -0.25, -0.5,  0.43,  1.0,  1.0,

        0.25,  0.5,  -0.43,  0.0,  0.0,
        -0.25, -0.5,  0.43,  1.0,  1.0,
        -0.25,  0.5,  0.43,  1.0,  0.0
};

float skyboxVertices[] = {
        -1.0,  1.0, -1.0,
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0,

        -1.0, -1.0,  1.0,
        -1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0,
        -1.0,  1.0,  1.0,
        -1.0, -1.0,  1.0,

        1.0, -1.0, -1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
        1.0,  1.0,  1.0,
        1.0,  1.0, -1.0,
        1.0, -1.0, -1.0,

        -1.0, -1.0,  1.0,
        -1.0,  1.0,  1.0,
        1.0,  1.0,  1.0,
        1.0,  1.0,  1.0,
        1.0, -1.0,  1.0,
        -1.0, -1.0,  1.0,

        -1.0,  1.0, -1.0,
        1.0,  1.0, -1.0,
        1.0,  1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0
};

float waterVertices[] = {
        0, 85.0, 0,
        0, 85.0, (float(size * terrainScale)),
        (float(size * terrainScale)), 85.0, 0.0,
        (float(size * terrainScale)), 85.0, 0.0,
        0, 85.0, (float(size * terrainScale)),
        (float(size * terrainScale)), 85.0, (float(size * terrainScale))
};

load::shaderSource shaders[] = {load::shaderSource{R"(C:\CLionProjects\shaders\shaders\terrainshader.vert)",
                                       R"(C:\CLionProjects\shaders\shaders\terrainshader.frag)"},
                                load::shaderSource{R"(C:\CLionProjects\shaders\shaders\skyshader.vert)",
                                               R"(C:\CLionProjects\shaders\shaders\skyshader.frag)"},
                                load::shaderSource{R"(C:\CLionProjects\shaders\shaders\grassshader.vert)",
                                                   R"(C:\CLionProjects\shaders\shaders\grassshader.frag)"},
                                load::shaderSource{R"(C:\CLionProjects\shaders\shaders\watershader.vert)",
                                                   R"(C:\CLionProjects\shaders\shaders\watershader.frag)"}};

int shaderNum = sizeof(shaders)/sizeof(shaders[0]);
auto shaderPrograms = new int[shaderNum];

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

camera camera{707, 102, 402,
              1, 0, 0,
              0, 0, 1,
              0, -20};


void reflectCamera() {
    camera.stdDirY *= -1;
    camera.pitch *= -1;
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
    glm::mat4 lookAt = glm::mat4(1.0f);
    lookAt = glm::rotate(lookAt, glm::radians(float(k1 * 180 * acosf(dot(vy, vu)) / M_PI)), glm::vec3(1.0, 0.0, 0.0));
    lookAt = glm::rotate(lookAt, glm::radians(float(k2 * 180 * acosf(dot(vx, vr)) / M_PI)), glm::vec3(0.0, 1.0, 0.0));
    lookAt = glm::translate(lookAt, glm::vec3(-Cx, -Cy, -Cz));
    return lookAt;
}

void updCamera(double **height_map) {
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
        camera.stdDirX = cosf(M_PI * camera.yaw / 180) * cosf(M_PI * camera.pitch / 180);
        camera.stdDirY = sinf(M_PI * camera.pitch / 180);
        camera.stdDirZ = sinf(M_PI * camera.yaw / 180) * cosf(M_PI * camera.pitch / 180);
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
    if (camera.posX >= float(size * terrainScale) - 10) {
        camera.posX = size * terrainScale - 10;
    }
    if (camera.posZ >= float(size * terrainScale) - 10) {
        camera.posZ = size * terrainScale - 10;
    }
    if (camera.posX <= 10) {
        camera.posX = 10;
    }
    if (camera.posZ <= 10) {
        camera.posZ = 10;
    }
    auto height = float(height_map[int(camera.posX) / terrainScale][int(camera.posZ) / terrainScale]);
    if (height < waterHeight) {
        height = waterHeight;
    }
    if (camera.posY <= height + 2) {
        camera.posY = height + 2;
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

int getGrassInstances(const GLfloat *vertices, GLfloat *grass_instances, double **height_map, uint64_t seed) {
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed >> 32u)};
    std::mt19937_64 rng(ss);
    std::uniform_real_distribution<float> unif(-1, 1);
    glm::vec3 normal;
    float offset_coef = 2.5;
    int grass_num = 0;
    float rand_offset = 0;
    float rand_type = 0;
    float grass_type = 0;
    float rand_grass = 0;
    float rand_cos = 0;
    int blockPos;
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {
            blockPos = i * (size - 1) * 24 + j * 24;
            normal.x = vertices[blockPos + 3 + 0];
            normal.y = vertices[blockPos + 3 + 1];
            normal.z = vertices[blockPos + 3 + 2];
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

void calculateVertices(GLfloat *vertices, double **height_map) {
    glm::vec3 normal;
    int blockPos;
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {
            blockPos = i * (size - 1) * 24 + j * 24;
            vertices[blockPos + 0 + 0] = i * terrainScale;
            vertices[blockPos + 0 + 1] = height_map[i][j];
            vertices[blockPos + 0 + 2] = j * terrainScale;
            normal = getNormal(i, j, height_map);
            vertices[blockPos + 3 + 0] = normal.x;
            vertices[blockPos + 3 + 1] = normal.y;
            vertices[blockPos + 3 + 2] = normal.z;
            vertices[blockPos + 6 + 0] = i / 2.0;
            vertices[blockPos + 6 + 1] = j / 2.0;

            vertices[blockPos + 8 + 0] = i * terrainScale;
            vertices[blockPos + 8 + 1] = height_map[i][j + 1];
            vertices[blockPos + 8 + 2] = (j + 1) * terrainScale;
            normal = getNormal(i, j + 1, height_map);
            vertices[blockPos + 11 + 0] = normal.x;
            vertices[blockPos + 11 + 1] = normal.y;
            vertices[blockPos + 11 + 2] = normal.z;
            vertices[blockPos + 14 + 0] = i / 2.0;
            vertices[blockPos + 14 + 1] = (j + 1) / 2.0;

            vertices[blockPos + 16 + 0] = (i + 1) * terrainScale;
            vertices[blockPos + 16 + 1] = height_map[i + 1][j];
            vertices[blockPos + 16 + 2] = j * terrainScale;
            normal = getNormal(i + 1, j, height_map);
            vertices[blockPos + 19 + 0] = normal.x;
            vertices[blockPos + 19 + 1] = normal.y;
            vertices[blockPos + 19 + 2] = normal.z;
            vertices[blockPos + 22 + 0] = (i + 1) / 2.0;
            vertices[blockPos + 22 + 1] = j / 2.0;

            int step = (size - 1) * (size - 1) * 3 * 8;

            vertices[step + blockPos + 0 + 0] = (i + 1) * terrainScale;
            vertices[step + blockPos + 0 + 1] = height_map[i + 1][j + 1];
            vertices[step + blockPos + 0 + 2] = (j + 1) * terrainScale;
            normal = getNormal(i + 1, j + 1, height_map);
            vertices[step + blockPos + 3 + 0] = normal.x;
            vertices[step + blockPos + 3 + 1] = normal.y;
            vertices[step + blockPos + 3 + 2] = normal.z;
            vertices[step + blockPos + 6 + 0] = (i + 1) / 2.0;
            vertices[step + blockPos + 6 + 1] = (j + 1) / 2.0;

            vertices[step + blockPos + 8 + 0] = (i + 1) * terrainScale;
            vertices[step + blockPos + 8 + 1] = height_map[i + 1][j];
            vertices[step + blockPos + 8 + 2] = j * terrainScale;
            normal = getNormal(i + 1, j, height_map);
            vertices[step + blockPos + 11 + 0] = normal.x;
            vertices[step + blockPos + 11 + 1] = normal.y;
            vertices[step + blockPos + 11 + 2] = normal.z;
            vertices[step + blockPos + 14 + 0] = (i + 1) / 2.0;
            vertices[step + blockPos + 14 + 1] = j / 2.0;

            vertices[step + blockPos + 16 + 0] = i * terrainScale;
            vertices[step + blockPos + 16 + 1] = height_map[i][j + 1];
            vertices[step + blockPos + 16 + 2] = (j + 1) * terrainScale;
            normal = getNormal(i, j + 1, height_map);
            vertices[step + blockPos + 19 + 0] = normal.x;
            vertices[step + blockPos + 19 + 1] = normal.y;
            vertices[step + blockPos + 19 + 2] = normal.z;
            vertices[step + blockPos + 22 + 0] = i / 2.0;
            vertices[step + blockPos + 22 + 1] = (j + 1) / 2.0;
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

void render(bool isNotRef, glm::mat4 lookAt, glm::vec4 clip) {
    glUseProgram(shaderPrograms[0]);
    glUniformMatrix4fv(la_ter, 1, GL_FALSE, glm::value_ptr(lookAt));
    glUniform4fv(clip_ter, 1, glm::value_ptr(clip));
    glUniform3f(light_ter, 0.0, sun_y, sun_z);
    glUniform3f(camPos_ter, camera.posX, camera.posY, camera.posZ);
    glUniform3f(camDir_ter, camera.stdDirX, camera.stdDirY, camera.stdDirZ);
    glBindVertexArray(VAO_T);
    glDrawArrays(GL_TRIANGLES, 0, (size - 1) * 2 * (size - 1) * 3);

    glUseProgram(shaderPrograms[2]);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glUniformMatrix4fv(la_gr, 1, GL_FALSE, glm::value_ptr(lookAt));
    glUniform4fv(clip_gr, 1, glm::value_ptr(clip));
    glUniform3f(light_gr, 0.0, sun_y, sun_z);
    glUniform3f(camPos_gr, camera.posX, camera.posY, camera.posZ);
    glUniform3f(camDir_gr, camera.stdDirX, camera.stdDirY, camera.stdDirZ);
    glUniform1f(time_gr, delta);
    glBindVertexArray(VAO_G);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 18, grassNum);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    if (isNotRef) {
        glUseProgram(shaderPrograms[3]);
        glUniformMatrix4fv(la_w, 1, GL_FALSE, glm::value_ptr(lookAt));
        glUniform3f(camPos_w, camera.posX, camera.posY, camera.posZ);
        glUniform3f(light_w, 0.0, sun_y, sun_z);
        glUniform1f(stride_w, waveSpeed);
        glBindVertexArray(VAO_W);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glDepthFunc(GL_LEQUAL);
    glUseProgram(shaderPrograms[1]);
    glUniformMatrix4fv(la_s, 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(lookAt))));
    glUniform3f(sun_s, 0.0, sun_y, sun_z);
    glUniform3f(camDir_s, camera.stdDirX, camera.stdDirY, camera.stdDirZ);
    glBindVertexArray(VAO_S);
    glDrawArrays(GL_TRIANGLES, 0, 30);
    glDepthFunc(GL_LESS);
}

int main() {
    //генерация

    uint64_t seed = 239879823;
    auto height_map = new double*[size];
    for (int i = 0; i < size; i++) {
        height_map[i] = new double[size];
    }
    perlin::create_height_map(seed, height_map, size, 512, 6, 0.5);
    auto *vertices = new GLfloat[(size - 1) * (size - 1) * 2 * 3 * 8];
    auto *grass_instances = new GLfloat[(size - 1) * (size - 1) * 6];
    calculateVertices(vertices, height_map);
    grassNum = getGrassInstances(vertices, grass_instances, height_map, seed);

    //инициализация GLFW
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL", nullptr, nullptr);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CLIP_DISTANCE0);
    glEnable(GL_CLIP_DISTANCE1);
    glViewport(0, 0, screenWidth, screenHeight);

    //загрузка шейдеров
    load::load_shaders(shaders, shaderNum, shaderPrograms);

    //загрузка текстур
    unsigned int clay = load::load_texture(R"(C:\CLionProjects\shaders\textures\clay_texture.jpg)");
    unsigned int dirt = load::load_texture(R"(C:\CLionProjects\shaders\textures\dirt_texture.jpg)");
    unsigned int earth = load::load_texture(R"(C:\CLionProjects\shaders\textures\earth_texture.jpg)");
    unsigned int sand = load::load_texture(R"(C:\CLionProjects\shaders\textures\sand_texture.jpg)");
    unsigned int grass = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture.jpg)");
    unsigned int grass3 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture0.png)");
    unsigned int grass4 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture4.png)");
    unsigned int grass5 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture5.png)");
    unsigned int grass6 = load::load_texture(R"(C:\CLionProjects\shaders\textures\grass_texture6.png)");
    unsigned int dudv = load::load_texture(R"(C:\CLionProjects\shaders\textures\dudv.png)");
    unsigned int normal = load::load_texture(R"(C:\CLionProjects\shaders\textures\normal.png)");

    //создание буферов
    glGenVertexArrays(1, &VAO_S);
    glGenBuffers(1, &VBO_S);
    glBindVertexArray(VAO_S);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &VAO_T);
    glGenBuffers(1, &VBO_T);
    glBindVertexArray(VAO_T);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_T);
    glBufferData(GL_ARRAY_BUFFER, (size - 1) * 2 * (size - 1) * 3 * 8 * sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &VBO_instanced);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_instanced);
    glBufferData(GL_ARRAY_BUFFER, (size - 1) * (size - 1) * 6 * sizeof(float), grass_instances, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO_G);
    glGenBuffers(1, &VBO_G);
    glBindVertexArray(VAO_G);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_G);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_instanced);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glGenVertexArrays(1, &VAO_W);
    glGenBuffers(1, &VBO_W);
    glBindVertexArray(VAO_W);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_W);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenFramebuffers(1, &reflectionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glGenTextures(1, &reflectionTexture);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texResolutionWidth, texResolutionHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0);

    glGenRenderbuffers(1, &reflectionDepthFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, reflectionDepthFBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflectionDepthFBO);

    glGenFramebuffers(1, &refractionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glGenTextures(1, &refractionTexture);
    glBindTexture(GL_TEXTURE_2D, refractionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texResolutionWidth, texResolutionHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionTexture, 0);

    glGenTextures(1, &refractionDepthTexture);
    glBindTexture(GL_TEXTURE_2D, refractionDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, texResolutionWidth, texResolutionHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, refractionDepthTexture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //uniform-текстуры и их активация
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
    glUseProgram(shaderPrograms[3]);
    glUniform1i(glGetUniformLocation(shaderPrograms[3], "reflection"), 9);
    glUniform1i(glGetUniformLocation(shaderPrograms[3], "refraction"), 10);
    glUniform1i(glGetUniformLocation(shaderPrograms[3], "depth"), 11);
    glUniform1i(glGetUniformLocation(shaderPrograms[3], "dudv"), 12);
    glUniform1i(glGetUniformLocation(shaderPrograms[3], "normal"), 13);

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
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, grass4);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, grass3);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, grass5);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, grass6);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, refractionTexture);
    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, refractionDepthTexture);
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, dudv);
    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_2D, normal);

    //определение положения uniform-переменных
    glm::mat4 frustum = glm::perspective(glm::radians(45.0f), (float)screenWidth/(float)screenHeight, 0.1f, 4000.0f);
    glUseProgram(shaderPrograms[0]);
    proj = glGetUniformLocation(shaderPrograms[0], "projection");
    glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
    la_ter = glGetUniformLocation(shaderPrograms[0], "lookAt");
    light_ter = glGetUniformLocation(shaderPrograms[0], "light");
    camPos_ter = glGetUniformLocation(shaderPrograms[0], "camPos");
    camDir_ter = glGetUniformLocation(shaderPrograms[0], "camDir");
    clip_ter = glGetUniformLocation(shaderPrograms[0], "clip");
    glUseProgram(shaderPrograms[2]);
    proj = glGetUniformLocation(shaderPrograms[2], "projection");
    glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
    la_gr = glGetUniformLocation(shaderPrograms[2], "lookAt");
    light_gr = glGetUniformLocation(shaderPrograms[2], "light");
    camPos_gr = glGetUniformLocation(shaderPrograms[2], "camPos");
    time_gr = glGetUniformLocation(shaderPrograms[2], "deltaTime");
    camDir_gr = glGetUniformLocation(shaderPrograms[2], "camDir");
    clip_gr = glGetUniformLocation(shaderPrograms[2], "clip");
    glUseProgram(shaderPrograms[1]);
    proj = glGetUniformLocation(shaderPrograms[1], "projection");
    glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
    sun_s = glGetUniformLocation(shaderPrograms[1], "uSunPos");
    la_s = glGetUniformLocation(shaderPrograms[1], "lookAt");
    camDir_s = glGetUniformLocation(shaderPrograms[1], "camDir");
    glUseProgram(shaderPrograms[3]);
    proj = glGetUniformLocation(shaderPrograms[3], "projection");
    glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
    la_w = glGetUniformLocation(shaderPrograms[3], "lookAt");
    stride_w = glGetUniformLocation(shaderPrograms[3], "waveStride");
    camPos_w = glGetUniformLocation(shaderPrograms[3], "camPos");
    light_w = glGetUniformLocation(shaderPrograms[3], "light");
    mapSize_w = glGetUniformLocation(shaderPrograms[3], "mapSize");
    glUniform1f(mapSize_w, size * terrainScale);

    bool isNotRef;
    float dist;
    float theta = 0.07;
    while (!glfwWindowShouldClose(window)) {
        curTime = float(glfwGetTime());
        delta = curTime - prevTime;
        prevTime = curTime;
        speed = basicSpeed * delta;
        waveSpeed = waveSpeed + basicWaveSpeed * delta;
        waveSpeed = waveSpeed - long(waveSpeed);
        theta += 0.00125f * delta;
        sun_z = cosf(2.0f * float(M_PI) * theta);
        sun_y = sinf(2.0f * float(M_PI) * theta);
        std::cout << 1 / delta << std::endl;
        lookAtM = lookAt(camera.posX + camera.stdDirX, camera.posY + camera.stdDirY, camera.posZ + camera.stdDirZ,
                                       camera.posX, camera.posY, camera.posZ, 0, 1, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, texResolutionWidth, texResolutionHeight);
        isNotRef = false;
        dist = 2 * (camera.posY - waterHeight);
        camera.posY -= dist;
        reflectCamera();
        lookAtRefl = lookAt(camera.posX + camera.stdDirX, camera.posY + camera.stdDirY, camera.posZ + camera.stdDirZ,
                         camera.posX, camera.posY, camera.posZ, 0, 1, 0);
        render(isNotRef, lookAtRefl, glm::vec4{0.0, 1.0, 0.0, -waterHeight + glitchOffset});
        reflectCamera();
        camera.posY += dist;
        glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render(isNotRef, lookAtM, glm::vec4{0.0, -1.0, 0.0, waterHeight + glitchOffset});

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        isNotRef = true;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);
        render(isNotRef, lookAtM, glm::vec4{0.0, 1.0, 0.0, 10000});

        glfwSwapBuffers(window);
        glfwPollEvents();
        updCamera(height_map);
    }

    glDeleteVertexArrays(1, &VAO_T);
    glDeleteBuffers(1, &VBO_T);
    glDeleteVertexArrays(1, &VAO_S);
    glDeleteBuffers(1, &VBO_S);
    glDeleteVertexArrays(1, &VAO_G);
    glDeleteBuffers(1, &VBO_G);
    glDeleteBuffers(1, &VBO_instanced);
    glDeleteVertexArrays(1, &VAO_W);
    glDeleteBuffers(1, &VBO_W);
    glDeleteFramebuffers(1, &reflectionFBO);
    glDeleteFramebuffers(1, &refractionFBO);
    glDeleteRenderbuffers(1, &reflectionDepthFBO);
    glfwTerminate();
    delete[] vertices;
    delete[] grass_instances;
    for (int i = 0; i < size; i++) {
        delete[] height_map[i];
    }
    delete[] height_map;
    delete[] shaderPrograms;
    return 0;
}