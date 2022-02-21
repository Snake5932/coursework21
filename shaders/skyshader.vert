#version 330 core

layout (location = 0) in vec3 aPosition;

out vec3 vPosition;

uniform mat4 lookAt;
uniform mat4 projection;
uniform vec3 camDir;

void main() {
    vec4 pos = vec4(aPosition, 1.0);
    gl_ClipDistance[0] = dot(normalize(camDir), normalize(aPosition - vec3(0.0, 0.0, 0.0)));
    pos = projection * lookAt * pos;
    gl_Position = pos.xyww;
    vPosition = aPosition;
}