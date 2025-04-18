#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 out_uv;

const vec2 projection_fixup = vec2(0.03125, 0.125);

vec2 camera_project(vec2 point) {
    return 2.0 * (point - vec2(-50)) * 1.0 / vec2(800, 600);
}

void main() {
    vec2 projection = camera_project(vertex.xy);
    gl_Position = vec4(projection - projection_fixup, 0, 1);
    out_uv = vertex.zw;
}
