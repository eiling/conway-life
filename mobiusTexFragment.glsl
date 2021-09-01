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

uniform sampler2D tex;

float mod(float x, float y){
    float val = x - y;
    return val < 0 ? -val : val;
}

void main(){
    float depth = (1.0 - gl_FragCoord.z);
    depth = sqrt(depth);
    float colorVal = texture(tex, vec2(uv.y, uv.x)).x;
    color = vec3(colorVal, 0.0, 0.0) * depth;
}
