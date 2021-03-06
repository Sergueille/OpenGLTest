#version 330 core
layout (location = 0) in vec3 vtexPos;
layout (location = 1) in vec2 vtexuv;

out vec2 texCoord;
out vec3 position;
out vec2 lightmapCoord;

uniform mat4 transform;
uniform mat4 projection;
uniform vec2 UVstart = vec2(0);
uniform vec2 UVend = vec2(1);

uniform vec2 lightmapStart;
uniform vec2 lightmapEnd;

void main()
{
    gl_Position = projection * transform * vec4(vtexPos.xyz, 1.0);
    position = (transform * vec4(vtexPos.xyz, 1.0)).xyz;
    texCoord = UVstart + vtexuv * (UVend - UVstart);
    lightmapCoord = (vec2(position) - lightmapStart) / (lightmapEnd - lightmapStart);
}