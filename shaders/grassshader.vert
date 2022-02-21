#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texPos;
layout (location = 2) in vec3 grassPos;
layout (location = 3) in float type;
layout (location = 4) in vec3 rotation;

out vec2 oTexPos;
out vec3 oPosition;
out vec3 oNormal;
flat out int oType;

uniform mat4 lookAt;
uniform mat4 projection;
uniform float deltaTime;
uniform vec3 camPos;
uniform vec3 camDir;
uniform vec4 clip;

const float clipAngle = 0.7;

//шумовая функция https://stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader
const vec2 k = vec2(23.175769263277926, 2.6651441426902251);
float random(vec2 uv) {
    return fract(cos(mod(1652354674.0, 1024.0 * dot(uv,k))));
}

void main() {
    gl_ClipDistance[0] = dot(normalize(camDir), normalize(grassPos - camPos)) - clipAngle;
    float xCoef;
    float yCoef;
    float yOffset;
    float rndAdd = random(grassPos.xz) * 5;
    if (type == 2) {
        xCoef = 6;
        yCoef = 6 + rndAdd;
        yOffset = 2.6;
    }
    if (type == 3) {
        xCoef = 6;
        yCoef = 6 + rndAdd;
        yOffset = 2.6;
    }
    if (type == 6) {
        xCoef = 6;
        yCoef = 7 + rndAdd;
        yOffset = 3.1;
    }
    if (type == 8) {
        xCoef = 6;
        yCoef = 9.6 + rndAdd;
        yOffset = 3.7;
    }
    mat3 rotationMat = mat3(rotation.x, 0, -rotation.y,
                            0, 1, 0,
                            rotation.y, 0, rotation.x);
    vec3 positionR = rotationMat * position;
    vec4 pos = vec4(vec3(grassPos.x, grassPos.y + yOffset, grassPos.z) +
                     vec3(positionR.x * xCoef, positionR.y * yCoef, positionR.z * xCoef), 1.0);
    gl_ClipDistance[1] = dot(clip, pos);
    vec3 camR = normalize(vec3(lookAt[0][0], lookAt[1][0], lookAt[2][0]));
    vec3 camUp = normalize(vec3(lookAt[0][1], lookAt[1][1], lookAt[2][1]));
    oTexPos = texPos;
    oType = int(type);
    oPosition = vec3(grassPos.x, grassPos.y + yOffset, grassPos.z) + camR * position.x * xCoef + camUp * position.y * yCoef;
    float coef = abs(cross(camR, camUp).y);
    camUp.y = (1 - coef) * camUp.y + coef;
    camUp = normalize(camUp);
    oNormal = normalize(cross(camR, camUp) + camUp);
    gl_Position = projection * lookAt * pos;
}