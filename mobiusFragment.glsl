#version 430 core
in vec2 uv;

out vec3 color;

void main(){
    float depth = (1.0 - gl_FragCoord.z);
    depth = depth * depth;

    float u = uv.x;
    float v = uv.y;
    if (u > 0.5) {
        u = 1.0 - u;
    }
    if (v > 0.5) {
        v = 1.0 - v;
    }

    color = vec3(u, 0.0, v + 0.5) * depth;
}
