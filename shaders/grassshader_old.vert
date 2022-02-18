#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texPos;
layout (location = 2) in vec3 grassPos;
layout (location = 3) in float type;

out vec2 oTexPos;
out vec3 oPosition;
out vec3 oNormal;
flat out int oType;


uniform mat4 lookAt;
uniform mat4 projection;

void main() {
    float xCoef = 1;
    float yCoef = 1;
    float yOffset = 0.5;
    if (type == 2 || type == 7) {
        xCoef = 5;
        yCoef = 1.8;
        yOffset = 0.5;
    }
    if (type == 3) {
        xCoef = 6;
        yCoef = 4;
        yOffset = 0.5;
    }
    if (type == 6) {
        xCoef = 3.5;
        yCoef = 4;
        yOffset = 0.5;
    }
    if (type == 8) {
        xCoef = 4;
        yCoef = 6.4;
        yOffset = 0.5;
    }
    vec3 camR = normalize(vec3(lookAt[0][0], lookAt[1][0], lookAt[2][0]));
    vec3 camUp = normalize(vec3(lookAt[0][1], lookAt[1][1], lookAt[2][1]));
    oTexPos = texPos;
    oType = int(type);
    oPosition = vec3(grassPos.x, grassPos.y + 0.5, grassPos.z) + camR * position.x * xCoef + camUp * position.y * yCoef;
    float coef = abs(cross(camR, camUp).y);
    camUp.y = (1 - coef) * camUp.y + coef;
    camUp = normalize(camUp);
    oNormal = normalize(cross(camR, camUp) + camUp);
    gl_Position = projection * lookAt * vec4(vec3(grassPos.x, grassPos.y + yOffset, grassPos.z) +
                                             camR * position.x * xCoef + camUp * position.y * yCoef, 1.0f);
}