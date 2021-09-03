#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer Params {
    int boardWidth;
    int boardHeight;
    int neighborIndices[8];
};

layout(std430, binding = 1) buffer CurrentBoard {
    float currentBoard[];
};

layout(std430, binding = 2) buffer OldBoard {
    float oldBoard[];
};

void main() {
    float fadeConst = 0.5;
//    for (int i = 0; i < boardWidth; ++i) {
//        oldBoard[(i + 1) * (boardWidth + 2)] = oldBoard[boardWidth + (boardHeight - i) * (boardWidth + 2)];
//        oldBoard[boardWidth + 1 + (i + 1) * (boardWidth + 2)] = oldBoard[1 + (boardHeight - i) * (boardWidth + 2)];
//    }
    uint index = gl_WorkGroupID.x + 1 + (gl_WorkGroupID.y + 1) * (boardWidth + 2);
    int sum = 0;
    for (int k = 0; k < 8; ++k) {
        sum += oldBoard[index + neighborIndices[k]] == 1.0 ? 1 : 0;
    }
    if (oldBoard[index] == 1.0) {
        if (sum < 2 || sum >= 4) {
            // board[index] = .0f;
            currentBoard[index] = oldBoard[index] * fadeConst;
        } else {
            currentBoard[index] = 1.0;
        }
    } else {
        if (sum == 3) {
            currentBoard[index] = 1.0;
        } else {
            // board[index] = .0f;
            currentBoard[index] = oldBoard[index] * fadeConst;
        }
    }
}
