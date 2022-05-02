layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoord;
in vec2 lightmapCoord;
  
uniform vec4 mainColor; 

uniform bool hasTexture = false;
uniform sampler2D mainTexture;

uniform bool isLit = false;
uniform sampler2D lightmap;

void main()
{
    vec4 color;
    if (hasTexture)
    {
        color = texture(mainTexture, texCoord) * mainColor;
    }
    else
    {
        color = mainColor;
    }

    if (isLit)
    {
        color *= vec4(texture(lightmap, lightmapCoord).rgb, 1);
    }

    if (color.a < 0.01)
    {
        discard;
    }

    FragColor = color;
    BrightColor = getBright(color);
} 