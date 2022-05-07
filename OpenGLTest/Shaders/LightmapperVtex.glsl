#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTexCoord;

uniform vec2 quadSize;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(vPos.xy * 2, 0, 1); // Multiply by two so it's -1 1 not -0.5f 0.5f
    texCoord = vTexCoord;
}