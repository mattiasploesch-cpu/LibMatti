#version 150

#moj_import <fog.glsl>

in vec3 Position;

void main() {
    gl_Position = vec4(Position, 1.0);
}
