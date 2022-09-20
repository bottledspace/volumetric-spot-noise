#version 300 es
precision highp float;

layout (location = 0) in vec3 loc_in;

uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(loc_in, 1.0);
}