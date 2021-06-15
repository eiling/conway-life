#version 430 core
layout (location = 0) in vec3 data;
layout (location = 1) in vec2 vertex_uv;

out vec2 uv;

layout (location = 2) uniform vec3 rot;

void main() {
    mat4 rotx = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, cos(rot.x), -sin(rot.x), 0.0,
    0.0, sin(rot.x), cos(rot.x), 0.0,
    0.0, 0.0, 0.0, 1.0
    );
    mat4 roty = mat4(
    cos(rot.y), 0.0, -sin(rot.y), 0.0,
    0.0, 1.0, 0.0, 0.0,
    sin(rot.y), 0.0, cos(rot.y), 0.0,
    0.0, 0.0, 0.0, 1.0
    );
    mat4 rotz = mat4(
    cos(rot.z), -sin(rot.z), 0.0, 0.0,
    sin(rot.z), cos(rot.z), 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
    );
    gl_Position = rotx * roty * rotz * vec4(data.xyz, 1.0);
    uv = vertex_uv;
}
