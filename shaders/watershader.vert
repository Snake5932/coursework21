#version 330 core

layout (location = 0) in vec3 aPosition;

out vec4 oTexPos;
out vec2 dudvPos;
out vec3 oPosition;

uniform mat4 projection;
uniform mat4 lookAt;
uniform float mapSize;

const float multCoef = 80;

void main() {
    oTexPos = projection * lookAt * vec4(aPosition, 1.0);
    dudvPos = vec2(aPosition.x / mapSize, aPosition.z / mapSize) * multCoef;
    oPosition = aPosition;
    gl_Position = oTexPos;
}