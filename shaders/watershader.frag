#version 330 core

in vec4 oTexPos;
in vec2 dudvPos;
in vec3 oPosition;

out vec4 outColor;

uniform sampler2D reflection;
uniform sampler2D refraction;
uniform sampler2D dudv;
uniform sampler2D normal;
uniform sampler2D depth;
uniform float waveStride;
uniform vec3 camPos;
uniform vec3 light;

float strength = 0.005;
float near = 0.1;
float far = 4000;
float depthCoef = 2.0;
float flattnessCoef = 2.0;

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

vec3 getLight(vec3 lightColor, float specularCoef, vec3 light, vec3 normal, vec3 camDir) {
    vec3 lightDir = normalize(light);

    vec3 middleDir = normalize(lightDir + camDir);
    float spec = pow(max(dot(normal, middleDir), 0.0), 50.0);
    vec3 specular = specularCoef * spec * lightColor;
    return specular;
}

void main() {
    float sunAttn = 1.0;
    float moonAttn = 0.0;
    float redAdd = 0.0;
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

    vec2 texCoord = oTexPos.xy / oTexPos.w / 2.0 + 0.5;
    vec2 reflectionCoord = vec2(texCoord.x, -texCoord.y);
    vec2 refractionCoord = vec2(texCoord.x, texCoord.y);
    float waterDepth = texture(depth, refractionCoord).r;
    float bottomDist = 2.0 * near * far / (far + near - (2.0 * waterDepth - 1.0) * (far - near));
    waterDepth = gl_FragCoord.z;
    float waterDist = 2.0 * near * far / (far + near - (2.0 * waterDepth - 1.0) * (far - near));
    waterDepth = bottomDist - waterDist;
    float depthAlpha = fade(clamp(waterDepth / depthCoef, 0.0, 1.0));
    vec2 distortion = texture(dudv, vec2(dudvPos.x + waveStride, dudvPos.y)).rg * 0.1;
    distortion = dudvPos + vec2(distortion.x, distortion.y + waveStride);
    distortion = (texture(dudv, vec2(distortion.x, distortion.y)).rg * 2.0 - 1.0) * strength * depthAlpha;

    reflectionCoord += distortion;
    reflectionCoord.x = clamp(reflectionCoord.x, 0.0001, 0.9999);
    reflectionCoord.y = clamp(reflectionCoord.y, -0.9999, -0.0001);
    refractionCoord += distortion;
    refractionCoord = clamp(refractionCoord, 0.0001, 0.9999);
    vec4 reflectionColor = texture(reflection, reflectionCoord);
    vec4 refractionColor = texture(refraction, refractionCoord);
    vec3 camDir = normalize(camPos - oPosition);
    vec4 normalColor = texture(normal, distortion);
    vec3 normalDir = normalize(vec3(normalColor.r * 2.0 - 1.0, normalColor.b * flattnessCoef, normalColor.g * 2.0 - 1.0));
    float reflectionCoef = dot(camDir, normalDir);
    reflectionCoef = clamp(reflectionCoef, 0.3, 0.7);
    reflectionCoef = fade(reflectionCoef);
    vec3 lightColor = getLight(vec3(0.9 + redAdd, 0.9, 0.9), 0.7, light, normalDir, camDir) * sunAttn * depthAlpha +
                      getLight(vec3(0.2, 0.2, 0.25), 1.0, vec3(0.8, 0.8, 0.7), normalDir, camDir) * moonAttn;
    outColor = mix(reflectionColor, refractionColor, reflectionCoef) + vec4(lightColor, 1.0);
    outColor.a = depthAlpha;
}