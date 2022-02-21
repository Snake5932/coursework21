#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec3 oNormal;
out vec3 oPosition;
out vec2 oTexCoord;

uniform mat4 lookAt;
uniform mat4 projection;
uniform vec3 camDir;
uniform vec3 camPos;
uniform vec4 clip;

const float clipAngle = 0.7;

void main() {
    vec4 pos = vec4(position, 1.0);
    gl_ClipDistance[1] = dot(clip, pos);
    gl_ClipDistance[0] = dot(normalize(camDir), normalize(position - camPos)) - clipAngle;
    oNormal = normal;
    oPosition = position;
    oTexCoord = texCoord;
    gl_Position = projection * lookAt * pos;
}