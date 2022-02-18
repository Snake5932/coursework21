#ifndef LOADER_H
#define LOADER_H

#include <string>

namespace load {
    struct shaderSource {
        std::string vertexShader;
        std::string fragmentShader;
    };

    void load_shaders(shaderSource* shaders, int size, int* shaderProg);
    unsigned int load_texture(char const * path);
}

#endif
