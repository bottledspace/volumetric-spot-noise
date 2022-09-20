#version 300 es
precision highp float;
precision highp sampler3D;

uniform sampler2D accumbuf;
uniform sampler2D countbuf;

layout (location = 0) out vec4 color_out;

const float EPSILON = 0.00001;

bool isApproximatelyEqual(float a, float b) {
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}
float max3(vec3 v) {
    return max(max(v.x, v.y), v.z);
}

void main() {
    vec4 accum = texelFetch(accumbuf, ivec2(gl_FragCoord.xy), 0);
    float reveal = texelFetch(countbuf, ivec2(gl_FragCoord.xy), 0).r;
    if (isApproximatelyEqual(reveal, 1.0f))
        discard;
    if (isinf(max3(abs(accum.rgb))))
        accum.rgb = vec3(accum.a);
    vec3 avg = accum.rgb / max(accum.a, EPSILON);

    color_out = vec4(avg.rgb, 1.0-reveal);
}
