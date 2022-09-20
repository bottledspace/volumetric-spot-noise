#version 300 es
precision highp float;
precision highp int;
precision highp sampler3D;

uniform int mode;
uniform sampler2D impulses;
uniform mat4 invproj;
uniform mat4 proj;
uniform mat4 mv;
uniform mat4 mvp;

layout (location = 0) out vec4 accum_out;
layout (location = 1) out float count_out;

in vec2 axis;
in mat2 Q;
in vec4 U;
in vec4 color;
in float scalefactor;
in float light;

float d(float z) {
    return ((1.0*500.0)/(z)-500.0)/(1.0-500.0);
}
float w(float z) {
    return max(1e-2,min(3e3,10.0/(1e-5+pow(abs(z)/5.0,2.0)+pow(abs(z)/200.0,6.0))));
}

void main() {
    vec4 ndcPos;
    ndcPos.xy = ((2.0 * gl_FragCoord.xy)) / 1024.0 - 1.0;
    ndcPos.z = 1.0;
    ndcPos.w = 1.0;

    vec2 disp = 1024.0*(ndcPos.xy-U.xy);
    float dist = dot(disp, Q*disp);
    float ai = min(1.0, scalefactor*exp(-0.5*dist));

    //if (dot(vec3(disp,1.0), vec3(axis,1.0))<0.0)
    //    discard;
    //if (ai < 1e-6)
    //    discard;
    //0.04 for grass
    float lightfactor = light*0.06*max(0.0,abs(dot(disp, axis)));
    float weight =
        max(min(1.0, max(max(color.r, color.g), color.b) * ai*color.a), ai*color.a)
        * clamp(0.03 / (1e-5 + pow(U.z / 200.0, 4.0)), 1e-2, 3e3);

    accum_out = vec4(color.rgb * lightfactor, ai*color.a)*weight;
    count_out = ai*color.a;
}
