#include "loader.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

void load::load_shaders(shaderSource* shaders, int size, int* shaderProg) {
    std::string vShaderCode, fShaderCode;
    int vertexShader, fragmentShader, success, shaderProgram;
    const char* vertexShaderCode;
    const char* fragmentShaderCode;
    for (int i = 0; i < size; i++) {
        std::ifstream vShader(shaders[i].vertexShader);
        std::ifstream fShader(shaders[i].fragmentShader);
        std::stringstream vSStream, fSStream;
        vSStream << vShader.rdbuf();
        fSStream << fShader.rdbuf();
        vShader.close();
        fShader.close();
        vShaderCode = vSStream.str();
        fShaderCode = fSStream.str();
        vertexShaderCode = vShaderCode.c_str();
        fragmentShaderCode = fShaderCode.c_str();

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
        glCompileShader(vertexShader);

        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        shaderProg[i] = shaderProgram;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
}

unsigned int load::load_texture(char const* path) {
    unsigned int texture;
    glGenTextures(1, &texture);

    int width, height, channelNum;
    unsigned char* data = stbi_load(path, &width, &height, &channelNum, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (channelNum == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cout << "Failed to load a texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture;
}