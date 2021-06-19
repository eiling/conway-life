#version 430 core
layout (std430, binding = 0) buffer Board {
    float board[];
};

layout (std430, binding = 1) buffer Params {
    int boardWidth;
    int boardHeight;
};

in vec2 uv;

out vec3 color;

void main(){
    float depth = (1.0 - gl_FragCoord.z);
    depth = depth * depth;

//    float u = uv.x;
//    float v = uv.y;
//    if (u > 0.5) {
//        u = 1.0 - u;
//    }
//    if (v > 0.5) {
//        v = 1.0 - v;
//    }
//
//    color = vec3(u, 0.0, v + 0.5) * depth;

    int x = int(uv.y * (boardWidth));
    int y = int(uv.x * (boardHeight));
    color = vec3((board[x + 1 + (y + 1) * (boardWidth + 2)] + 1.0) / 2.0) * depth;
}
