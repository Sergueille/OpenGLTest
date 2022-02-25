#version 330 core
layout (location = 0) in vec3 vtexPos;
layout (location = 1) in vec2 vtexuv;

out vec2 texCoord;

uniform mat4 transform;
uniform mat4 projection;

void main()
{
    gl_Position = projection * transform * vec4(vtexPos.xy, vtexPos.z - 1.0, 1.0);
    texCoord = vtexuv;
}