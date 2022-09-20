#version 300 es
precision highp float;

layout (location = 0) in vec3 loc_in;

void main() {
    gl_Position = vec4(loc_in,1.0);
}