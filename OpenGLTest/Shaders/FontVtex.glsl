#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 texCoord;
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vPos.xyz, 1.0);
    TexCoords = texCoord;
}  