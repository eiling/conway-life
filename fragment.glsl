#version 430 core
layout (std430, binding = 0) buffer Board {
    float board[];
};

layout (std430, binding = 1) buffer Params {
    int width;
    int height;
    int boardWidth;
    int boardHeight;
};

out vec3 color;

void main(){
    int x = int(gl_FragCoord.x * boardWidth / width);
    int y = int(gl_FragCoord.y * boardHeight / height);
    color = vec3(board[x + 1 + (y + 1) * (boardWidth + 2)]) * 0.7 + 0.1;
}
