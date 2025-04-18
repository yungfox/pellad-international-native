#version 330 core

uniform sampler2D text;
uniform vec3 color;

in vec4 out_color;
in vec2 out_uv;

out vec4 frag_color;

void main() {
    float d = texture(text, out_uv).r;
    float aaf = fwidth(d);
    float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);
    frag_color = vec4(color.rgb, alpha);
}
