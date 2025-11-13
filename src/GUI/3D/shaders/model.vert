attribute vec3 in_pos;
attribute vec3 in_normal;
uniform mat4 u_mvp;
varying vec3 v_normal;

void main() {
    gl_Position = u_mvp * vec4(in_pos, 1.0);
    v_normal = in_normal;
}