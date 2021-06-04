#version 330 core
out vec3 color;

void main(){
    float x = gl_FragCoord.x / 640.0;
    float y = gl_FragCoord.y / 480.0;
    color = vec3(x, x * y, y);
}
