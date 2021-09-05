#version 430 core
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;

layout(std430, binding = 0) buffer Params {
    int boardWidth;
    int boardHeight;
    int neighborIndices[8];
};

layout(std430, binding = 1) buffer Board {
    float board[];
};

void main() {
    uint local_x = gl_LocalInvocationID.x;
    uint local_y = gl_LocalInvocationID.y;
    if (local_x != 0 && local_x != gl_WorkGroupSize.x - 1 && local_y != 0 && local_y != gl_WorkGroupSize.y - 1) {
        uint x = gl_WorkGroupID.x;
        uint y = gl_WorkGroupID.y;
        float boardVal = board[x + 1 + (y + 1) * (boardWidth + 2)];
        boardVal = (boardVal + 1.0) / 2.0;
        imageStore(img, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, 0.0, boardVal, 1.0));
    } else {
        imageStore(img, ivec2(gl_GlobalInvocationID.xy), vec4(1.0, 0.0, 0.0, 1.0));
    }
}
