#version 330 core
layout (location = 0) in vec3 vtexPos;
layout (location = 1) in vec2 vtexuv;

out vec2 texCoord;

uniform mat4 transform;
uniform mat4 projection;
uniform vec2 UVstart;
uniform vec2 UVend;

void main()
{
    gl_Position = projection * transform * vec4(vtexPos.xyz, 1.0);
    texCoord = UVstart + vtexuv * (UVend - UVstart);
}