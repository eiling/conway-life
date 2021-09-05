#version 430 core
in vec2 uv;

out vec3 color;

layout(location = 3) uniform sampler2D tex;

float mod(float x, float y){
    float val = x - y;
    return val < 0 ? -val : val;
}

void main(){
    float depth = (1.0 - gl_FragCoord.z);
    depth = sqrt(depth);
    color = texture(tex, vec2(uv.y, uv.x)).xyz * depth;
}
