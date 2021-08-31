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

float mod(float x, float y){
    float val = x - y;
    return val < 0 ? -val : val;
}

void main(){
    float zero = 0.05;
    float half_zero = zero / 2.0;
    float depth = (1.0 - gl_FragCoord.z);
    depth = sqrt(depth);

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

    float raw_x = uv.y * (boardWidth) + half_zero;
    float raw_y = uv.x * (boardHeight) + half_zero;
    int x = int(raw_x);
    int y = int(raw_y);
    float dx = mod(raw_x, float(x));
    float dy = mod(raw_y, float(y));
    if (dx <= zero || dy <= zero) {
        color = vec3(0.5, 0.0, 0.0);
    } else {
        color = vec3(0.0, 0.0, (board[x + 1 + (y + 1) * (boardWidth + 2)] + 1.0) / 2.0) * depth;
    }
}
