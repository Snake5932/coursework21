#version 330 core

#define PI 3.141592

in vec3 vPosition;
out vec4 outColor;

uniform vec3 uSunPos;

const int segmentNum = 16;

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

vec2 ray_sphere_inter(vec3 r0, vec3 r, float atmRad) {
    float a = dot(r, r);
    float b = 2.0 * dot(r, r0);
    float c = dot(r0, r0) - (atmRad * atmRad);
    float d = (b * b) - 4.0 * a * c;
    if (d < 0.0) return vec2(1.0, -1.0);
    return vec2(
        (-b - sqrt(d)) / (2.0 * a),
        (-b + sqrt(d)) / (2.0 * a)
    );
}

vec3 atmosphere(vec3 r, vec3 r0, vec3 sunPos, float sunIntensity, float planetRad, float atmRad, vec3 rayleighCoef,
                float mieCoef, float scaleHeightR, float scaleHeightM, float g, float tMin, float tMax, float rayleighAttn) {

    vec3 sunPos2 = normalize(vec3(1.0, 0.5, 0.0));
    r = normalize(r);

    vec2 t = ray_sphere_inter(r0, r, atmRad);
    if (t.x > t.y || t.y < 0) return vec3(0, 0, 0);
    if (t.x > tMin && t.x > 0) tMin = t.x;
    if (t.y < tMax) tMax = t.y;
    float segmentLen = (tMax - tMin) / float(segmentNum);

    float curT = tMin;

    vec3 totalRayleigh = vec3(0, 0, 0);
    vec3 totalMie = vec3(0, 0, 0);

    float opticalDepthR = 0.0;
    float opticalDepthM = 0.0;

    float mu = dot(r, sunPos);
    float rayleighPhase = 3.0 / (16.0 * PI) * (1.0 + mu * mu);
    float miePhase = 3.0 / (8.0 * PI) * ((1.0 - g * g) * (mu * mu + 1.0)) / (pow(1.0 + g * g - 2.0 * mu * g, 1.5) * (2.0 + g * g));

    for (int i = 0; i < segmentNum; i++) {
        vec3 curPos = r0 + r * (curT + segmentLen * 0.5);

        float height = length(curPos) - planetRad;

        if (rayleighAttn < 0.00001) {
            float opticalDepthStM = exp(-height / scaleHeightM) * segmentLen;
            opticalDepthM += opticalDepthStM;

            float lightSegmentLen = ray_sphere_inter(curPos, sunPos, atmRad).y / float(segmentNum);

            float lightCurT = 0.0;

            float lightODM = 0.0;

            int j;
            for (j = 0; j < segmentNum; j++) {
                vec3 lightCurPos = curPos + sunPos * (lightCurT + lightSegmentLen * 0.5);

                float lightHeight = length(lightCurPos) - planetRad;
                if (lightHeight < 0) break;
                lightODM += exp(-lightHeight / scaleHeightM) * lightSegmentLen;

                lightCurT += lightSegmentLen;
            }

            if (j == segmentNum) {
                vec3 attn = exp(-(vec3(0.0) + mieCoef * 1.1 * (opticalDepthM + lightODM)));
                totalMie += opticalDepthStM * attn;
            }
            curT += segmentLen;
        } else {
            float opticalDepthStR = exp(-height / scaleHeightR) * segmentLen;
            float opticalDepthStM = exp(-height / scaleHeightM) * segmentLen;

            opticalDepthR += opticalDepthStR;
            opticalDepthM += opticalDepthStM;

            float lightSegmentLen = ray_sphere_inter(curPos, sunPos, atmRad).y / float(segmentNum);

            float lightCurT = 0.0;

            float lightODR = 0.0;
            float lightODM = 0.0;

            int j;
            for (j = 0; j < segmentNum; j++) {
                vec3 lightCurPos = curPos + sunPos * (lightCurT + lightSegmentLen * 0.5);

                float lightHeight = length(lightCurPos) - planetRad;
                if (lightHeight < 0) break;
                lightODR += exp(-lightHeight / scaleHeightR) * lightSegmentLen;
                lightODM += exp(-lightHeight / scaleHeightM) * lightSegmentLen;

                lightCurT += lightSegmentLen;
            }

            if (j == segmentNum) {
                vec3 attn = exp(-(rayleighCoef * (opticalDepthR + lightODR) + mieCoef * 1.1 * (opticalDepthM + lightODM)));
                totalRayleigh += opticalDepthStR * attn;
                totalMie += opticalDepthStM * attn;
            }
            curT += segmentLen;
        }
    }

    return sunIntensity * (rayleighPhase * rayleighCoef * totalRayleigh * rayleighAttn + miePhase * mieCoef * totalMie);
}

void main() {
    vec3 color;
    vec3 color1;
    vec3 color2;
    vec3 sunPos = normalize(uSunPos);
    float scaleHeightM = 2.0e3;
    float redAdd = 0.0;
    float rayleighAttnMoon = 0.0;
    float dayMoonAttn = 2.0;
    float sun_y = abs(sunPos.y);
    if (sun_y < 0.2 && sun_y > -0.2) {
        float sun_y_faded = fade(sun_y / 0.2);
        scaleHeightM += 1 - sun_y_faded;
        redAdd += (1 - sun_y_faded) / 10;
        rayleighAttnMoon = (1 - sun_y_faded) / 2;
        dayMoonAttn = (1 + sun_y_faded);
    }
    if (sunPos.y < 0.0) {
        rayleighAttnMoon = 1.0;
        dayMoonAttn = 1.0;
    }

    if (sunPos.y < -0.2) {
        color1 = vec3(0.0);
    } else {
        color = atmosphere(
        vPosition,
        vec3(0, 6377e3, 0),
        sunPos,
        70.0,
        6371e3,
        6477e3,
        vec3(5.8e-6, 13.5e-6, 33.1e-6),
        21e-6,
        8e3,
        scaleHeightM,
        0.998,
        0,
        1000000000,
        1
        );

        color1 = 1.0 - exp(-1.0 * color);
    }

    color = atmosphere(
        vPosition,
        vec3(0, 6377e3, 0),
        normalize(vec3(0.8, 0.8, 0.7)),
        70.0,
        6371e3,
        6477e3,
        vec3(11.4e-6, 17.5e-6, 33.1e-6),
        5e-6,
        1.4e3,
        4.0e3,
        0.9999,
        0,
        1000000000,
        rayleighAttnMoon
    );

    color2 = 1.0 - exp(-1.0 * color);

    color2.z += 0.1;

    color = color1 + color2 / dayMoonAttn;
    outColor = vec4(color.x + redAdd, color.y, color.z, 1.0);
}