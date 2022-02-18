#version 330 core

in vec2 oTexPos;
in vec3 oPosition;
in vec3 oNormal;
flat in int oType;

out vec4 outColor;

uniform vec3 light;
uniform vec3 camPos;
uniform sampler2D grassTexture2;
uniform sampler2D grassTexture3;
uniform sampler2D grassTexture5;
uniform sampler2D grassTexture6;

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

vec3 getLight(vec3 lightColor, float ambientCoef, float specularCoef, vec3 light) {
    vec3 ambient = ambientCoef * lightColor;

    vec3 lightDir = normalize(light);
    vec3 viewDir = normalize(camPos - oPosition);
    vec3 normal = normalize(oNormal);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = 0.65 * diff * lightColor;

    vec3 middleDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, middleDir), 0.0), 32.0);
    vec3 specular = specularCoef * spec * lightColor;
    return ambient + diffuse + specular;
}

void main() {
    vec4 texColor;
    vec4 texColor2 = texture(grassTexture2, oTexPos);
    vec4 texColor3 = texture(grassTexture3, oTexPos);
    vec4 texColor5 = texture(grassTexture5, oTexPos);
    vec4 texColor6 = texture(grassTexture6, oTexPos);
    if (oType == 2 || oType == 7) {
        texColor = texColor2;
    }
    if (oType == 3) {
        texColor = texColor3;
    }
    if (oType == 6) {
        texColor = texColor5;
    }
    if (oType == 8) {
        texColor = texColor6;
    }
    if(texColor.a < 0.4) {
        discard;
    } else {
        texColor.a = 0.99;
    }

    float sunAttn = 1.0;
    float moonAttn = 0.0;
    float redAdd = 0.0;
    vec3 sun = normalize(light);
    float sun_y = abs(light.y);

    if (sun_y < 0.2) {
        float sun_y_faded = fade(sun_y / 0.2);
        sunAttn = sun_y_faded;
        redAdd += (1 - sun_y_faded) / 10;
        moonAttn = (1 - sun_y_faded);
    }
    if (light.y < 0.0) {
        sunAttn = 0.0;
        moonAttn = 1.0;
    }

    vec3 lightColor = getLight(vec3(0.9 + redAdd, 0.9, 0.9), 0.7, 0.1, light) * sunAttn +
                      getLight(vec3(0.3, 0.3, 0.35), 1.0, 0.0, vec3(0.8, 0.8, 0.7)) * moonAttn;

    outColor = texColor * vec4(lightColor, 1.0);
}