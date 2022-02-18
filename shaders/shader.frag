#version 330 core

in vec3 oNormal;
in vec3 oPosition;
in float oSecondary;
in vec2 oTexCoord;

out vec4 outColor;

uniform vec3 light;
uniform vec3 camPos;
uniform sampler2D grassTexture;
uniform sampler2D earthTexture;
uniform sampler2D dirtTexture;
uniform sampler2D clayTexture;
uniform sampler2D sandTexture;

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

vec4 getTexColor(float first, vec2 oTexCoord) {
    float angle = abs(normalize(oNormal).y);
    vec4 sandColor = texture(sandTexture, oTexCoord);
    vec4 earthColor = texture(earthTexture, oTexCoord);
    vec4 dirtColor = texture(dirtTexture, oTexCoord);
    vec4 clayColor = texture(clayTexture, oTexCoord);
    vec4 grassColor = texture(grassTexture, oTexCoord);
    if (first >= 146 && angle >= 0.998) {
        return sandColor;
    }
    if (first >= 144 && first < 146 && angle >= 0.998) {
        float coef = (first - 144) / 2.0;
        coef = fade(coef);
        return (1 - coef) * clayColor + coef * sandColor;
    }
    if (first >= 146 && angle >= 0.978 && angle < 0.998) {
        float coef = (angle - 0.978) / 0.02;
        coef = fade(coef);
        return (1 - coef) * clayColor + coef * sandColor;
    }
    if (first >= 144 && first < 146 && angle >= 0.978 && angle < 0.998) {
        float coef = ((angle - 0.978) / 0.02) * ((first - 144) / 2.0);
        coef = fade(coef);
        return (1 - coef) * clayColor + coef * sandColor;
    }
    if (first >= 141) {
        return clayColor;
    }
    if (first >= 134 && first < 141) {
        float coef = (first - 134) / 7.0;
        coef = fade(coef);
        return (1 - coef) * earthColor + coef * clayColor;
    }
    if (first < 82) {
        return dirtColor;
    }
    if (first >= 82 && first < 87) {
        float coef = (first - 82) / 5.0;
        coef = fade(coef);
        return (1 - coef) * dirtColor + coef * earthColor;
    }
    if (first >= 87 && first < 94) {
        return earthColor;
    }
    if (first >= 94 && first < 100 && angle >= 0.997) {
        float coef = (first - 94) / 6.0;
        coef = fade(coef);
        return (1 - coef) * earthColor + coef * grassColor;
    }
    if (first >= 100 && first < 120 && angle >= 0.977 && angle < 0.997) {
        float coef = (angle - 0.977) / 0.02;
        coef = fade(coef);
        return (1 - coef) * earthColor + coef * grassColor;
    }
    if (first >= 94 && first < 100 && angle >= 0.977 && angle < 0.997) {
        float coef = ((angle - 0.977) / 0.02) * ((first - 94) / 6.0);
        coef = fade(coef);
        return (1 - coef) * earthColor + coef * grassColor;
    }
    if (first >= 100 && first < 120 && angle >= 0.997) {
        return grassColor;
    }
    if (first >= 120 && first < 126 && angle >= 0.997) {
        float coef = (first - 120) / 6.0;
        coef = fade(coef);
        return (1 - coef) * grassColor + coef * earthColor;
    }
    if (first >= 120 && first < 126 && angle >= 0.977 && angle < 0.997) {
        float coef = (1 - (0.997 - angle) / 0.02) * (1 - (first - 120) / 6.0);
        coef = fade(coef);
        return (1 - coef) * earthColor + coef * grassColor;
    }
    return earthColor;
}

vec3 getLight(vec3 lightColor, float ambientCoef, float specularCoef, vec3 light) {
    vec3 ambient = ambientCoef * lightColor;

    vec3 lightDir = normalize(light);
    vec3 viewDir = normalize(camPos - oPosition);
    vec3 normal = normalize(-oNormal);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 middleDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, middleDir), 0.0), 32.0);
    vec3 specular = specularCoef * spec * lightColor;
    return ambient + diffuse + specular;
}

void main() {
    float sunSpec;
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

    vec4 objectTextureColor = getTexColor(oPosition.y, oTexCoord);

    if (oPosition.y < 82) {
        sunSpec = 0.3;
    } else if (oPosition.y >= 82 && oPosition.y < 87) {
        float coef = (oPosition.y - 82) / 5.0;
        coef = fade(coef);
        sunSpec = 0.3 * (1 - coef);
    } else {
        sunSpec = 0.0;
    }

    vec3 lightColor = getLight(vec3(0.9 + redAdd, 0.9, 0.9), 0.3, sunSpec, light) * sunAttn +
                      getLight(vec3(0.2, 0.2, 0.25), 0.8, 0.0, vec3(0.8, 0.8, 0.7)) * moonAttn;

    outColor = vec4(lightColor, 1.0) * objectTextureColor;
}