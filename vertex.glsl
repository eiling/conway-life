#version 430 core
layout (location = 0) in vec2 pos;
layout (location = 1) in float in_step;

out float step;

void main() {
    step = in_step;
    gl_Position = vec4(pos, 0.0, 1.0);
}
