#version 100
attribute vec2 aPos;
attribute vec2 aTex;

varying vec2 uv;

void main() {
    uv = aTex;
    gl_Position = vec4(aPos, 0.0, 1.0);
}