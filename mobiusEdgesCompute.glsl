#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer Params {
    int boardWidth;
    int boardHeight;
    int neighborIndices[8];
};

layout(std430, binding = 2) buffer CurrentBoard {
    float board[];
};

void main() {
    uint i = gl_WorkGroupID.x;
    board[(i + 1) * (boardWidth + 2)] = board[boardWidth + (boardHeight - i) * (boardWidth + 2)];
    board[boardWidth + 1 + (i + 1) * (boardWidth + 2)] = board[1 + (boardHeight - i) * (boardWidth + 2)];
}
