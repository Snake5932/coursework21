#version 330 core

layout (location = 0) in vec3 aPosition;

out vec3 vPosition;

uniform mat4 lookAt;
uniform mat4 projection;

void main() {
    vec4 pos = projection * lookAt * vec4(aPosition, 1.0);
    gl_Position = pos.xyww;
    vPosition = aPosition;
}