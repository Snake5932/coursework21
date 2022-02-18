#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in float secondary;
layout (location = 3) in vec2 texCoord;

out vec3 oNormal;
out vec3 oPosition;
out float oSecondary;
out vec2 oTexCoord;

uniform mat4 model;
uniform mat4 rModel;
uniform mat4 lookAt;
uniform mat4 projection;

void main() {
    oNormal = normal;
    oPosition = vec3(rModel * model * vec4(position, 1.0f));
    oSecondary = secondary;
    oTexCoord = texCoord;
    gl_Position = projection * lookAt * rModel * model * vec4(position, 1.0f);
}